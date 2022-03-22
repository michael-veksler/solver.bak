#pragma once
#include "constraint.hpp"
#include <cassert>
#include <concepts>
#include <cstdint>
#include <utility>
#include <vector>

namespace solver {
template<constraint_state state_template_t>
requires std::is_same_v<typename state_template_t::domain_type::value_type, bool>
class binary_clause
{
  public:
    using state_t = state_template_t;
    using param_index_t = typename state_t::param_index_t;
    using parameter_t = typename state_t::parameter_t;

    explicit binary_clause(const std::vector<parameter_t> &parameters, const std::vector<bool> &positive_literals)
    {
        assert(positive_literals.size() == parameters.size());
        m_literals.reserve(positive_literals.size());
        for (unsigned i = 0; i != positive_literals.size(); ++i) {
            m_literals.push_back({ parameters[i], positive_literals[i] });
        }
    }
    bool is_satisfied(const state_t &state) const
    {
        for (const auto &[param, positive_literal] : m_literals) {
            if (state.get_value(param) == positive_literal) {
                return true;
            }
        }
        return false;
    }
    propagation_result_t propagate(state_t &state, unsigned trigger_param)
    {
        assert(trigger_param < m_literals.size());
        if (trigger_param == m_watch0) {
            return propagate_by_watch(state, m_watch0, m_watch1);
        }
        if constexpr (state_t::only_watches_trigger) {
            return propagate_by_watch(state, m_watch1, m_watch0);
        } else if (trigger_param == m_watch1) {
            return propagate_by_watch(state, m_watch1, m_watch0);
        } else {
            return propagation_result_t::CONSISTENT;
        }
    }


  private:
    propagation_result_t propagate_by_watch(state_t &state, param_index_t &triggered_watch, param_index_t other_watch)
    {
        if (state.get_value(m_literals[triggered_watch].first) == m_literals[triggered_watch].second) {
            return propagation_result_t::SAT;
        }
        param_index_t next_watch = get_next_watch(triggered_watch);
        for (; next_watch != triggered_watch; next_watch = get_next_watch(next_watch)) {
            auto [param, positive_literal] = m_literals[next_watch];
            if (next_watch == other_watch || state.get_domain(param).count(positive_literal) == 0) {
                continue;
            }
            if (state.get_domain(param).size() == 1) {
                return propagation_result_t::SAT;
            }

            state.unregister_watch(m_literals[triggered_watch].first);
            triggered_watch = next_watch;
            state.register_watch(param);
            return propagation_result_t::CONSISTENT;
        }
        if (const auto &literal = m_literals[other_watch]; state.get_domain(literal.first).count(literal.second)) {
            return state.get_domain(literal.first).size() == 1 ? propagation_result_t::SAT
                                                               : propagation_result_t::CONSISTENT;
        }
        return propagation_result_t::UNSAT;
    }
    param_index_t get_next_watch(const param_index_t &watch) const
    {
        return std::cmp_equal(watch + 1, m_literals.size()) ? 0 : watch + 1;
    }

    std::vector<std::pair<parameter_t, bool>> m_literals;
    param_index_t m_watch0 = 0;
    param_index_t m_watch1 = 1;
};
}// namespace solver
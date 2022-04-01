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
class binary_clause_base
{
  public:
    using state_t = state_template_t;
    using param_index_t = typename state_t::param_index_t;
    using parameter_t = typename state_t::parameter_t;

    explicit binary_clause_base(const std::vector<parameter_t> &parameters, const std::vector<bool> &positive_literals)
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
    parameter_t get_parameter(unsigned index) const { return m_literals[index].first; }
    bool is_positive_literal(unsigned index) const { return m_literals[index].second; }
    size_t num_parameters() const { return m_literals.size(); }

  private:
    std::vector<std::pair<parameter_t, bool>> m_literals;
};

template<constraint_state state_template_t>
requires bool_domain<state_template_t>
class binary_clause : public binary_clause_base<state_template_t>
{
  public:
    using state_t = state_template_t;
    using param_index_t = typename state_t::param_index_t;
    using parameter_t = typename state_t::parameter_t;

    explicit binary_clause(const std::vector<parameter_t> &parameters, const std::vector<bool> &positive_literals)
        : binary_clause_base<state_template_t>(parameters, positive_literals)
    {}
};

template<propagating_constraint_state state_template_t>
requires bool_domain<state_template_t>
class binary_clause<state_template_t> : public binary_clause_base<state_template_t>
{
  public:
    using state_t = state_template_t;
    using param_index_t = typename state_t::param_index_t;
    using parameter_t = typename state_t::parameter_t;
    using binary_clause_base<state_template_t>::num_parameters;
    using binary_clause_base<state_template_t>::get_parameter;
    using binary_clause_base<state_template_t>::is_positive_literal;

    explicit binary_clause(const std::vector<parameter_t> &parameters, const std::vector<bool> &positive_literals)
        : binary_clause_base<state_template_t>(parameters, positive_literals)
    {}
    propagation_result_t propagate(state_t &state, unsigned trigger_param)
    {
        if (trigger_param == m_watch0) {
            return propagate_by_watch(state, m_watch0, m_watch1);
        }
        if constexpr (state_t::only_watches_trigger) {
            return propagate_by_watch(state, m_watch1, m_watch0);
        } else {
            if (trigger_param >= num_parameters() || trigger_param > std::numeric_limits<param_index_t>::max()) {
                throw std::out_of_range("tigger param is too big");
            }
            m_watch1 = static_cast<param_index_t>(trigger_param);
            return propagate_by_watch(state, m_watch1, m_watch0);
        }
    }


  private:
    propagation_result_t
        propagate_by_watch(state_t &state, param_index_t &triggered_watch, const param_index_t &other_watch) const
    {
        if (state.get_value(get_parameter(triggered_watch)) == is_positive_literal(triggered_watch)) {
            return propagation_result_t::SAT;
        }
        param_index_t next_watch = get_next_watch(triggered_watch);
        for (; next_watch != triggered_watch; next_watch = get_next_watch(next_watch)) {
            parameter_t param = get_parameter(next_watch);
            bool positive_literal = is_positive_literal(next_watch);
            if (next_watch != other_watch && state.get_domain(param).count(positive_literal)) {
                if (state.get_domain(param).size() == 1) {
                    return propagation_result_t::SAT;
                }

                state.unregister_watch(get_parameter(triggered_watch));
                triggered_watch = next_watch;
                state.register_watch(param);
                return propagation_result_t::CONSISTENT;
            }
        }
        parameter_t other_param = get_parameter(other_watch);
        bool other_is_positive = is_positive_literal(other_watch);

        if (state.get_domain(other_param).count(other_is_positive)) {
            if (state.get_domain(other_param).size() > 1) {
                state.set_value(other_param, other_is_positive);
            }
            return propagation_result_t::SAT;
        }
        return propagation_result_t::UNSAT;
    }
    param_index_t get_next_watch(const param_index_t &watch) const
    {
        return std::cmp_equal(watch + 1, num_parameters()) ? 0 : watch + 1;
    }

    param_index_t m_watch0 = 0;
    param_index_t m_watch1 = 1;
};
}// namespace solver
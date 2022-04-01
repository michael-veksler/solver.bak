#pragma once

#include "binary_clause.hpp"
#include "constraint.hpp"
#include "utilities.hpp"
#include <algorithm>
#include <cinttypes>
#include <cstdlib>
#include <iterator>
#include <vector>

namespace solver {

template<typename Domain, std::integral VariableIndexType> class uniform_constraint_state
{
  public:
    using domain_type = Domain;
    using value_type = typename domain_type::value_type;
    using param_index_t = VariableIndexType;
    struct parameter_t
    {
        VariableIndexType variable_index;
    };
    explicit uniform_constraint_state(unsigned variables, const domain_type &domain) : m_variables(variables, domain) {}
    const domain_type &get_domain(parameter_t param) const { return m_variables[param.variable_index]; }
    value_type get_value(parameter_t param) const
    {
        const domain_type &domain = get_domain(param);
        assert(domain.size() == 1);
        return *domain.begin();
    }
    void set_domain(parameter_t param, const domain_type &dom) { m_variables[param.variable_index] = dom; }
    void set_value(parameter_t param, value_type value)
    {
        m_variables[param.variable_index].clear();
        m_variables[param.variable_index].insert(value);
    }
    state_saver<domain_type> saved_state(unsigned variable)
    {
        return state_saver<domain_type>{ m_variables[variable] };
    }
    size_t size() const { return m_variables.size(); }

  private:
    std::vector<domain_type> m_variables;
};

template<constraint_state constraint_state_t> class exhaustive_solver
{
  public:
    using domain_type = typename constraint_state_t::domain_type;
    using parameter_t = typename constraint_state_t::parameter_t;
    using value_type = typename domain_type::value_type;

    exhaustive_solver(unsigned variables, const domain_type &domain) : m_state(variables, domain) {}

    bool solve() { return try_assignments(0); }

    void add_clause(const std::vector<int> &literals)
    {
        std::vector<parameter_t> parameters;
        parameters.reserve(literals.size());
        std::transform(literals.begin(), literals.end(), std::back_inserter(parameters), [](int literal) {
            return parameter_t{ static_cast<value_type>(std::abs(literal) - 1) };
        });

        std::vector<bool> positive_literals;
        positive_literals.reserve(literals.size());
        std::transform(literals.begin(), literals.end(), std::back_inserter(positive_literals), [](int literal) {
            return literal > 0;
        });

        m_clauses.emplace_back(binary_clause<constraint_state_t>(parameters, positive_literals));
    }
    value_type get_value(unsigned index) const { return m_state.get_value(parameter_t{ static_cast<uint8_t>(index) }); }
    size_t num_variables() const { return m_state.size(); }

  private:
    bool try_assignments(uint8_t depth)
    {
        if (depth >= m_state.size()) {
            return std::all_of(begin(m_clauses), end(m_clauses), [this](const auto &constraint) {
                return constraint.is_satisfied(m_state);
            });
        }

        state_saver<domain_type> saved_domain = m_state.saved_state(depth);
        for (value_type val : saved_domain.saved_state()) {
            m_state.set_value(parameter_t{ depth }, val);
            if (try_assignments(depth + 1)) {
                return true;
            }
        }
        return false;
    }

    uniform_constraint_state<domain_type, std::uint8_t> m_state;
    std::vector<binary_clause<constraint_state_t>> m_clauses;
};
}// namespace solver

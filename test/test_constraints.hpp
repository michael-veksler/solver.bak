#pragma once

#include <set>
#include <stdexcept>

namespace solver::test {

template<bool only_watches_trigger_ = true> struct test_constraint_state;

class test_bool_parameter
{
  public:
    test_bool_parameter(unsigned id, void * = nullptr) : m_variable_id(id) {}
    template<bool> friend struct test_constraint_state;

  private:
    unsigned m_variable_id;
};

template<bool only_watches_trigger_> struct test_constraint_state
{
    //  static constexpr bool only_watches_trigger = true;
    using param_index_t = uint8_t;
    static constexpr bool only_watches_trigger = only_watches_trigger_;
    using domain_type = std::set<bool>;
    using parameter_t = test_bool_parameter;

    [[nodiscard]] bool get_value(test_bool_parameter param) const
    {
        if (m_variables[param.m_variable_id].size() != 1) {
            throw std::domain_error("Get value only works on singleton domains");
        }
        return *begin(m_variables[param.m_variable_id]);
    }
    [[nodiscard]] const std::set<bool> &get_domain(test_bool_parameter param) const
    {
        return m_variables[param.m_variable_id];
    }

    void register_watch(parameter_t param) { m_watches.insert(param.m_variable_id); }
    void unregister_watch(parameter_t param) { m_watches.erase(param.m_variable_id); }
    std::vector<std::set<bool>> m_variables;
    std::set<unsigned> m_watches;
};
}// namespace solver::test

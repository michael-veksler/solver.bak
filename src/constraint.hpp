#pragma once
#include <concepts>
#include <cstdint>
#include <vector>

namespace solver {

template<typename T>
concept constraint_state = requires(T a)
{
    typename T::param_index_t;
    typename T::parameter_t;
    typename T::domain_type;
    typename T::domain_type::value_type;
    std::integral<typename T::param_index_t>;
    {
        a.get_value(std::declval<typename T::parameter_t>())
        } -> std::same_as<typename T::domain_type::value_type>;
    {
        a.get_domain(std::declval<typename T::parameter_t>())
        } -> std::convertible_to<typename T::domain_type>;
    { a.set_value(std::declval<typename T::parameter_t>(), std::declval<typename T::domain_type::value_type>()) };
    { a.set_domain(std::declval<typename T::parameter_t>(), std::declval<typename T::domain_type>()) };
};

template<typename T>
concept propagating_constraint_state = constraint_state<T> && requires(T a)
{
    T::only_watches_trigger;
    { a.register_watch(std::declval<typename T::parameter_t>()) };
    { a.unregister_watch(std::declval<typename T::parameter_t>()) };
};

template<typename T>
concept bool_domain = std::is_same_v<typename T::domain_type::value_type, bool>;


enum class propagation_result_t : int8_t { UNSAT, CONSISTENT, SAT };

template<typename T>
concept constraint = requires(T c)
{
    typename T::state_t;
    constraint_state<typename T::state_t>;
    {
        c.is_satisfied(std::declval<typename T::state_t>())
        } -> std::same_as<bool>;
};

template<typename T>
concept propagating_constraint = requires(T a)
{
    constraint<T>;
    {
        a.propagate(std::declval<typename T::state>(), 1)
        } -> std::same_as<propagation_result_t>;
};

}// namespace solver

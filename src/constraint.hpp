#pragma once
#include <cassert>
#include <concepts>
#include <cstdint>
#include <vector>

namespace solver {

template<typename T>
concept constraint_state = requires(T a)
{
    // {
    //     std::as_const(T::only_watches_trigger)
    //     } -> std::same_as<const bool &>;
    typename T::param_index_t;
    typename T::parameter_t;
    typename T::domain_type;
    typename T::domain_type::value_type;
    T::only_watches_trigger;
    { a.register_watch(std::declval<typename T::parameter_t>()) };
    { a.unregister_watch(std::declval<typename T::parameter_t>()) };
    //{ T::only_watches_trigger } -> std::same_as<const bool>;
    std::integral<typename T::param_index_t>;
    {
        a.get_value(std::declval<typename T::parameter_t>())
        } -> std::same_as<typename T::domain_type::value_type>;
    {
        a.get_domain(std::declval<typename T::parameter_t>())
        } -> std::convertible_to<typename T::domain_type>;
};

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

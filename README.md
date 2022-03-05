# A generic constaint solver in C++

[![ci](https://github.com/michael-veksler/solver/actions/workflows/ci.yml/badge.svg)](https://github.com/michael-veksler/solver/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/cpp-best-practices/cpp_boilerplate_project/branch/main/graph/badge.svg)](https://codecov.io/gh/michael-veksler/solver)
[![Language grade: C++](https://img.shields.io/lgtm/grade/cpp/github/cpp-best-practices/cpp_boilerplate_project)](https://lgtm.com/projects/g/michael-veksler/solver/context:cpp)


## About the constaint solver

Constraint solver's goal is to solve
[Constraint Satisfaction Problems [(CSPs)](https://en.wikipedia.org/wiki/Constraint_satisfaction_problem). 
The goal of this solver is to be highly configurable, letting a programmer mix-and-match different
representation and solving strategies. Like C++ standard library, the configurability is intended to have
an extremely low abstraction cost.

## Development mode

This is project is based on Jason Turner's
[c++ starter project](https://github.com/cpp-best-practices/cpp_starter_project).
As such, it includes the following features (copied from Jason's page):

By default (collectively known as `ENABLE_DEVELOPER_MODE`)

 * Address Sanitizer and Undefined Behavior Sanitizer enabled where possible
 * Warnings as errors
 * clang-tidy and cppcheck static analysis
 * conan for dependencies

It includes

 * a basic CLI example
 * examples for fuzz, unit, and constexpr testing
 * large github action testing matrix

It requires

 * cmake
 * conan
 * a compiler

## Testing

See [Catch2 tutorial](https://github.com/catchorg/Catch2/blob/master/docs/tutorial.md)

## Fuzz testing

See [libFuzzer Tutorial](https://github.com/google/fuzzing/blob/master/tutorial/libFuzzerTutorial.md)



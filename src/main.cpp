#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <set>

#include "dimacs_parser.hpp"
#include "exhaustive_solver.hpp"
#include <docopt/docopt.h>
#include <spdlog/spdlog.h>

// This file will be generated automatically when you run the CMake configuration step.
// It creates a namespace called `myproject`.
// You can modify the source template at `configured_files/config.hpp.in`.
#include <internal_use_only/config.hpp>

static constexpr auto USAGE =
    R"(Solve problem

    Usage:
          solve --exhaustive --dimacs=FILE
          solve (-h | --help)
          solve --version
    Options:
          -h --help       Show this screen.
          --version       Show version.
          --exhaustive    Use exhaustive-search strategy, which traverses all
                          dom_size ** num_variables search space.
          --dimacs=fILE   DIMACS formatted CNF file.
)";

int main(int argc, const char **argv)
{
    try {
        std::map<std::string, docopt::value> args = docopt::docopt(USAGE,
            { std::next(argv), std::next(argv, argc) },
            true,// show help if requested
            fmt::format("{} {}",
                myproject::cmake::project_name,
                myproject::cmake::project_version));// version string, acquired from config.hpp via CMake


        fmt::print("c solving {}\n", args.at("--dimacs").asString());
        std::ifstream dimacs_stream{ args.at("--dimacs").asString() };
        if (!dimacs_stream) {
            fmt::print(
                "c Could not open file {}\n", args.at("--dimacs").asString(), std::filesystem::current_path().string());
            fmt::print("s UNKNOWN\n");
            return 1;
        }
        using domain_t = std::set<bool>;
        domain_t unset = { false, true };
        using solver_t = solver::exhaustive_solver<solver::uniform_constraint_state<domain_t, std::uint8_t>>;
        std::unique_ptr<solver_t> solver_ptr;
        auto constructor = [&](unsigned variables, unsigned constraints) {
            std::ignore = constraints;
            solver_ptr = std::make_unique<solver_t>(variables, unset);
        };
        auto add_clause = [&](const std::vector<int> &literals) { solver_ptr->add_clause(literals); };
        solver::parse_dimacs(dimacs_stream, constructor, add_clause);
        if (!solver_ptr) {
            fmt::print("s UNKNOWN\n");
            return 1;
        }
        if (solver_ptr->solve()) {
            fmt::print("s SATISFIABLE\n");
            fmt::print("v ");
            for (unsigned i = 0; i != solver_ptr->num_variables(); ++i) {
                if (solver_ptr->get_value(i)) {
                    fmt::print("{} ", i + 1);
                } else {
                    fmt::print("-{} ", i + 1);
                }
            }
            fmt::print("0\n");
        } else {
            fmt::print("s UNSATISFIABLE\n");
        }
    } catch (const std::exception &e) {
        fmt::print("c Unhandled exception in main: {}\n", e.what());
        fmt::print("s UNKNOWN\n");
    }
}

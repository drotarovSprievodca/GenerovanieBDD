#pragma once

#include <iostream>
#include <libteddy/reliability.hpp>

class Strategy {
    protected:
        bool use_var_reordering_heuristics;
    public:
        Strategy(bool use_var_reordering_heuristics) : use_var_reordering_heuristics(use_var_reordering_heuristics) {};
        virtual ~Strategy() = default;
        virtual double process_function(teddy::bss_manager& default_manager, int number_of_vars, teddy::pla_file* pla, int which_function) = 0;
        virtual std::string to_string() = 0;
};
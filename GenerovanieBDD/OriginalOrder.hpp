#pragma once

#include "Strategy.hpp"

class OriginalOrder : public Strategy {
    public:
        OriginalOrder(bool use_var_reordering_heuristics);
        ~OriginalOrder();
        double process_function(teddy::bss_manager& default_manager, int number_of_vars, teddy::pla_file* pla, CSVOutput* csv, int which_function) override;
        std::string to_string() override;
};
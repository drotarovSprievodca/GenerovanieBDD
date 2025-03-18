#pragma once

#include "Strategy.hpp"

class OriginalOrder : public Strategy {
    public:
        OriginalOrder(bool use_var_reordering_heuristics, bool generate_graph_before_order, bool generate_graph_after_order);
        ~OriginalOrder();
        void process_function(teddy::bss_manager& default_manager, int number_of_vars, teddy::pla_file* pla, CSVOutput* csv, int which_function, std::string file_name_without_extension) override;
        std::string to_string() override;
};
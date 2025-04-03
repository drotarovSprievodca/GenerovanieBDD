#pragma once

#include "DerivativesBasedOrder.hpp"

class DerivativesBasedOrderDT : public DerivativesBasedOrder {

public:
    DerivativesBasedOrderDT(bool use_var_reordering_heuristics, bool ascending, bool generate_graph_before_order, bool generate_graph_after_order);
    ~DerivativesBasedOrderDT();
    void get_order_from_ODT(teddy::bss_manager& default_manager, std::vector<int>& order_of_vars_from_ODT, teddy::bss_manager::diagram_t& diagram, int which_diagram);
    void process_function(teddy::bss_manager& default_manager, int number_of_vars, teddy::pla_file* pla, CSVOutput* csv, int which_function, std::string file_name_without_extension) override;
    std::string to_string() override;
    std::string get_strategy_name() override;
};
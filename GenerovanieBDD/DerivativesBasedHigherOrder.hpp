#pragma once

#include "DerivativesBasedOrder.hpp"

class DerivativesBasedHigherOrder : public DerivativesBasedOrder {

public:
    DerivativesBasedHigherOrder(bool use_var_reordering_heuristics, bool ascending);
    ~DerivativesBasedHigherOrder();
    void get_order_from_higher_order_derivatives(teddy::bss_manager& default_manager, std::vector<int>& order_of_vars_from_HOD, teddy::bss_manager::diagram_t& diagram, int which_diagram);
    void process_function(teddy::bss_manager& default_manager, int number_of_vars, teddy::pla_file* pla, CSVOutput* csv, int which_function) override;
    std::string to_string() override;
};
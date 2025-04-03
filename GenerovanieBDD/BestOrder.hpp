#pragma once

#include "Strategy.hpp"
#include <limits>

class BestOrder : public Strategy {

public:
    BestOrder(bool generate_graph_before_order, bool generate_graph_after_order);
    ~BestOrder();
    void process_function(teddy::bss_manager& default_manager, int number_of_vars, teddy::pla_file* pla, CSVOutput* csv, int which_function, std::string file_name_without_extension) override;
    std::string to_string() override;
    std::string get_strategy_name() override;
};
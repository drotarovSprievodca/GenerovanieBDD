#include <libteddy/core.hpp>
#include <cassert>
#include <fstream>
#include <iostream>
#include <vector>
#include <array>
#include <libteddy/details/types.hpp>
#include <libteddy/details/pla_file.hpp>
#include "libteddy/details/reliability_manager.hpp"
#include <libteddy/reliability.hpp>

int main()
{
    std::optional<teddy::pla_file> pla_file = teddy::pla_file::load_file("C:\\Users\\DELL\\git\\Diplomka\\GenerovanieBDD\\PLA\\bw.pla"); //9sym.pla  alu4.pla  misex3.pla  5xp1.pla  bw.pla

    if (pla_file.has_value()) {

        teddy::pla_file* pla = &pla_file.value();

        int number_of_vars = pla->get_variable_count();

        std::cout << "Number of function in file: " << pla->get_function_count() << std::endl;
        std::cout << "Number of variables in the file: " << number_of_vars << std::endl;
        std::cout << "Number of lines in the file: " << pla->get_line_count() << std::endl;

        std::vector<std::string> const& input_labels = pla->get_input_labels();
        if (input_labels.size() > 0) {
            std::cout << "Input labels: " << std::endl;
            for (auto label : input_labels) {
                std::cout << label << " ";
            }
            std::cout << "" << std::endl;
        }
        
        std::vector<std::string> const& output_labels = pla->get_output_labels();
        if (output_labels.size() > 0) {
            std::cout << "Output labels: " << std::endl;
            for (auto label : output_labels) {
                std::cout << label << " ";
            }
            std::cout << "" << std::endl;
        }

        /*
        std::cout << "Lines: " << std::endl;
        std::vector<teddy::pla_file::pla_line> const& lines = pla->get_lines();
        for (auto line : lines) {
            for (int i = 0; i < line.cube_.size(); ++i) {
                std::cout << line.cube_.get(i);
            }
            std::cout << " ";
            for (int i = 0; i < line.fVals_.size(); ++i) {
                std::cout << line.fVals_.get(i);
            }
            std::cout << "\n";
        }
        std::cout << "" << std::endl;
        */
        
        std::vector<teddy::int32> order = std::vector<teddy::int32>(number_of_vars);
        order[0] = 4;
        order[1] = 3;
        order[2] = 0;
        order[3] = 1;
        order[4] = 2;

        teddy::bss_manager manager(number_of_vars, 1'000);//, order);
        teddy::bss_manager::diagram_t diagram = manager.from_pla(*pla, teddy::fold_type::Tree)[0]; //first function
        //teddy::bss_manager
        

        /*
        auto vector_of_diagrams = manager.from_pla(*pla, teddy::fold_type::Tree);
        int poradie = 0;
        for (auto diagram : vector_of_diagrams) {
            std::ofstream ofst("diag_" + std::to_string(poradie) + ".dot");
            manager.to_dot_graph(ofst, diagram);
            poradie++;
        }
        */

        // prints diagram to console
        // manager.to_dot_graph(std::cout, diagram);
        // prints diagram to file
        std::ofstream ofst_diag("diagram.dot");
        manager.to_dot_graph(ofst_diag, diagram);
        // converts .dot file to .png file 
        // .\dot.exe -Tpng diagram.dot -o output.png

        // returns number of nodes in the diagram including terminal nodes
        int node_count = manager.get_node_count(diagram);
        std::cout << "Number of nodes in diagram (including terminal nodes): " << node_count <<  std::endl;

        std::cout << "Number of variables: " << manager.get_var_count() << std::endl;

        std::cout << "Order before reorder:" << std::endl;
        node_count = manager.get_node_count(diagram);
        std::cout << "Number of nodes in diagram (including terminal nodes) before reordering: " << node_count << std::endl;
        std::vector<int> order_before = manager.get_order();
        for (auto o : order_before) {
            std::cout << o << std::endl;
        }

        manager.set_auto_reorder(false); // Enables or disables automatic variable reordering.

        manager.force_reorder(); // Runs variable reordering heuristic.

        std::cout << "Order after reorder:" << std::endl;
        node_count = manager.get_node_count(diagram);
        std::cout << "Number of nodes in diagram (including terminal nodes) after reordering: " << node_count << std::endl;
        std::vector<int> order_after = manager.get_order();
        for (auto o : order_after) {
            std::cout << o << std::endl;
        }

        std::ofstream ofst_diag_after("diagram_after.dot");
        manager.to_dot_graph(ofst_diag_after, diagram);




        /*
        std::cout << "Domains:" << std::endl;
        std::vector<int> domains = manager.get_domains();
        for (auto d : domains) {
            std::cout << d << std::endl;
        }
        */

        // variable x0 decreases from 1 -> 0 while function decreases from 1 -> 0
        teddy::bss_manager::diagram_t dpbd = manager.dpld({ 0, 1, 0 }, teddy::dpld::type_1_decrease(1), diagram);
        //std::ofstream ofst_dpbd("dpbd.dot");
        //manager.to_dot_graph(ofst_dpbd, dpbd);

        // variables with dpbd == 1
        std::vector<std::vector<int>> vars_with_ones = manager.satisfy_all<std::vector<int>>(1, dpbd);
        int number_of_ones_in_dpbd = vars_with_ones.size();
        std::cout << "Number of ones in dpbd: " << number_of_ones_in_dpbd << std::endl;
    
        /*
        for (auto var : vars_with_ones) {
            for (int i = 0; i < number_of_vars; ++i) {
                std::cout << var[i];
            }
            std::cout << " ";
        }
        std::cout << "" << std::endl;
        */

        std::cout << "" << std::endl;

        // variables with dpbd == 0
        std::vector<std::vector<int>> vars_with_zeros = manager.satisfy_all<std::vector<int>>(0, dpbd);
        int number_of_zeros_in_dpbd = vars_with_zeros.size();
        std::cout << "Number of zeros in dpbd: " << number_of_zeros_in_dpbd << std::endl;

        /*
        for (auto var : vars_with_zeros) {
            for (int i = 0; i < number_of_vars; ++i) {
                std::cout << var[i];
            }
            std::cout << " ";
        }
        std::cout << "" << std::endl;
        */
    }
}

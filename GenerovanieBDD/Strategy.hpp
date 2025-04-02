#pragma once

#include <iostream>
#include <libteddy/reliability.hpp>

#include "CSVOutput.hpp"

class Strategy {
    protected:
        bool use_var_reordering_heuristics;
        bool generate_graph_before_order;
        bool generate_graph_after_order;
        std::string working_directory_for_graphs = "C:\\Users\\DELL\\git\\Diplomka\\GenerovanieBDD";
        std::string graphviz_exe_path = "C:\\\"Program Files\"\\Graphviz\\bin\\dot.exe";

    public:
        Strategy(bool use_var_reordering_heuristics, bool generate_graph_before_order, bool generate_graph_after_order) : 
            use_var_reordering_heuristics(use_var_reordering_heuristics),
            generate_graph_before_order(generate_graph_before_order),
            generate_graph_after_order(generate_graph_after_order)
        {};
        virtual ~Strategy() = default;
        virtual void process_function(teddy::bss_manager& default_manager, int number_of_vars, teddy::pla_file* pla, CSVOutput* csv, int which_function, std::string file_name_without_extension) = 0;
        virtual std::string to_string() = 0;

        std::string generate_dot_file(std::string file_name, teddy::bss_manager::diagram_t& diagram, teddy::bss_manager& manager) {
            std::string diagram_dot = working_directory_for_graphs + "\\diagrams\\dot_diag\\" + file_name + ".dot";
            std::ofstream ofst_diag(diagram_dot);
            manager.to_dot_graph(ofst_diag, diagram);
            return diagram_dot;
        }

        bool convert_dot_file_to_png(std::string file_name, std::string diagram_dot) {
            std::string diagram_png = working_directory_for_graphs + "\\diagrams\\png_diag\\" + file_name + ".png";
            std::string command = graphviz_exe_path + " -Tpng " + diagram_dot + " -o " + diagram_png;
            return system(command.c_str()) == 0;
        }

        bool generate_diagram(std::string file_name_without_extension, teddy::bss_manager::diagram_t& diagram, teddy::bss_manager& manager, int which_function) {
            std::string file_name = file_name_without_extension + "_diagram_" + std::to_string(which_function);
            // generate_dot_file() needs to be returned for proper creating of dot file
            std::string dot_file_path = generate_dot_file(file_name, diagram, manager);
            return convert_dot_file_to_png(file_name, dot_file_path);
        }
};


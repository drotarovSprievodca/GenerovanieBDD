#pragma once

#include <iostream>
#include <libteddy/reliability.hpp>
#include "json.hpp"
#include <chrono>

#include "CSVOutput.hpp"

class Strategy {
    protected:
        bool use_var_reordering_heuristics;
        bool generate_graph_before_order;
        bool generate_graph_after_order;
        std::string working_directory_for_graphs;
        std::string graphviz_exe_path;
        double timer; // lasting of strategy proccessing in seconds

    public:
        Strategy(bool use_var_reordering_heuristics, bool generate_graph_before_order, bool generate_graph_after_order) : 
            use_var_reordering_heuristics(use_var_reordering_heuristics),
            generate_graph_before_order(generate_graph_before_order),
            generate_graph_after_order(generate_graph_after_order),
            timer(0.0)
        {
            std::ifstream config_file("config.json");
            if (!config_file.is_open()) {
                std::cerr << "Could not open config file!" << std::endl;
            }

            nlohmann::json config;
            config_file >> config;
            config_file.close();

            std::string working_directory_for_graphs = config.value("working_directory_for_graphs", "");
            std::string graphviz_exe_path = config.value("graphviz_exe_path", "");
        };
        virtual ~Strategy() = default;
        virtual void process_function(teddy::bss_manager& default_manager, int number_of_vars, teddy::pla_file* pla, CSVOutput* csv, int which_function, std::string file_name_without_extension) = 0;
        virtual std::string to_string() = 0;
        virtual std::string get_strategy_name() = 0;

        std::string generate_dot_file(std::string file_name, teddy::bss_manager::diagram_t& diagram, teddy::bss_manager& manager) {
            std::string diagram_dot = working_directory_for_graphs + "\\diagrams\\dot_diag\\" + file_name + ".dot";
            std::ofstream ofst_diag(diagram_dot);
            manager.to_dot_graph(ofst_diag, diagram);
            return diagram_dot;
        }

        std::string get_working_directory_for_graphs() {
            return working_directory_for_graphs;
        }

        std::string get_graphviz_exe_path() {
            return graphviz_exe_path;
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

        void add_to_timer(double time_to_add) {
            timer += time_to_add;
        }

        double get_timer() {
            return timer;
        }

        void clear_timer() {
            timer = 0.0;
        }
};


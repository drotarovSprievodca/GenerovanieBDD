#include <libteddy/core.hpp>
#include <cassert>
#include <fstream>
#include <iostream>
#include <vector>
#include <array>
#include <libteddy/details/pla_file.hpp>

int main()
{
    std::optional<teddy::pla_file> pla_file = teddy::pla_file::load_file("C:\\Users\\DELL\\git\\Diplomka\\GenerovanieBDD\\PLA\\misex3.pla");

    if (pla_file.has_value()) {
        teddy::pla_file* pla = &pla_file.value();

        std::cout << "Number of function in file: " << pla->get_function_count() << std::endl;
        std::cout << "Number of variables in the file: " << pla->get_variable_count() << std::endl;
        std::cout << "Number of lines in the file: " << pla->get_line_count() << std::endl;

        std::cout << "Input labels: " << std::endl;
        std::vector<std::string> const& input_labels = pla->get_input_labels();
        for (auto label : input_labels) {
            std::cout << label << " ";
        }
        std::cout << "" << std::endl;

        std::cout << "Output labels: " << std::endl;
        std::vector<std::string> const& output_labels = pla->get_output_labels();
        for (auto label : output_labels) {
            std::cout << label << " ";
        }
        std::cout << "" << std::endl;

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
    }
}


set(CMAKE_CXX_STANDARD 20)

find_package(
    TeDDy REQUIRED
)

add_executable(
    GenerovanieBDD "GenerovanieBDD.cpp"
 "BDDStatisticsGenerator.hpp" "BDDStatisticsGenerator.cpp" "Strategy.hpp" "DerivativesBasedOrder.hpp" "DerivativesBasedOrder.cpp" "OriginalOrder.hpp" "OriginalOrder.cpp" "RandomOrder.hpp" "RandomOrder.cpp" "CSVOutput.hpp" "CSVOutput.cpp" "EntropyBasedOrder.hpp" "EntropyBasedOrder.cpp")

target_link_libraries(
    GenerovanieBDD PRIVATE teddy::teddy
)

target_include_directories(
    GenerovanieBDD PRIVATE "C:/Program Files (x86)/TeDDy/include/"
)

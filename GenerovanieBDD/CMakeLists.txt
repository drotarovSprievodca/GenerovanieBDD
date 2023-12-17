
set(CMAKE_CXX_STANDARD 20)

find_package(
    TeDDy REQUIRED
)

add_executable(
    GenerovanieBDD "GenerovanieBDD.cpp" "GenerovanieBDD.h" 
)

target_link_libraries(
    GenerovanieBDD PRIVATE teddy::teddy
)

target_include_directories(
    GenerovanieBDD PRIVATE "C:/Program Files (x86)/TeDDy/include/"
)

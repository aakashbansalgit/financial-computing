cmake_minimum_required(VERSION 3.16)
project(financial_computing CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_library(pricing lib/pricing.cpp)

add_executable(study study/study_main.cpp)
target_link_libraries(study pricing)

add_executable(tests tests/test_main.cpp)
target_link_libraries(tests pricing)

# the original coursework programs
add_executable(barrier_option barrier-options/barrier_option.cpp barrier-options/barrier_option_main.cpp)
add_executable(trinomial_tree trinomial-tree/trinomial.cpp trinomial-tree/trinomial_main.cpp)

enable_testing()
add_test(NAME pricing_tests COMMAND tests)

cmake_minimum_required(VERSION 3.10)
project(eva01)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -O0 -ggdb -std=c++17 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")

# set(BOOST_ROOT /opt/homebrew/Cellar/boost/1.76.0)
# find_package(Boost)

# find_library(PTHREAD pthread)

include_directories(.)
# include_directories(~/user/local/include)
# link_directories(~/user/local/lib)

set(LIB_SRC
    src/log.cpp
    )

add_library(eva01 SHARED ${LIB_SRC})


set(LIBS
    eva01
    )

add_executable(test_log tests/test_log.cpp)
add_dependencies(test_log eva01)
target_link_libraries(test_log ${LIBS})

add_executable(example_logging examples/logging.cpp)
add_dependencies(example_logging eva01)
target_link_libraries(example_logging ${LIBS})

add_custom_target(test
    COMMAND test_log
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
    )

add_custom_target(example
    COMMAND example_logging
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
    )

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
cmake_minimum_required(VERSION 3.10)
project(eva01)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -O0 -ggdb -std=c++14 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")

# set(BOOST_ROOT /opt/homebrew/Cellar/boost/1.76.0)
# find_package(Boost)

find_library(PTHREAD pthread)

include_directories(.)
include_directories(~/user/local/include)
link_directories(~/user/local/lib)

set(LIB_SRC
    src/log.cpp
    src/thread.cpp
    src/util.cpp
    src/semaphore.cpp
    src/fiber.cpp
    src/timer.cpp
    src/scheduler.cpp
    src/iomanager.cpp
    src/noncopyable.h
    src/mutex.h
    )

add_library(eva01 SHARED ${LIB_SRC})

set(LIBS
    eva01
    pthread
    )

# Tests
add_executable(test_log tests/test_log.cpp)
add_dependencies(test_log eva01)
target_link_libraries(test_log ${LIBS})

add_executable(test_thread tests/test_thread.cpp)
add_dependencies(test_thread eva01)
target_link_libraries(test_thread ${LIBS})

add_executable(test_mutex tests/test_mutex.cpp)
add_dependencies(test_mutex eva01)
target_link_libraries(test_mutex ${LIBS})

add_executable(test_fiber tests/test_fiber.cpp)
add_dependencies(test_fiber eva01)
target_link_libraries(test_fiber ${LIBS})

add_executable(test_scheduler tests/test_scheduler.cpp)
add_dependencies(test_scheduler eva01)
target_link_libraries(test_scheduler ${LIBS})

add_executable(test_iomanager tests/test_iomanager.cpp)
add_dependencies(test_iomanager eva01)
target_link_libraries(test_iomanager ${LIBS})

add_custom_target(test
    COMMAND test_log -d
    COMMAND test_thread -d
    COMMAND test_mutex -d
    COMMAND test_fiber -d
    COMMAND test_scheduler -d
    COMMAND test_iomanager -d
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
    )

# Examples
add_executable(example_logging examples/logging.cpp)
add_dependencies(example_logging eva01)
target_link_libraries(example_logging ${LIBS})

add_executable(example_thread examples/thread.cpp)
add_dependencies(example_thread eva01)
target_link_libraries(example_thread ${LIBS})

add_custom_target(run
    COMMAND example_logging
    COMMAND example_thread
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
    )

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

#enable_testing()
#add_test(NAME TestLogging COMMAND test_log)

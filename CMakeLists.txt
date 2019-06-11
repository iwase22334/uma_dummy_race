cmake_minimum_required(VERSION 2.8)
project(uma_dummy_race CXX)

set(CMAKE_CXX_STANDARD, 14)
set(CMAKE_CXX_FLAGS "-Wextra -Wall")

# load psql
set(PQXX /usr/local/include/pqxx)
find_library(PQXX_LIB pqxx)
find_library(PQ_LIB pq)

# load boost
set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME     ON)

find_package(Boost 1.36.0 REQUIRED
             COMPONENTS date_time system regex coroutine)

message("## Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}") 
message("## Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}") 

include_directories(${Boost_INCLUDE_DIR})

# Build
file(GLOB DUMMY_RACE_SRC
    "src/*.hpp"
    "src/*.cpp"
)
add_executable(dummy_race ${DUMMY_RACE_SRC})
target_link_libraries(dummy_race ${Boost_LIBRARIES} ${PQXX_LIB} ${PQ_LIB})


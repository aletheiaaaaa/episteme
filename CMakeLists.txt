cmake_minimum_required(VERSION 3.10.0)
project(valhalla)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-mavx2 -g -fsanitize=address -fsanitize=undefined")
set(CMAKE_CXX_FLAGS_RELEASE "-mavx2 -O3 -flto")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 23)
set(SRC "${CMAKE_SOURCE_DIR}/src")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_executable(episteme
    "${SRC}/chess/position.cpp" 
    "${SRC}/chess/move.cpp" 
    "${SRC}/chess/movegen.cpp" 
    "${SRC}/chess/perft.cpp" 
    "${SRC}/chess/zobrist.cpp" 
    "${SRC}/evaluation/evaluate.cpp" 
    "${SRC}/evaluation/nnue.cpp" 
    "${SRC}/search/search.cpp" 
    "${SRC}/search/ttable.cpp"
    "${SRC}/uci/uci.cpp" 
    "${SRC}/main.cpp"
)

set(EVAL_BIN "${CMAKE_SOURCE_DIR}/episteme_dev_net.bin")

target_compile_definitions(episteme PRIVATE EVALFILE="${EVAL_BIN}")
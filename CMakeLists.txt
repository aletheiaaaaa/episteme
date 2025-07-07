cmake_minimum_required(VERSION 3.10.0)
project(episteme)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-mavx2 -g -fsanitize=address,undefined,leak")
set(CMAKE_CXX_FLAGS_RELEASE "-mavx2 -O3 -flto=auto")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 23)
set(SRC "${CMAKE_SOURCE_DIR}/src")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_executable(episteme
    "${SRC}/engine/chess/position.cpp" 
    "${SRC}/engine/chess/move.cpp" 
    "${SRC}/engine/chess/movegen.cpp" 
    "${SRC}/engine/chess/perft.cpp" 
    "${SRC}/engine/chess/zobrist.cpp" 
    "${SRC}/engine/evaluation/evaluate.cpp" 
    "${SRC}/engine/evaluation/nnue.cpp" 
    "${SRC}/engine/search/search.cpp" 
    "${SRC}/engine/search/ttable.cpp"
    "${SRC}/engine/uci/uci.cpp" 
    "${SRC}/utils/datagen.cpp"
    "${SRC}/utils/format.cpp"
    "${SRC}/main.cpp"
)

set(EVAL_BIN "${CMAKE_SOURCE_DIR}/64_v2.bin")

target_compile_definitions(episteme PRIVATE EVALFILE="${EVAL_BIN}")
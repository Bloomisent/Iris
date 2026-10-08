cmake_minimum_required(VERSION 3.15)
project(Iris VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_SOURCES
    src/main.c
    src/lexer.c
    src/token.c
    src/AST.c
    src/parser.c
    src/scope.c
    src/visitor.c
    src/window.c
)

add_executable(Iris ${PROJECT_SOURCES})
target_include_directories(Iris PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

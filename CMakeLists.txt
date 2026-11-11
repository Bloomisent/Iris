cmake_minimum_required(VERSION 3.15)
project(Iris VERSION 4.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(PROJECT_SOURCES
    src/main.c
    src/lexer.c
    src/token.c
    src/AST.c
    src/parser.c
    src/scope.c
    src/builtin.c
    src/visitor.c
    src/value.c
    src/chunk.c
    src/compiler.c
    src/disasm.c
    src/VM.c
)

add_executable(Iris ${PROJECT_SOURCES})
target_include_directories(Iris PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}src/include)

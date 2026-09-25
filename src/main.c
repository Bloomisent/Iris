#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "include/lexer.h"
#include "include/parser.h"
#include "include/visitor.h"

int current_line = 1;
int current_col = 1;

Scope_T* g_iris_gc_root_scope = NULL;
char* g_iris_library_dir;

char version[] = "v3.0.4";

char* read_file_to_string(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        perror("Error seeking file");
        fclose(file);
        return NULL;
    }

    long file_size = ftell(file);
    if (file_size < 0) {
        perror("Error getting file size");
        fclose(file);
        return NULL;
    }
    rewind(file);
    char *buffer = malloc(file_size + 1);
    if (buffer == NULL) {
        perror("Memory allocation failed");
        fclose(file);
        return NULL;
    }
    size_t bytes_read = fread(buffer, 1, file_size, file);
    buffer[bytes_read] = '\0';
    fclose(file);
    return buffer;
}

bool ends_with(const char *str, const char *suffix) {
    if (!str || !suffix) return false;

    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    if (suffix_len > str_len) return false;
    return strcmp(str + (str_len - suffix_len), suffix) == 0;
}

int main(int argc, char *argv[]) {

    g_iris_library_dir = join_path(get_directory(argv[0]), "src\\libs\\");

    if (argc < 2) {
        printf("You did not input a path.\n");
        exit(1);
    };

    if (argv[1] != NULL) {
        if (strcmp(argv[1], "--version") == 0){
            printf("Iris Version: %s", version);
            exit(101);
            return 1;
        };
        if (!ends_with(argv[1], ".iris")) {
            printf("Invalid file type, try a '.iris' file extension.\n");
            exit(101);
            return 1;
        }

        char* contents = read_file_to_string(argv[1]);

        if (contents != NULL) {
            Lexer_T* Lexer = Init_Lexer(contents);
            Parser_T* Parser = Init_Parser(Lexer);
            Parser->current_dir = get_directory(argv[1]);
            g_iris_gc_root_scope = Parser->Scope;

            AST_T* root = Parser_Parse(Parser, Parser->Scope);
            Visitor_T* visitor = Init_Visitor();
            AST_T* visit = NULL;
            if (argc < 3 || strcmp(argv[2], "--grammar") != 0) {
                visit = Visitor_Visit(visitor, root);
                Visitor_Clean(visitor, root);
            };
            free(visit);
            free(Lexer);
            free(Parser);
            free(contents);
        } else {
            printf("FILE IS NULL.\n");
        }
        return 0;
    } else {
        printf("Invalid arguments, argument 1 is NULL or not a valid path.\n");
        exit(100);
    }
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "vdbe.c"

typedef struct
{
    char* buffer;
    size_t buffer_length;
    ssize_t input_length;
}InputBuffer;

typedef enum{
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
}MetaCommandResult;

typedef enum{
    PREPARE_SUCCESS, 
    PREPARE_SYNTAX_ERROR,
    PREPARE_UNRECOGNIZED_STATEMENT,
    PREPARE_STRING_TOO_LONG,
    PREPARE_NEGATIVE_ID
}PrepareResult;


InputBuffer* new_input_buffer(){
    InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;
    return input_buffer;
}

void print_prompt(){
    printf("db > ");
}

void print_leaf_node(void* node) {
    uint32_t num_cells = *leaf_node_num_cells(node);
    printf("leaf (size %d)\n", num_cells);
    for (uint32_t i = 0; i < num_cells; i++) {
        uint32_t key = *leaf_node_key(node, i);
        printf("  - %d : %d\n", i, key);
    }
}

void read_input(InputBuffer* input_buffer){
    ssize_t bytes_read = getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);

    if (bytes_read <= 0){
        printf("An error has occured when reading input.");
        exit(EXIT_FAILURE);
    }

    input_buffer->input_length = bytes_read - 1;
    input_buffer->buffer[bytes_read - 1] = 0;
}

void close_input_buffer(InputBuffer* input_buffer){
    free(input_buffer);
}

Table* db_open(const char* filename){
    Pager* pager = pager_open(filename);

    Table* table = (Table*)malloc(sizeof(Table));
    table->pager = pager;
    table->root_page_num = 0;

    if (pager->num_pages == 0){ // new database file
        void* root_node = get_page(pager, 0);
        initialize_leaf_node(root_node);
        set_node_root(root_node, true);
    }
    return table;
}

void db_close(Table* table) {
    Pager* pager = table->pager;

    for (uint32_t i = 0; i < pager->num_pages; i++) {
        if (pager->pages[i] == NULL) {
            continue;
        }
        pager_flush(pager, i);
        free(pager->pages[i]);
        pager->pages[i] = NULL;
    }

    int result = close(pager->file_descriptor);
    if (result == -1) {
        printf("Error closing db file.\n");
        exit(EXIT_FAILURE);
    }
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        void* page = pager->pages[i];
        if (page) {
            free(page);
            pager->pages[i] = NULL;
        }
    }
    free(pager);
    free(table);
}

MetaCommandResult execute_meta_command(InputBuffer* input_buffer, Table* table){
    if(strcmp(input_buffer->buffer, ".exit") == 0){
        close_input_buffer(input_buffer);
        db_close(table);
        exit(EXIT_SUCCESS);
    }
    else if (strcmp(input_buffer->buffer, ".btree") == 0) {
        printf("Tree:\n");
        print_leaf_node(get_page(table->pager, 0));
        return META_COMMAND_SUCCESS;
    }
    else{
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

PrepareResult prepare_insert(InputBuffer* input_buffer, Statement* statement) {
    statement->type = STATEMENT_INSERT;

    char* keyword = strtok(input_buffer->buffer, " ");
    char* id_string = strtok(NULL, " ");
    char* student_name = strtok(NULL, " ");
    char* email = strtok(NULL, " ");

    if (id_string == NULL || student_name == NULL || email == NULL) {
        return PREPARE_SYNTAX_ERROR;
    }

    int id = atoi(id_string);
    if (id < 0)
        return PREPARE_NEGATIVE_ID;
    if (strlen(student_name) > COLUMN_STUDENT_NAME) {
        return PREPARE_STRING_TOO_LONG;
    }
    if (strlen(email) > COLUMN_EMAIL_SIZE) {
        return PREPARE_STRING_TOO_LONG;
    }

    statement->row_to_insert.id = id;
    strcpy(statement->row_to_insert.student_name, student_name);
    strcpy(statement->row_to_insert.email, email);

    return PREPARE_SUCCESS;
}


PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement){
    if (strncmp(input_buffer->buffer, "insert", 6) == 0){
        statement->type = STATEMENT_INSERT;
        return prepare_insert(input_buffer, statement);
    }

    if (strcmp(input_buffer->buffer, "select") == 0){   
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }
    
    return PREPARE_UNRECOGNIZED_STATEMENT;
}

int main(int argc, char* argv[]){
    if (argc < 2){
        printf("Must supply a database filename");
        exit(EXIT_FAILURE);
    }
    char* filename = argv[1];
    Table* table = db_open(filename);

    InputBuffer* input_buffer = new_input_buffer();
    while(true){
        print_prompt();
        read_input(input_buffer);

        if (input_buffer->buffer[0] == '.'){
            switch (execute_meta_command(input_buffer, table)){
                case (META_COMMAND_SUCCESS):
                    continue;
                case (META_COMMAND_UNRECOGNIZED_COMMAND):
                    printf("Unrecognized command '%s'.\n", input_buffer->buffer);
                    continue;
            }
        }

        Statement statement;
        switch (prepare_statement(input_buffer, &statement)){
            case (PREPARE_SUCCESS):
                break;
            case (PREPARE_UNRECOGNIZED_STATEMENT):
                printf("Unrecognized SQL command '%s'\n", input_buffer->buffer);
                continue;
            case (PREPARE_STRING_TOO_LONG):
                printf("Input string is too long!\n");
                continue;
            case (PREPARE_SYNTAX_ERROR):
                printf("Syntax error. Parser reports an error.\n");
                continue;
            case (PREPARE_NEGATIVE_ID):
                printf("ID cannot be negative.\n");
                continue;
        }

        switch (execute_statement(&statement, table)) {
            case (EXECUTE_SUCCESS):
                printf("SQL execution is done.\n");
                break;
            case (EXECUTE_TABLE_FULL):
                printf("Error: Table full.\n");
                break;
            case (EXECUTE_DUPLICATE_KEY):
                printf("Error: Duplicate key.\n");
                break;
        }
    }
}
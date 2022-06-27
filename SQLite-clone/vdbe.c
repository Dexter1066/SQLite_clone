#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include "pager.c"
#include "Btree.c"

typedef enum{
    STATEMENT_INSERT,
    STATEMENT_SELECT,
}StatementType;

typedef struct{
    StatementType type;
    Row row_to_insert;
}Statement;

typedef enum{
    EXECUTE_SUCCESS,
    EXECUTE_TABLE_FULL
}ExecuteResult;

void print_row(Row* row){
    printf("(%d, %s, %s)\n", row->id, row->student_name, row->email);
}

ExecuteResult execute_insert(Statement* statement, Table* table){
    //printf("Num_rows is %d. Reported by insert.\n", table->num_rows);
    void* node = get_page(table->pager, table->root_page_num);
    if ((*leaf_node_num_cells(node)) >= LEAF_NODE_MAX_CELLS) {
        return EXECUTE_TABLE_FULL;
    }

    Row* row_to_insert = &(statement->row_to_insert);
    Cursor* cursor = table_end(table);

    leaf_node_insert(cursor, row_to_insert->id, row_to_insert);

    return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Statement* Statement, Table* table){
    Cursor* cursor = table_start(table);
    Row row;
    //printf("Num_rows is %d.\n", table->num_rows);
    while (!cursor->end_of_table)
    {
        deserialize_row(cursor_value(cursor), &row);
        print_row(&row);
        cursor_advance(cursor);
    }

    return EXECUTE_SUCCESS;
}

ExecuteResult execute_statement(Statement* statement, Table* table){
    switch (statement->type){
        case (STATEMENT_INSERT):
            return execute_insert(statement, table);
        case (STATEMENT_SELECT):
            return execute_select(statement, table);
    }
}
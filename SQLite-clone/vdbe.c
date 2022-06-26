#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pager.c"

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

void serialize_row(Row* source, void* destination){
    memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy(destination + STUDENT_NAME_OFFSET, &(source->student_name), STUDENT_NAME_SIZE);
    memcpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserialize_row(void* source, Row* destination){
    memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->student_name), source + STUDENT_NAME_OFFSET, STUDENT_NAME_SIZE);
    memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

ExecuteResult execute_insert(Statement* statement, Table* table){
    //printf("Num_rows is %d. Reported by insert.\n", table->num_rows);
    if (table->num_rows >= TABLE_MAX_ROWS)
        return EXECUTE_TABLE_FULL;

    Row* row_to_insert = &(statement->row_to_insert);
    serialize_row(row_to_insert, row_slot(table, table->num_rows));
    table->num_rows += 1;

    return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Statement* Statement, Table* table){
    Row row;
    //printf("Num_rows is %d.\n", table->num_rows);
    for(uint32_t i = 0; i < table->num_rows; i++)
    {
        deserialize_row(row_slot(table, i), &row);
        print_row(&row);
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
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>

#define COLUMN_STUDENT_NAME 32
#define COLUMN_EMAIL_SIZE 255
#define TABLE_MAX_PAGES 100
#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

typedef struct {
    uint32_t id;
    char student_name[COLUMN_STUDENT_NAME + 1];
    char email[COLUMN_EMAIL_SIZE + 1];
}Row;

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t STUDENT_NAME_SIZE = size_of_attribute(Row, student_name);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t STUDENT_NAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = STUDENT_NAME_OFFSET + STUDENT_NAME_SIZE;
const uint32_t ROW_SIZE = ID_OFFSET + STUDENT_NAME_SIZE + EMAIL_SIZE;

const uint32_t PAGE_SIZE = 4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

typedef struct{
    uint32_t num_rows;
    void* pages[TABLE_MAX_PAGES];
}Table;

void* row_slot(Table* table, uint32_t row_num);
Table* new_table();
void free_table();

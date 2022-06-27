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

typedef struct {
    int file_descriptor;
    uint32_t file_length;
    void* pages[TABLE_MAX_PAGES];
    uint32_t num_pages;
}Pager;

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t STUDENT_NAME_SIZE = size_of_attribute(Row, student_name);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t STUDENT_NAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = STUDENT_NAME_OFFSET + STUDENT_NAME_SIZE;
const uint32_t ROW_SIZE = ID_OFFSET + STUDENT_NAME_SIZE + EMAIL_SIZE;

const uint32_t PAGE_SIZE = 4096;

typedef struct{
    uint32_t num_rows;
    Pager* pager;
    uint32_t root_page_num;
}Table;

void serialize_row(Row* source, void* destination);
void deserialize_row(void* source, Row* destination);
void* get_page(Pager* pager, uint32_t page_num);
Pager* pager_open(const char* filename);
void pager_flush(Pager* pager, uint32_t page_num);



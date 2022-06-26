#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "pager.h"

void* row_slot(Table* table, uint32_t row_num){
    uint32_t page_num = row_num / ROW_SIZE;
    void *page = table->pages[page_num];
    if (page == NULL){
        page = table->pages[page_num] = malloc(PAGE_SIZE);
    }

    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * ROW_SIZE;
    return page + byte_offset;
}

Table* new_table(){
    Table* table = (Table*)malloc(sizeof(Table));
    table->num_rows = 0;
    for(uint32_t i = 0; i < TABLE_MAX_PAGES; i++)
        table->pages[i] = NULL;
    return table;
}

void free_table(Table* table){
    for(uint32_t i = 0; i < TABLE_MAX_PAGES; i++)
        free(table->pages[i]);
    free(table);
}


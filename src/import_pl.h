#ifndef IMPORT_PL_H
#define IMPORT_PL_H

#include <stdlib.h>
#include <string.h>

#include "logger.h"

#define LINE_CAP 256
#define LINE_ELEMENT_CAP 64
#define DATA_CAP 128

typedef struct ImportEntry {
    char title[LINE_ELEMENT_CAP];
    char artist[LINE_ELEMENT_CAP];
    char album[LINE_ELEMENT_CAP];
} ImportEntry;

typedef struct ImportData {
    ImportEntry* entries;
    size_t       size;
    size_t       cap;

    size_t idx;
} ImportData;

ImportData* get_import_data(const char* file_path);
void        clear_import_data(ImportData** data);

char* build_query_import(ImportEntry* entry);

#endif

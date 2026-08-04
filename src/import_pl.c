#include "import_pl.h"

#include <stdio.h>

#include "library.h"

ImportData* get_import_data(const char* file_path) {
    if (!file_path) {
        errlog(ERR_NULL_OBJECT, "import_pl:get:file_path");
        return NULL;
    }

    char* buf = malloc(LINE_CAP);
    if (!buf) {
        errlog(ERR_MALLOC_NULL, "import_pl:get:buf");
        return NULL;
    }

    const char* home = getenv("HOME");
    snprintf(buf, LINE_CAP, "%s/Music/shellify/%s", home, file_path);

    ImportData* data = malloc(sizeof(ImportData));
    if (!data) {
        errlog(ERR_MALLOC_NULL, "import_pl:get:data");
        free(buf);
        return NULL;
    }

    data->size = 0;
    data->cap = DATA_CAP;
    data->idx = 0;
    data->entries = malloc(DATA_CAP * sizeof(ImportEntry));
    if (!data->entries) {
        errlog(ERR_MALLOC_NULL, "import_pl:get:data");
        free(data);
        free(buf);
        return NULL;
    }

    FILE* file = fopen(buf, "r");
    if (!file) {
        errlog(ERR_FILE_OPENING, "import_pl:get:buf");
        free(data->entries);
        free(data);
        free(buf);
        return NULL;
    }

    int first = 1;
    while (fgets(buf, LINE_CAP, file)) {
        if (first) {
            first = 0;
            continue;
        }

        if (data->size + 1 >= data->cap) {
            size_t       new_cap = data->cap * 2;
            ImportEntry* temp =
                realloc(data->entries, new_cap * sizeof(ImportEntry));
            if (!temp) {
                errlog(ERR_MALLOC_NULL, "import_pl:get:temp");
                if (data->entries) free(data->entries);
                free(data);
                free(buf);
                fclose(file);
                return NULL;
            }

            data->entries = temp;
            data->cap = new_cap;
        }

        ImportEntry* entry = &data->entries[data->size];
        if (strlen(buf) != 0) {
            char* token = strtok(buf, "|");
            strcpy(entry->title, token);
            token = strtok(NULL, "|");
            strcpy(entry->artist, token);
            token = strtok(NULL, "|");
            strcpy(entry->album, token);

            data->size++;
        }
    }

    free(buf);
    fclose(file);

    return data;
}

void clear_import_data(ImportData** data) {
    if (!data || !*data) return;

    if ((*data)->entries) free((*data)->entries);
    free(*data);
    *data = NULL;
}

char* build_query_import(ImportEntry* entry) {
    if (!entry) {
        errlog(ERR_NULL_OBJECT, "import_pl:build:entry");
        return NULL;
    }

    char* buf = malloc(LINE_CAP);
    if (!buf) {
        errlog(ERR_MALLOC_NULL, "import_pl:build:buf");
        return NULL;
    }

    snprintf(buf, LINE_CAP, "%s - %s", entry->title, entry->artist);

    return buf;
}

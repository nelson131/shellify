#ifndef STORAGE_H
#define STORAGE_H

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

#include "db_handler.h"
#include "error_handler.h"

int storage_init(sqlite3** db);
int storage_close(sqlite3** db);

#endif

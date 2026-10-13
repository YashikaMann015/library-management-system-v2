#ifndef DATABASE_H
#define DATABASE_H
#include <sqlite3.h>
int db_init(const char *path);
int db_close(void);
int db_seed(void);
sqlite3 *db_get(void);
#endif

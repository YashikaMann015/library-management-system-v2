#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "config.h"
#include "database.h"
#include "server.h"

int main(void) {
    srand((unsigned)time(NULL));
    printf("========================================\n");
    printf(" Library Management System - C Backend\n");
    printf("========================================\n");
    if (db_init(DB_PATH) != 0) return 1;
    db_seed();
    start_server(SERVER_PORT);
    db_close();
    return 0;
}

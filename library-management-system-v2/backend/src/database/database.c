#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include "database.h"

static sqlite3 *db = NULL;

sqlite3 *db_get(void) { return db; }

int db_init(const char *path) {
    if (sqlite3_open(path, &db) != SQLITE_OK) {
        fprintf(stderr, "SQLite error: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    const char *sql =
        "PRAGMA foreign_keys=ON;"

        "CREATE TABLE IF NOT EXISTS users("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,email TEXT UNIQUE NOT NULL,"
        "password_hash TEXT NOT NULL,"
        "role TEXT NOT NULL CHECK(role IN('STUDENT','LIBRARIAN','ADMIN')),"
        "phone TEXT,status TEXT DEFAULT 'ACTIVE',created_at TEXT DEFAULT CURRENT_TIMESTAMP);"

        "CREATE TABLE IF NOT EXISTS categories("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,name TEXT UNIQUE NOT NULL,description TEXT);"

        "CREATE TABLE IF NOT EXISTS books("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,isbn TEXT UNIQUE NOT NULL,"
        "title TEXT NOT NULL,author TEXT NOT NULL,publisher TEXT,"
        "category_id INTEGER,publication_year INTEGER,quantity INTEGER DEFAULT 0,"
        "available_copies INTEGER DEFAULT 0,shelf TEXT,description TEXT,"
        "created_at TEXT DEFAULT CURRENT_TIMESTAMP,"
        "FOREIGN KEY(category_id) REFERENCES categories(id));"

        "CREATE TABLE IF NOT EXISTS borrow_requests("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,user_id INTEGER NOT NULL,book_id INTEGER NOT NULL,"
        "request_date TEXT DEFAULT CURRENT_TIMESTAMP,status TEXT DEFAULT 'PENDING',processed_by INTEGER);"

        "CREATE TABLE IF NOT EXISTS issued_books("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,user_id INTEGER NOT NULL,book_id INTEGER NOT NULL,"
        "issued_by INTEGER,issue_date TEXT DEFAULT CURRENT_TIMESTAMP,due_date TEXT NOT NULL,"
        "return_date TEXT,status TEXT DEFAULT 'ISSUED');"

        "CREATE TABLE IF NOT EXISTS fines("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,issue_id INTEGER NOT NULL,user_id INTEGER NOT NULL,"
        "amount REAL DEFAULT 0,reason TEXT,status TEXT DEFAULT 'UNPAID',paid_at TEXT,"
        "created_at TEXT DEFAULT CURRENT_TIMESTAMP);"

        "CREATE TABLE IF NOT EXISTS activity_logs("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,user_id INTEGER,action TEXT NOT NULL,"
        "description TEXT,created_at TEXT DEFAULT CURRENT_TIMESTAMP);";

    char *err = NULL;
    if (sqlite3_exec(db, sql, NULL, NULL, &err) != SQLITE_OK) {
        fprintf(stderr, "Schema error: %s\n", err);
        sqlite3_free(err);
        return 1;
    }
    return 0;
}

int db_seed(void) {
    const char *sql =
        "INSERT OR IGNORE INTO categories(id,name,description) VALUES"
        "(1,'Programming','Programming books'),"
        "(2,'Database','Database systems'),"
        "(3,'Algorithms','Algorithms and data structures'),"
        "(4,'Operating Systems','Operating systems');"

        "INSERT OR IGNORE INTO users(id,name,email,password_hash,role) VALUES"
        "(1,'Rahul Sharma','student@library.local','student123','STUDENT'),"
        "(2,'Priya Librarian','librarian@library.local','librarian123','LIBRARIAN'),"
        "(3,'System Admin','admin@library.local','admin123','ADMIN');"

        "INSERT OR IGNORE INTO books(id,isbn,title,author,publisher,category_id,publication_year,quantity,available_copies,shelf) VALUES"
        "(1,'9780073523323','Database System Concepts','Korth','McGraw Hill',2,2022,5,4,'DB-A1'),"
        "(2,'9780133594140','Operating System Concepts','Silberschatz','Wiley',4,2021,4,2,'OS-B2'),"
        "(3,'9780131103627','The C Programming Language','Kernighan & Ritchie','Prentice Hall',1,2019,5,5,'C-C1'),"
        "(4,'9780262046305','Introduction to Algorithms','Cormen','MIT Press',3,2022,3,1,'ALG-D4');";

    char *err = NULL;
    if (sqlite3_exec(db, sql, NULL, NULL, &err) != SQLITE_OK) {
        fprintf(stderr, "Seed error: %s\n", err);
        sqlite3_free(err);
        return 1;
    }
    return 0;
}

int db_close(void) {
    if (db) sqlite3_close(db);
    db = NULL;
    return 0;
}

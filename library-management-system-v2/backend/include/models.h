#ifndef MODELS_H
#define MODELS_H

typedef enum {
    ROLE_STUDENT = 0,
    ROLE_LIBRARIAN = 1,
    ROLE_ADMIN = 2
} UserRole;

typedef struct {
    int id;
    char name[128];
    char email[160];
    char role[32];
} User;

typedef struct {
    int id;
    char isbn[32];
    char title[256];
    char author[160];
    char category[100];
    int year;
    int quantity;
    int available_copies;
    char shelf[64];
} Book;

#endif

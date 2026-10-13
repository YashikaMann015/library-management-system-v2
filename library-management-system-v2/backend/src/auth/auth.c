#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sqlite3.h>
#include "auth.h"
#include "database.h"

#define MAX_SESSIONS 256

typedef struct {
    char token[65];
    AuthUser user;
    int active;
} Session;

static Session sessions[MAX_SESSIONS];
static unsigned long counter = 1000;

static void make_token(char *out, size_t n) {
    snprintf(out, n, "lms_%lu_%u_%u", ++counter,
             (unsigned)(time(NULL) & 0xffffffffu),
             (unsigned)(rand() & 0xffffu));
}

const char *auth_create_token(int user_id) {
    static char token[65];
    for (int i=0;i<MAX_SESSIONS;i++) {
        if (!sessions[i].active) {
            make_token(token, sizeof(token));
            sessions[i].active = 1;
            strncpy(sessions[i].token, token, sizeof(sessions[i].token)-1);
            sessions[i].user.id = user_id;
            sqlite3_stmt *s = NULL;
            sqlite3_prepare_v2(db_get(),
                "SELECT name,email,role FROM users WHERE id=?", -1, &s, NULL);
            sqlite3_bind_int(s,1,user_id);
            if (sqlite3_step(s)==SQLITE_ROW) {
                strncpy(sessions[i].user.name,(const char*)sqlite3_column_text(s,0),127);
                strncpy(sessions[i].user.email,(const char*)sqlite3_column_text(s,1),159);
                strncpy(sessions[i].user.role,(const char*)sqlite3_column_text(s,2),31);
            }
            sqlite3_finalize(s);
            return token;
        }
    }
    return NULL;
}

int auth_get_user(const char *token, AuthUser *user) {
    if (!token || !user) return 0;
    for (int i=0;i<MAX_SESSIONS;i++) {
        if (sessions[i].active && strcmp(sessions[i].token, token)==0) {
            *user = sessions[i].user;
            return 1;
        }
    }
    return 0;
}

int auth_login(const char *email, const char *password, char *json, int size) {
    sqlite3_stmt *s=NULL;
    sqlite3_prepare_v2(db_get(),
        "SELECT id,name,email,role FROM users WHERE email=? AND password_hash=? AND status='ACTIVE'",
        -1,&s,NULL);
    sqlite3_bind_text(s,1,email,-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(s,2,password,-1,SQLITE_TRANSIENT);

    if (sqlite3_step(s)==SQLITE_ROW) {
        int id=sqlite3_column_int(s,0);
        const char *name=(const char*)sqlite3_column_text(s,1);
        const char *mail=(const char*)sqlite3_column_text(s,2);
        const char *role=(const char*)sqlite3_column_text(s,3);
        const char *token=auth_create_token(id);
        snprintf(json,size,
            "{\"success\":true,\"token\":\"%s\",\"user\":{\"id\":%d,\"name\":\"%s\",\"email\":\"%s\",\"role\":\"%s\"}}",
            token?token:"",id,name,mail,role);
        sqlite3_finalize(s);
        return token!=NULL;
    }
    sqlite3_finalize(s);
    snprintf(json,size,"{\"success\":false,\"message\":\"Invalid credentials\"}");
    return 0;
}

int auth_me(const char *token, char *json, int size) {
    AuthUser u;
    if (!auth_get_user(token,&u)) {
        snprintf(json,size,"{\"success\":false,\"message\":\"Unauthorized\"}");
        return 0;
    }
    snprintf(json,size,
        "{\"success\":true,\"user\":{\"id\":%d,\"name\":\"%s\",\"email\":\"%s\",\"role\":\"%s\"}}",
        u.id,u.name,u.email,u.role);
    return 1;
}

int auth_logout(const char *token) {
    for (int i=0;i<MAX_SESSIONS;i++) {
        if (sessions[i].active && strcmp(sessions[i].token,token)==0) {
            sessions[i].active=0;
            return 1;
        }
    }
    return 0;
}

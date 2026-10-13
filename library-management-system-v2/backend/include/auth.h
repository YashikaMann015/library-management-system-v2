#ifndef AUTH_H
#define AUTH_H
typedef struct {
    int id;
    char name[128];
    char email[160];
    char role[32];
} AuthUser;

int auth_login(const char *email, const char *password, char *json, int json_size);
int auth_me(const char *token, char *json, int json_size);
int auth_logout(const char *token);
int auth_get_user(const char *token, AuthUser *user);
const char *auth_create_token(int user_id);
#endif

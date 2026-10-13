#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib")
typedef SOCKET socket_t;
#define CLOSESOCKET closesocket
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
typedef int socket_t;
#define CLOSESOCKET close
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <sqlite3.h>
#include "server.h"
#include "database.h"
#include "auth.h"

#define REQ_MAX 65536
#define RES_MAX 262144

static void send_json(socket_t c,int status,const char *body){
    const char *s=status==200?"OK":status==201?"Created":status==400?"Bad Request":
        status==401?"Unauthorized":status==403?"Forbidden":status==404?"Not Found":
        status==409?"Conflict":"Internal Server Error";
    char h[1024];
    snprintf(h,sizeof(h),
        "HTTP/1.1 %d %s\r\nContent-Type: application/json\r\n"
        "Access-Control-Allow-Origin: http://localhost:5173\r\n"
        "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
        "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
        "Content-Length: %zu\r\nConnection: close\r\n\r\n",status,s,strlen(body));
    send(c,h,(int)strlen(h),0); send(c,body,(int)strlen(body),0);
}

static const char *header_value(const char *req,const char *header){
    static char out[512]; out[0]=0;
    const char *p=strstr(req,header);
    if(!p) return NULL;
    p+=strlen(header);
    while(*p==' ')p++;
    size_t i=0;
    while(*p && *p!='\r' && *p!='\n' && i<sizeof(out)-1) out[i++]=*p++;
    out[i]=0; return out;
}

static int get_user(const char *req,AuthUser *u){
    const char *a=header_value(req,"Authorization:");
    if(!a || strncmp(a,"Bearer ",7)!=0) return 0;
    return auth_get_user(a+7,u);
}

static int body_start(const char *req,const char **body){
    const char *p=strstr(req,"\r\n\r\n");
    if(!p)return 0; *body=p+4; return 1;
}

static int json_string(const char *body,const char *key,char *out,size_t n){
    char pattern[128]; snprintf(pattern,sizeof(pattern),"\"%s\"",key);
    const char *p=strstr(body,pattern); if(!p)return 0;
    p=strchr(p,':'); if(!p)return 0; p++;
    while(isspace((unsigned char)*p))p++;
    if(*p!='\"')return 0;p++;
    size_t i=0;
    while(*p && *p!='\"' && i<n-1){ if(*p=='\\'&&p[1])p++; out[i++]=*p++; }
    out[i]=0; return 1;
}

static int json_int(const char *body,const char *key,int *out){
    char pattern[128];snprintf(pattern,sizeof(pattern),"\"%s\"",key);
    const char*p=strstr(body,pattern);if(!p)return 0;p=strchr(p,':');if(!p)return 0;
    *out=atoi(p+1);return 1;
}

static int allowed(AuthUser *u,const char *role){
    return u && (strcmp(u->role,"ADMIN")==0 || strcmp(u->role,role)==0);
}

static void books(socket_t c){
    sqlite3_stmt*s=NULL;
    sqlite3_prepare_v2(db_get(),
        "SELECT b.id,b.isbn,b.title,b.author,COALESCE(c.name,''),b.publication_year,b.quantity,b.available_copies,COALESCE(b.shelf,'') "
        "FROM books b LEFT JOIN categories c ON c.id=b.category_id ORDER BY b.id DESC",-1,&s,NULL);
    char *buf=malloc(RES_MAX); strcpy(buf,"{\"success\":true,\"books\":[");
    int first=1;
    while(sqlite3_step(s)==SQLITE_ROW){
        char row[2048];
        snprintf(row,sizeof(row),
            "%s{\"id\":%d,\"isbn\":\"%s\",\"title\":\"%s\",\"author\":\"%s\",\"category\":\"%s\",\"year\":%d,\"quantity\":%d,\"available\":%d,\"shelf\":\"%s\"}",
            first?"":",",sqlite3_column_int(s,0),(const char*)sqlite3_column_text(s,1),
            (const char*)sqlite3_column_text(s,2),(const char*)sqlite3_column_text(s,3),
            (const char*)sqlite3_column_text(s,4),sqlite3_column_int(s,5),
            sqlite3_column_int(s,6),sqlite3_column_int(s,7),(const char*)sqlite3_column_text(s,8));
        strncat(buf,row,RES_MAX-strlen(buf)-1);first=0;
    }
    sqlite3_finalize(s);strcat(buf,"]}");send_json(c,200,buf);free(buf);
}

static void create_book(socket_t c,const char *body,AuthUser*u){
    if(!allowed(u,"LIBRARIAN")){send_json(c,403,"{\"success\":false,\"message\":\"Forbidden\"}");return;}
    char isbn[64],title[256],author[160],category[128],shelf[64];int year=0,qty=0;
    if(!json_string(body,"isbn",isbn,sizeof(isbn))||!json_string(body,"title",title,sizeof(title))||
       !json_string(body,"author",author,sizeof(author))||!json_string(body,"category",category,sizeof(category))||
       !json_int(body,"year",&year)||!json_int(body,"quantity",&qty)){
        send_json(c,400,"{\"success\":false,\"message\":\"isbn,title,author,category,year,quantity are required\"}");return;
    }
    json_string(body,"shelf",shelf,sizeof(shelf));
    sqlite3_stmt*s=NULL;
    sqlite3_prepare_v2(db_get(),"SELECT id FROM categories WHERE name=?",-1,&s,NULL);
    sqlite3_bind_text(s,1,category,-1,SQLITE_TRANSIENT);
    int cid=0;if(sqlite3_step(s)==SQLITE_ROW)cid=sqlite3_column_int(s,0);sqlite3_finalize(s);
    if(!cid){
        sqlite3_prepare_v2(db_get(),"INSERT INTO categories(name) VALUES(?)",-1,&s,NULL);
        sqlite3_bind_text(s,1,category,-1,SQLITE_TRANSIENT);sqlite3_step(s);sqlite3_finalize(s);cid=(int)sqlite3_last_insert_rowid(db_get());
    }
    sqlite3_prepare_v2(db_get(),
        "INSERT INTO books(isbn,title,author,category_id,publication_year,quantity,available_copies,shelf) VALUES(?,?,?,?,?,?,?,?)",-1,&s,NULL);
    sqlite3_bind_text(s,1,isbn,-1,SQLITE_TRANSIENT);sqlite3_bind_text(s,2,title,-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(s,3,author,-1,SQLITE_TRANSIENT);sqlite3_bind_int(s,4,cid);sqlite3_bind_int(s,5,year);
    sqlite3_bind_int(s,6,qty);sqlite3_bind_int(s,7,qty);sqlite3_bind_text(s,8,shelf,-1,SQLITE_TRANSIENT);
    int rc=sqlite3_step(s);sqlite3_finalize(s);
    if(rc!=SQLITE_DONE){send_json(c,409,"{\"success\":false,\"message\":\"Could not create book; ISBN may already exist\"}");return;}
    send_json(c,201,"{\"success\":true,\"message\":\"Book created\"}");
}

static void request_book(socket_t c,const char *body,AuthUser*u){
    if(strcmp(u->role,"STUDENT")!=0){send_json(c,403,"{\"success\":false,\"message\":\"Students only\"}");return;}
    int book=0;if(!json_int(body,"book_id",&book)){send_json(c,400,"{\"success\":false,\"message\":\"book_id required\"}");return;}
    sqlite3_stmt*s=NULL;
    sqlite3_prepare_v2(db_get(),"SELECT available_copies FROM books WHERE id=?",-1,&s,NULL);
    sqlite3_bind_int(s,1,book);
    if(sqlite3_step(s)!=SQLITE_ROW){sqlite3_finalize(s);send_json(c,404,"{\"success\":false,\"message\":\"Book not found\"}");return;}
    int avail=sqlite3_column_int(s,0);sqlite3_finalize(s);
    if(avail<=0){send_json(c,409,"{\"success\":false,\"message\":\"Book unavailable\"}");return;}
    sqlite3_prepare_v2(db_get(),"INSERT INTO borrow_requests(user_id,book_id) VALUES(?,?)",-1,&s,NULL);
    sqlite3_bind_int(s,1,u->id);sqlite3_bind_int(s,2,book);sqlite3_step(s);sqlite3_finalize(s);
    send_json(c,201,"{\"success\":true,\"message\":\"Request submitted\"}");
}

static void list_requests(socket_t c,AuthUser*u){
    if(!allowed(u,"LIBRARIAN")){send_json(c,403,"{\"success\":false,\"message\":\"Forbidden\"}");return;}
    sqlite3_stmt*s=NULL;
    sqlite3_prepare_v2(db_get(),
        "SELECT r.id,u.name,b.title,r.status,r.request_date FROM borrow_requests r JOIN users u ON u.id=r.user_id JOIN books b ON b.id=r.book_id ORDER BY r.id DESC",-1,&s,NULL);
    char *buf=malloc(RES_MAX);strcpy(buf,"{\"success\":true,\"requests\":[");int first=1;
    while(sqlite3_step(s)==SQLITE_ROW){
        char row[1500];snprintf(row,sizeof(row),"%s{\"id\":%d,\"student\":\"%s\",\"book\":\"%s\",\"status\":\"%s\",\"date\":\"%s\"}",
            first?"":",",sqlite3_column_int(s,0),(const char*)sqlite3_column_text(s,1),(const char*)sqlite3_column_text(s,2),
            (const char*)sqlite3_column_text(s,3),(const char*)sqlite3_column_text(s,4));
        strncat(buf,row,RES_MAX-strlen(buf)-1);first=0;
    }
    sqlite3_finalize(s);strcat(buf,"]}");send_json(c,200,buf);free(buf);
}

static void approve_request(socket_t c,const char *path,AuthUser*u,int approve){
    if(!allowed(u,"LIBRARIAN")){send_json(c,403,"{\"success\":false,\"message\":\"Forbidden\"}");return;}
    int id=atoi(strrchr(path,'/')+1);sqlite3_stmt*s=NULL;
    sqlite3_prepare_v2(db_get(),"SELECT book_id FROM borrow_requests WHERE id=? AND status='PENDING'",-1,&s,NULL);
    sqlite3_bind_int(s,1,id);
    if(sqlite3_step(s)!=SQLITE_ROW){sqlite3_finalize(s);send_json(c,404,"{\"success\":false,\"message\":\"Pending request not found\"}");return;}
    int book=sqlite3_column_int(s,0);sqlite3_finalize(s);
    sqlite3_exec(db_get(),"BEGIN TRANSACTION",0,0,0);
    char sql[512];
    snprintf(sql,sizeof(sql),"UPDATE borrow_requests SET status='%s',processed_by=%d WHERE id=%d",
        approve?"APPROVED":"REJECTED",u->id,id);
    sqlite3_exec(db_get(),sql,0,0,0);
    if(approve) {
        snprintf(sql,sizeof(sql),"UPDATE books SET available_copies=available_copies-1 WHERE id=%d AND available_copies>0",book);
        sqlite3_exec(db_get(),sql,0,0,0);
    }
    sqlite3_exec(db_get(),"COMMIT",0,0,0);
    send_json(c,200,approve?"{\"success\":true,\"message\":\"Request approved\"}":"{\"success\":true,\"message\":\"Request rejected\"}");
}

static void dashboard(socket_t c,AuthUser*u){
    if(!u){send_json(c,401,"{\"success\":false,\"message\":\"Unauthorized\"}");return;}
    sqlite3_stmt*s=NULL;int books_count=0,available=0,issued=0,users=0,pending=0;
    sqlite3_prepare_v2(db_get(),"SELECT COUNT(*),COALESCE(SUM(available_copies),0) FROM books",-1,&s,NULL);
    if(sqlite3_step(s)==SQLITE_ROW){books_count=sqlite3_column_int(s,0);available=sqlite3_column_int(s,1);}sqlite3_finalize(s);
    sqlite3_prepare_v2(db_get(),"SELECT COUNT(*) FROM issued_books WHERE status='ISSUED'",-1,&s,NULL);
    if(sqlite3_step(s)==SQLITE_ROW)issued=sqlite3_column_int(s,0);sqlite3_finalize(s);
    sqlite3_prepare_v2(db_get(),"SELECT COUNT(*) FROM users",-1,&s,NULL);
    if(sqlite3_step(s)==SQLITE_ROW)users=sqlite3_column_int(s,0);sqlite3_finalize(s);
    sqlite3_prepare_v2(db_get(),"SELECT COUNT(*) FROM borrow_requests WHERE status='PENDING'",-1,&s,NULL);
    if(sqlite3_step(s)==SQLITE_ROW)pending=sqlite3_column_int(s,0);sqlite3_finalize(s);
    char out[2000];
    snprintf(out,sizeof(out),"{\"success\":true,\"stats\":{\"books\":%d,\"available\":%d,\"issued\":%d,\"users\":%d,\"pending\":%d}}",books_count,available,issued,users,pending);
    send_json(c,200,out);
}

static void route(socket_t c,const char *req){
    char method[16]={0},path[512]={0};sscanf(req,"%15s %511s",method,path);
    if(strcmp(method,"OPTIONS")==0){send_json(c,200,"{}");return;}
    if(strcmp(path,"/health")==0){send_json(c,200,"{\"success\":true,\"status\":\"running\"}");return;}

    const char *body=NULL;body_start(req,&body);
    AuthUser user;int logged=get_user(req,&user);

    if(strcmp(path,"/api/auth/login")==0 && strcmp(method,"POST")==0){
        char email[160],password[160],out[2000];
        if(!body||!json_string(body,"email",email,sizeof(email))||!json_string(body,"password",password,sizeof(password))){
            send_json(c,400,"{\"success\":false,\"message\":\"Email and password required\"}");return;
        }
        int ok=auth_login(email,password,out,sizeof(out));send_json(c,ok?200:401,out);return;
    }
    if(strcmp(path,"/api/auth/me")==0 && strcmp(method,"GET")==0){
        char out[1500];int ok=auth_me(header_value(req,"Authorization:")+7,out,sizeof(out));
        send_json(c,ok?200:401,out);return;
    }
    if(strcmp(path,"/api/auth/logout")==0 && strcmp(method,"POST")==0){
        const char*a=header_value(req,"Authorization:");
        if(a&&strncmp(a,"Bearer ",7)==0)auth_logout(a+7);
        send_json(c,200,"{\"success\":true}");return;
    }

    if(!logged){send_json(c,401,"{\"success\":false,\"message\":\"Authentication required\"}");return;}

    if(strcmp(path,"/api/books")==0 && strcmp(method,"GET")==0){books(c);return;}
    if(strcmp(path,"/api/books")==0 && strcmp(method,"POST")==0){create_book(c,body,&user);return;}
    if(strcmp(path,"/api/requests")==0 && strcmp(method,"POST")==0){request_book(c,body,&user);return;}
    if(strcmp(path,"/api/requests")==0 && strcmp(method,"GET")==0){list_requests(c,&user);return;}
    if(strncmp(path,"/api/requests/",14)==0 && strcmp(method,"PUT")==0){
        if(strstr(path,"/approve"))approve_request(c,path,&user,1);
        else if(strstr(path,"/reject"))approve_request(c,path,&user,0);
        else send_json(c,404,"{\"success\":false,\"message\":\"Unknown request action\"}");
        return;
    }
    if(strncmp(path,"/api/dashboard/",15)==0 && strcmp(method,"GET")==0){dashboard(c,&user);return;}

    send_json(c,404,"{\"success\":false,\"message\":\"Route not implemented\"}");
}

int start_server(int port){
#ifdef _WIN32
    WSADATA w; if(WSAStartup(MAKEWORD(2,2),&w)!=0)return 1;
#endif
    socket_t server=socket(AF_INET,SOCK_STREAM,0);if(server==(socket_t)-1)return 1;
    int opt=1;
#ifdef _WIN32
    setsockopt(server,SOL_SOCKET,SO_REUSEADDR,(const char*)&opt,sizeof(opt));
#else
    setsockopt(server,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
#endif
    struct sockaddr_in a={0};a.sin_family=AF_INET;a.sin_addr.s_addr=htonl(INADDR_ANY);a.sin_port=htons((unsigned short)port);
    if(bind(server,(struct sockaddr*)&a,sizeof(a))<0)return 1;if(listen(server,16)<0)return 1;
    printf("C API listening on http://localhost:%d\n",port);
    while(1){
        socket_t c=accept(server,NULL,NULL);if(c==(socket_t)-1)continue;
        char *buf=malloc(REQ_MAX);int n=recv(c,buf,REQ_MAX-1,0);
        if(n>0){buf[n]=0;route(c,buf);}free(buf);CLOSESOCKET(c);
    }
}

#include <getopt.h>
#include <limits.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include "libpq-fe.h"

/* sizes and limits */
#define BUFFERSIZE 4096
#define MAX_CONSOLE 8

/* значения по-умолчанию */
#define DEFAULT_HOSTADDR "127.0.0.1"
#define DEFAULT_PORT "5432"
#define DEFAULT_USER "postgres"
#define DEFAULT_DBNAME "postgres"

#define PGBRC_FILE ".pgbrc"
#define PGBRC_READ_OK 0
#define PGBRC_READ_ERR 1

#define SHOW_POOLS_COLUMNS_NUM 10
#define SHOW_CLIENTS_COLUMNS_NUM 12
#define SHOW_SERVERS_COLUMNS_NUM 12
#define SHOW_DATABASES_COLUMNS_NUM 7
#define SHOW_POOLS_COLUMNS_NUM 10
#define SHOW_POOLS_COLUMNS_NUM 10

static char pgbrcpath[PATH_MAX];

/* массив содержащий короткие параметры */
const char * short_options = "h:p:U:d:wW?";

/* структура содержащая длинные параметры и их короткие аналоги */
const struct option long_options[] = {
    {"help", no_argument, NULL, '?'},
    {"host", required_argument, NULL, 'h'},
    {"port", required_argument, NULL, 'p'},
    {"dbname", required_argument, NULL, 'd'},
    {"no-password", no_argument, NULL, 'w'},
    {"password", no_argument, NULL, 'W'},
    {"user", required_argument, NULL, 'U'},
    {NULL, 0, NULL, 0}
};

/*
 *  Структура которая описывает соединение, на основе структуры 
 *  будет создан массив структур для хранения параметров подключения.
 */
struct conn_opts
{
    int terminal;
    bool conn_used;
    char hostaddr[BUFFERSIZE];
    char port[BUFFERSIZE];
    char user[BUFFERSIZE];
    char dbname[BUFFERSIZE];
    char password[BUFFERSIZE];
    char conninfo[BUFFERSIZE];
};

/* enum for password purpose */
enum trivalue
{
    TRI_DEFAULT,
    TRI_NO,
    TRI_YES
};

/* массив структур который содержит параметры коннектов для всех консолей */
struct conn_opts connections[MAX_CONSOLE] = {
    { 0, false, "", "", "", "", "", "" },
    { 1, false, "", "", "", "", "", "" },
    { 2, false, "", "", "", "", "", "" },
    { 3, false, "", "", "", "", "", "" },
    { 4, false, "", "", "", "", "", "" },
    { 5, false, "", "", "", "", "", "" },
    { 6, false, "", "", "", "", "", "" },
    { 7, false, "", "", "", "", "", "" }};

/* Structures declaration. */
static struct passwd *pw;                  // get current user info: pw_name,pw_uid,pw_gid,pw_dir,pw_shell.

/* Functions prototypes */
void create_initial_conn(int argc, char *argv[], struct conn_opts connections[]);     // обработка входных параметров и установка дефолтов
int create_pgbrc_conn(int argc, char *argv[], struct conn_opts connections[], const int pos);
void print_conn(struct conn_opts connections[]);
void prepare_conninfo(struct conn_opts connections[]);          /* prepare conninfo string from conn_opts */
char * simple_prompt(const char *prompt, int maxlen, bool echo);
PGconn * do_connection(const char conninfo[]);
void close_connection(PGconn *conn);
PGresult * do_query(PGconn * conn, char query[]);

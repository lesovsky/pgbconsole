#include <getopt.h>
#include <limits.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* значения по-умолчанию */
#define DEFAULT_HOSTADDR "127.0.0.1"
#define DEFAULT_PORT "5432"
#define DEFAULT_USER "postgres"
#define DEFAULT_DBNAME "postgres"

#define PGBRC_FILE ".pgbrc"
#define PGBRC_READ_OK 0
#define PGBRC_READ_ERR 1
#define MAX_CONSOLE 8

static char pgbrcpath[PATH_MAX];

/* массив содержащий короткие параметры */
const char * short_options = "h:p:U:d:";

/* структура содержащая длинные параметры и их короткие аналоги */
const struct option long_options[] = {
    {"help", optional_argument, NULL, '?'},
    {"host", required_argument, NULL, 'h'},
    {"port", required_argument, NULL, 'p'},
    {"dbname", required_argument, NULL, 'd'},
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
    char hostaddr[4096];
    char port[4096];
    char user[4096];
    char dbname[4096];
    char password[4096];
};

/* массив структур который содержит параметры коннектов для всех консолей */
struct conn_opts connections[MAX_CONSOLE] = {
    { 0, false, "", "", "", "", "" },
    { 1, false, "", "", "", "", "" },
    { 2, false, "", "", "", "", "" },
    { 3, false, "", "", "", "", "" },
    { 4, false, "", "", "", "", "" },
    { 5, false, "", "", "", "", "" },
    { 6, false, "", "", "", "", "" },
    { 7, false, "", "", "", "", "" }};

/* Structures declaration. */
static struct passwd *pw;                  // get current user info: pw_name,pw_uid,pw_gid,pw_dir,pw_shell.

/* Functions prototypes */
void create_initial_conn(int argc, char *argv[], struct conn_opts connections[]);     // обработка входных параметров и установка дефолтов
int create_pgbrc_conn(int argc, char *argv[], struct conn_opts connections[], const int pos);
void print_conn(struct conn_opts connections[]);

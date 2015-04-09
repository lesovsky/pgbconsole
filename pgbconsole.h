#include <getopt.h>
#include <limits.h>
#include <pwd.h>
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
#define PGBRC_EXIST 0
#define PGBRC_NOT_EXIST 1
#define PGBRC_WRONG 2
#define MAX_CONSOLE 8

static char pgbrcpath[PATH_MAX];

/* массив содержащий короткие параметры */
const char * short_options = "h:p:U:d:";

/* структура содержащая длинные параметры и их короткие аналоги */
const struct option long_options[] = {
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
    char *hostaddr;
    char *port;
    char *user;
    char *dbname;
    char *password;
};

/* массив структур который содержит параметры коннектов для всех консолей */
struct conn_opts connections[MAX_CONSOLE];

/* Structures declaration. */
static struct passwd *pw;                  // get current user info: pw_name,pw_uid,pw_gid,pw_dir,pw_shell.

/* Functions prototypes */
struct conn_opts create_initial_conn(int argc, char *argv[], struct conn_opts connections[]);     // обработка входных параметров и установка дефолтов
int check_pgbrc(void);

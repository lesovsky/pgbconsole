#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

/* значения по-умолчанию */
#define DEFAULT_HOSTADDR "127.0.0.1"
#define DEFAULT_PORT "5432"
#define DEFAULT_USER "postgres"
#define DEFAULT_DBNAME "postgres"

#define PGBRCFILE ".pgbrc"
#define CONSOLES_LIMIT 8

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
 *  будет создана переменная-структура для хранения параметров подключения.
 */
struct conn_opts
{
    int *terminal;                  // number of terminal or tab
    char *hostaddr;                 // ip address
    char *port;
    char *user;
    char *dbname;
    char *password;
    struct conn_opts *prev;         // pointer to previous connection
    struct conn_opts *next;         // pointer to next connection
};

/* структуры которые будут организованы в список и будут содержать параметр коннектов */
struct conn_opts co1, co2, co3, co4, co5, co6, co7, co8;
/* и 2 указателя для перемещения по списку */
struct conn_opts *conns_list_head, *conns_list_ptr;

/* Structures declaration. */
struct passwd *pw;                  // get current user info: pw_name,pw_uid,pw_gid,pw_dir,pw_shell.

/* Functions prototypes */
void conns_create_list(void);       // create linked list for storing connection options
struct conn_opts arg_parse(int argc, char *argv[], struct conn_opts *conns_list_ptr);     // обработка входных параметров и установка дефолтов

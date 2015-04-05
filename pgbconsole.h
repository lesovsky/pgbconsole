#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

// значения по-умолчанию
#define DEFAULT_HOSTADDR "127.0.0.1"
#define DEFAULT_PORT "5432"
#define DEFAULT_USER "postgres"
#define DEFAULT_DBNAME "postgres"

// массив содержащий короткие параметры
const char * short_options = "h:p:U:d:";

// структура содержащая длинные параметры и их короткие аналоги
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
    char *hostaddr;
    char *port;
    char *user;
    char *dbname;
    struct conn_opts *prev;
    struct conn_opts *next;
};

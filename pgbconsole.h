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
#include <ncurses.h>

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

/* enum for query context */
enum context { pools, clients, servers, databases, stats };

/* Functions prototypes */
void create_initial_conn(int argc, char *argv[], struct conn_opts connections[]);     // обработка входных параметров и установка дефолтов
int create_pgbrc_conn(int argc, char *argv[], struct conn_opts connections[], const int pos);
void print_conn(struct conn_opts connections[]);
void prepare_conninfo(struct conn_opts connections[]);          /* prepare conninfo string from conn_opts */
char * simple_prompt(const char *prompt, int maxlen, bool echo);
PGresult * do_query(PGconn *conn, enum context query_context);
void open_connections(struct conn_opts connections[], PGconn * conns[]);
void close_connections(PGconn * conns[]);
int key_is_pressed(void);                                       /* check were key is pressed */
void print_data(PGresult *res, enum context query_context);   /* print query result */

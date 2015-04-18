#include <getopt.h>
#include <limits.h>
#include <ncurses.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include "libpq-fe.h"

/* sizes and limits */
#define BUFFERSIZE 4096
#define MAX_CONSOLE 8

/* connectins defaults */
#define DEFAULT_HOSTADDR "127.0.0.1"
#define DEFAULT_PORT "6432"
#define DEFAULT_USER "postgres"
#define DEFAULT_DBNAME "pgbuncer"

#define LOADAVG_FILE "/proc/loadavg"
#define PGBRC_FILE ".pgbrc"
#define PGBRC_READ_OK 0
#define PGBRC_READ_ERR 1

/*
 *  Struct which define connection options
 */
struct conn_opts_struct
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
void create_initial_conn(int argc, char *argv[], struct conn_opts_struct conn_opts[]);     // обработка входных параметров и установка дефолтов
int create_pgbrc_conn(int argc, char *argv[], struct conn_opts_struct conn_opts[], const int pos);
void print_conn(struct conn_opts_struct conn_opts[]);
void prepare_conninfo(struct conn_opts_struct conn_opts[]);          /* prepare conninfo string from conn_opts */
char * simple_prompt(const char *prompt, int maxlen, bool echo);
PGresult * do_query(PGconn *conn, enum context query_context);
void open_connections(struct conn_opts_struct conn_opts[], PGconn * conns[]);
void close_connections(PGconn * conns[]);
int key_is_pressed(void);                                       /* check were key is pressed */

void print_data(WINDOW * window, enum context query_context, PGresult *res);   /* print query result */
char * print_time();                                          /* print current time */
float get_loadavg();                                           /* get load average values */

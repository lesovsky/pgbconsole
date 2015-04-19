#include <errno.h>      /* errno functions */
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

#define STAT_FILE "/proc/stat"
#define UPTIME_FILE "/proc/uptime"
#define LOADAVG_FILE "/proc/loadavg"
#define PGBRC_FILE ".pgbrc"
#define PGBRC_READ_OK 0
#define PGBRC_READ_ERR 1

#define HZ              hz
unsigned int hz;
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

#define CONN_OPTS_SIZE (sizeof(struct conn_opts_struct))

struct stats_cpu_struct {
    unsigned long long cpu_user;
    unsigned long long cpu_nice;
    unsigned long long cpu_sys;
    unsigned long long cpu_idle;
    unsigned long long cpu_iowait;
    unsigned long long cpu_steal;
    unsigned long long cpu_hardirq;
    unsigned long long cpu_softirq;
    unsigned long long cpu_guest;
    unsigned long long cpu_guest_nice;
};

#define STATS_CPU_SIZE (sizeof(struct stats_cpu_struct))

/* enum for password purpose */
enum trivalue
{
    TRI_DEFAULT,
    TRI_NO,
    TRI_YES
};

#define SP_VALUE(m,n,p) (((double) ((n) - (m))) / (p) * 100)

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

void init_stats(struct stats_cpu_struct *st_cpu[]);
void read_stat_cpu(struct stats_cpu_struct *st_cpu, int nbr, unsigned long long *uptime, unsigned long long *uptime0);
unsigned long long get_interval(unsigned long long prev_uptime, unsigned long long curr_uptime);
double ll_sp_value(unsigned long long value1, unsigned long long value2, unsigned long long itv);
void write_cpu_stat(WINDOW * window, struct stats_cpu_struct *st_cpu[], int curr, unsigned long long itv);

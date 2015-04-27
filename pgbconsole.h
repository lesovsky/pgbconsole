/*
 * pgbconsole: top-like console for Pgbouncer - PostgerSQL connection pooler
 * (C) 2015 by Alexey Lesovsky (lesovsky <at> gmail.com)
 */
 
/* sizes and limits */
#include "libpq-fe.h"
#define BUFFERSIZE 4096
#define MAX_CONSOLE 8

/* connectins defaults */
#define DEFAULT_HOSTADDR    "127.0.0.1"
#define DEFAULT_PORT        "6432"
#define DEFAULT_USER        "postgres"
#define DEFAULT_DBNAME      "pgbouncer"

/* files */
#define STAT_FILE       "/proc/stat"
#define UPTIME_FILE     "/proc/uptime"
#define LOADAVG_FILE    "/proc/loadavg"
#define PGBRC_FILE      ".pgbrc"
#define PGBRC_READ_OK   0
#define PGBRC_READ_ERR  1

#define HZ              hz
unsigned int hz;

/* Struct which define connection options */
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

/* struct which used for cpu statistic */
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

#define CONN_OPTS_SIZE (sizeof(struct conn_opts_struct))
#define STATS_CPU_SIZE (sizeof(struct stats_cpu_struct))

/* enum for password purpose */
enum trivalue
{
    TRI_DEFAULT,
    TRI_NO,
    TRI_YES
};
/*
 * Macros used to display statistics values.
 *
 * NB: Define SP_VALUE() to normalize to %;
 */
#define SP_VALUE(m,n,p) (((double) ((n) - (m))) / (p) * 100)

/* enum for query context */
enum context
{
    pools,
    clients,
    servers,
    databases,
    stats
};

/*
 *************************************************************************** 
 * Functions prototypes 
 ***************************************************************************
 */

void
    create_initial_conn(int argc, char *argv[],
                    struct conn_opts_struct *conn_opts[]);
int
    create_pgbrc_conn(int argc, char *argv[],
                    struct conn_opts_struct *conn_opts[], const int pos);
void
    print_conn(struct conn_opts_struct *conn_opts[]);
void
    prepare_conninfo(struct conn_opts_struct *conn_opts[]);
char * 
    password_prompt(const char *prompt, int maxlen, bool echo);
PGresult * 
    do_query(PGconn *conn, enum context query_context);
void
    open_connections(struct conn_opts_struct *conn_opts[], PGconn * conns[]);
void
    close_connections(PGconn * conns[]);
int
    key_is_pressed(void);

void
    print_data(WINDOW * window, enum context query_context, PGresult *res);
char * 
    print_time(void);
void
    print_title(WINDOW * window, char * progname);

float
    get_loadavg();
void
    print_loadavg(WINDOW * window);

void
    print_conninfo(WINDOW * window, struct conn_opts_struct *conn_opts);

void
    init_conn_opts(struct conn_opts_struct *conn_opts[]);
void
    init_stats(struct stats_cpu_struct *st_cpu[]);
void
    read_cpu_stat(struct stats_cpu_struct *st_cpu, int nbr, 
                unsigned long long *uptime, unsigned long long *uptime0);
void
    write_cpu_stat_raw(WINDOW * window, struct stats_cpu_struct *st_cpu[],
                int curr, unsigned long long itv);
void
    print_cpu_usage(WINDOW * window, struct stats_cpu_struct *st_cpu[]);
unsigned long long
    get_interval(unsigned long long prev_uptime,
                unsigned long long curr_uptime);
double
    ll_sp_value(unsigned long long value1, unsigned long long value2,
                unsigned long long itv);
void print_pgbouncer_summary(WINDOW * window, PGconn *conn);

void
    key_processing(int ch);
int switch_conn(WINDOW * window, struct conn_opts_struct * conn_opts[],
    int ch, int console_index, int console_no);

/* Pgbouncer action function */
void do_reload(WINDOW * window, PGconn *conn);
void do_suspend(WINDOW * window, PGconn *conn);
void do_pause(WINDOW * window, PGconn *conn);
void do_resume(WINDOW * window, PGconn *conn);
/* END Pgbouncer action functions */

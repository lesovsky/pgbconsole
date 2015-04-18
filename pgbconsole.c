#include "pgbconsole.h"

int key_is_pressed(void)             /* check were key is pressed */
{
    int ch = getch();
    if (ch != ERR) {
        ungetch(ch);
        return 1;
    }
    else
        return 0;
}
    
void create_initial_conn(int argc, char *argv[], struct conn_opts_struct conn_opts[])
{
    struct passwd *pw = getpwuid(getuid());             /* get current user info */

    /* short options */
    const char * short_options = "h:p:U:d:wW?";
    
    /* long options */
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

    int param, option_index;
    enum trivalue prompt_password = TRI_DEFAULT;

    if (argc > 1)
    {
        if ((strcmp(argv[1], "-?") == 0) || (argc == 2 && (strcmp(argv[1], "--help") == 0)))
        {
            printf("print help here\n");
            exit(EXIT_SUCCESS);
        }
        if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-V") == 0)
        {
            printf("show version here\n");
            exit(EXIT_SUCCESS);
        }
    }

    while ( (param = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1 )
    {
        switch(param)
        {
            case 'h': 
                strcpy(conn_opts[0].hostaddr, optarg);
                break;
            case 'p':
                strcpy(conn_opts[0].port, optarg);
                break;
            case 'U':
                strcpy(conn_opts[0].user, optarg);
                break;
            case 'd':
                strcpy(conn_opts[0].dbname, optarg);
                break;
            case 'w':
                prompt_password = TRI_NO;
                break;
            case 'W':
                prompt_password = TRI_YES;
                break;
            case '?': default:
                fprintf(stderr, "Try \"%s --help\" for more information.\n", argv[0]);
                exit (EXIT_SUCCESS);
                break;
        }
    }

    while (argc - optind >= 1)
    {
        if ( (argc - optind > 1) && strlen(conn_opts[0].user) == 0 && strlen(conn_opts[0].dbname) == 0 )
            strcpy(conn_opts[0].user, argv[optind]);
        else if ( (argc - optind >= 1) && strlen(conn_opts[0].dbname) == 0 )
            strcpy(conn_opts[0].dbname, argv[optind]);
        else
            fprintf(stderr, "%s: warning: extra command-line argument \"%s\" ignored\n", argv[0], argv[optind]);

        optind++;
    }    

    if ( strlen(conn_opts[0].hostaddr) == 0 )
        strcpy(conn_opts[0].hostaddr, DEFAULT_HOSTADDR);

    if ( strlen(conn_opts[0].port) == 0 )
        strcpy(conn_opts[0].port, DEFAULT_PORT);

    if ( strlen(conn_opts[0].user) == 0 ) {
        strcpy(conn_opts[0].user, pw->pw_name);
    }

    if ( prompt_password == TRI_YES )
        strcpy(conn_opts[0].password, simple_prompt("Password: ", 100, false));

    if ( strlen(conn_opts[0].dbname) == 0 && strlen(conn_opts[0].user) == 0 ) {
        strcpy(conn_opts[0].user, pw->pw_name);
        strcpy(conn_opts[0].dbname, DEFAULT_DBNAME);
    }
    else if ( strlen(conn_opts[0].user) != 0 && strlen(conn_opts[0].dbname) == 0 )
        strcpy(conn_opts[0].dbname, conn_opts[0].user);

    conn_opts[0].conn_used = true;
}

int create_pgbrc_conn(int argc, char *argv[], struct conn_opts_struct conn_opts[], const int pos)
{
    FILE *fp;                                               /* file pointer for .pgbrc */
    static char pgbrcpath[PATH_MAX];                        /* path where located .pgbrc */
    struct stat statbuf;                                    /* .pgbrc metadata */
    char strbuf[BUFFERSIZE];
    int i = pos;
    struct passwd *pw = getpwuid(getuid());                 /* current user info */

    /* build path to .pgbrc file */
    strcpy(pgbrcpath, pw->pw_dir);
    strcat(pgbrcpath, "/");
    strcat(pgbrcpath, PGBRC_FILE);

    /* check permissions, must be 600 */
    stat(pgbrcpath, &statbuf);
    if ( statbuf.st_mode & (S_IRWXG | S_IRWXO) ) {
        printf("WARNING: %s has wrong permissions.\n", pgbrcpath);
        return PGBRC_READ_ERR;
    }

    /* read connections settings from .pgbrc */
    if ((fp = fopen(pgbrcpath, "r")) != NULL) {
        while (fgets(strbuf, 4096, fp) != 0) {
            sscanf(strbuf, "%[^:]:%[^:]:%[^:]:%[^:]:%[^:\n]", \
                conn_opts[i].hostaddr, conn_opts[i].port, conn_opts[i].dbname, conn_opts[i].user, conn_opts[i].password);
            conn_opts[i].terminal = i;
            conn_opts[i].conn_used = true;
            i++;
        }
        fclose(fp);
        return PGBRC_READ_OK;
    } else {
        printf("WARNING: failed to open %s. Try use defaults.\n", pgbrcpath);
        return PGBRC_READ_ERR;
    }
}

void print_conn(struct conn_opts_struct conn_opts[])
{
    int i;
    for ( i = 0; i < MAX_CONSOLE; i++ )
        if (conn_opts[i].conn_used)
            printf("qq: %s\n", conn_opts[i].conninfo);
}

void prepare_conninfo(struct conn_opts_struct conn_opts[])
{
    int i;
    for ( i = 0; i < MAX_CONSOLE; i++ )
        if (conn_opts[i].conn_used) {
            strcat(conn_opts[i].conninfo, "hostaddr=");
            strcat(conn_opts[i].conninfo, conn_opts[i].hostaddr);
            strcat(conn_opts[i].conninfo, " port=");
            strcat(conn_opts[i].conninfo, conn_opts[i].port);
            strcat(conn_opts[i].conninfo, " user=");
            strcat(conn_opts[i].conninfo, conn_opts[i].user);
            strcat(conn_opts[i].conninfo, " dbname=");
            strcat(conn_opts[i].conninfo, conn_opts[i].dbname);
            strcat(conn_opts[i].conninfo, " password=");
            strcat(conn_opts[i].conninfo, conn_opts[i].password);
        }
}

void open_connections(struct conn_opts_struct conn_opts[], PGconn * conns[])
{
    int i;
    for ( i = 0; i < MAX_CONSOLE; i++ ) {
        if (conn_opts[i].conn_used) {
            conns[i] = PQconnectdb(conn_opts[i].conninfo);
            if ( PQstatus(conns[i]) == CONNECTION_BAD )
                puts("We were unable to connect to the database");
        }
    }
}

void close_connections(PGconn * conns[])
{
    int i;
    PQclear;
    for (i = 0; i < MAX_CONSOLE; i++)
        PQfinish(conns[i]);
}

PGresult * do_query(PGconn *conn, enum context query_context)
{
    PGresult    *res;
    char query[20];
    switch (query_context) {
        case pools: default:
            strcpy(query, "show pools");
            break;
        case clients:
            strcpy(query, "show clients");
            break;
        case servers:
            strcpy(query, "show servers");
            break;
        case databases:
            strcpy(query, "show databases");
            break;
        case stats:
            strcpy(query, "show stats");
            break;
    }
    res = PQexec(conn, query);
    if ( PQresultStatus(res) != PGRES_TUPLES_OK ) {
        puts("We didn't get any data.");
        return NULL;
    }
    else
        return res;
}

void print_data(PGresult *res, enum context query_context, WINDOW * window)
{
    int    row_count, col_count, row, col, i;
    row_count = PQntuples(res);
    col_count = PQnfields(res);

    struct colAttrs {
        char name[20];
        int width;
    };
    struct colAttrs *columns = (struct colAttrs *) malloc(sizeof(struct colAttrs) * col_count);

    for ( col = 0, i = 0; col < col_count; col++, i++) {
        strcpy(columns[i].name, PQfname(res, col));
        int colname_len = strlen(PQfname(res, col));
        int width = colname_len;
        for (row = 0; row < row_count; row++ ) {
            int val_len = strlen(PQgetvalue(res, row, col));
            if ( val_len >= width )
                width = val_len;
        }
        columns[i].width = width + 3;
    }

    wclear(window);                                 /* clear window */

    wattron(window, A_BOLD);                                 /* print header with bold */
    for ( col = 0, i = 0; col < col_count; col++, i++ )
        wprintw(window, "%-*s", columns[i].width, PQfname(res,col));
    wprintw(window, "\n");
    wattroff(window, A_BOLD);                                /* disable bold */

    for ( row = 0; row < row_count; row++ ) {
        for ( col = 0, i = 0; col < col_count; col++, i++ ) {
            wprintw(window, "%-*s", columns[i].width, PQgetvalue(res, row, col));
        }
    wprintw(window, "\n");
    }
    wprintw(window, "\n");
    wrefresh(window);

    free(columns);
}

char * simple_prompt(const char *prompt, int maxlen, bool echo)
{
    struct termios t_orig, t;
    char *destination;
    destination = (char *) malloc(maxlen + 1);

    if (!echo) {
        tcgetattr(fileno(stdin), &t);
        t_orig = t;
        t.c_lflag &= ~ECHO;
        tcsetattr(fileno(stdin), TCSAFLUSH, &t);
    }

    fputs(prompt, stdout);
    if (fgets(destination, maxlen + 1, stdin) == NULL)
        destination[0] = '\0';

    if (!echo) {
        tcsetattr(fileno(stdin), TCSAFLUSH, &t_orig);
        fputs("\n", stdout);
        fflush(stdout);
    }

    return destination;
}

/*
 *  Summary window functions
 */

void print_summary(WINDOW * window, char * progname)
{
    char *time = print_time();

    wclear(window);
    wprintw(window, "%s - %s, ", progname, time);
    wprintw(window, "load average: %.2f, %.2f, %.2f", get_loadavg(1), get_loadavg(5), get_loadavg(15));
    wrefresh(window);

    free(time);
}

char * print_time()
{
    time_t rawtime;
    struct tm *timeinfo;
    char *strtime = (char *) malloc(sizeof(char) * 20);
    
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(strtime, 20, "%Y-%m-%d %H:%M:%S", timeinfo);
    return strtime;
}

float get_loadavg(const int m)
{
    if ( m != 1 && m != 5 && m != 15 )
        fprintf(stderr, "invalid value for load average\n");

    float avg1, avg5, avg15;
    FILE *loadavg_fd;
    char *loadavg = (char *) malloc(sizeof(char) * 20);

    if ((loadavg_fd = fopen(LOADAVG_FILE, "r")) == NULL) {
        fprintf(stderr, "can't open %s\n", LOADAVG_FILE);
        exit(EXIT_FAILURE);
    } else {
        fscanf(loadavg_fd, "%f %f %f", &avg1, &avg5, &avg15);
        fclose(loadavg_fd);
    }

    switch (m) {
        case 1: return avg1; break;
        case 5: return avg5; break;
        case 15: return avg15; break;
    }
}

/*
 *
 *  Main 
 *
 */

int main (int argc, char *argv[])
{
    char * progname = argv[0];
    PGresult    *res;                                   /* query result */
    struct conn_opts_struct conn_opts[MAX_CONSOLE] = { 
        { 0, false, "", "", "", "", "", "" }, 
        { 1, false, "", "", "", "", "", "" },
        { 2, false, "", "", "", "", "", "" },
        { 3, false, "", "", "", "", "", "" },
        { 4, false, "", "", "", "", "", "" },
        { 5, false, "", "", "", "", "", "" },
        { 6, false, "", "", "", "", "", "" },
        { 7, false, "", "", "", "", "", "" }};          /* connections options array - one element per options set */
    PGconn * conns[8];                                  /* array for connections */
    enum context query_context;                         /* context - query for pgbouncer */
    int console_no = 1, console_index = 0;              /* console number and indexes associated with indexes inside *conns[] */
    int ch;                                             /* var for key code */
    WINDOW  *w_summary, *w_cmdline, *w_answer;          /* main screen windows */


    /* parse input parameters if they are exists */
    if ( argc > 1 ) {                                           /* input parameters specified */
        create_initial_conn(argc, argv, conn_opts);           /* save input parameters */
        create_pgbrc_conn(argc, argv, conn_opts, 1);
    } else                                                     /* input parameters not specified */
        if (create_pgbrc_conn(argc, argv, conn_opts, 0) == PGBRC_READ_ERR)
            create_initial_conn(argc, argv, conn_opts);

    prepare_conninfo(conn_opts);
    print_conn(conn_opts);
    open_connections(conn_opts, conns);

    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);

    w_summary = newwin(5, 0, 0, 0);
    w_cmdline = newwin(1, 0, 5, 0);
    w_answer = newwin(0, 0, 6, 0);

    while (1) {
        if (key_is_pressed()) {
            ch = getch();
            switch (ch) {
                case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8':
                    if ( conn_opts[ch - '0' - 1].conn_used ) {
                        console_no = ch - '0', console_index = console_no - 1;
                        wprintw(w_cmdline, "Switch to another pgbouncer connection (console %i)", console_no);
                    } else                   
                        wprintw(w_cmdline, "Do not switch because no connection associated (stay on console %i)", console_no);
                    break;
                case 'N':
                    wprintw(w_cmdline, "Create new connection");
                    break;
                case 'W':
                    wprintw(w_cmdline, "Save connections into .pgbrc");
                    break;
                case 'L':
                    wprintw(w_cmdline, "Open current pgbouncer log");
                    break;
                case 'M':
                    wprintw(w_cmdline, "Reload current pgbouncer");
                    break;
                case 'P':
                    wprintw(w_cmdline, "Pause current pgbouncer");
                    break;
                case 'R':
                    wprintw(w_cmdline, "Resume current pgbouncer");
                    break;
                case 'S':
                    wprintw(w_cmdline, "Suspend current pgbouncer");
                    break;
                case 'p':
                    wprintw(w_cmdline, "Show pools");
                    query_context = pools;
                    break;
                case 'c':
                    wprintw(w_cmdline, "Show clients");
                    query_context = clients;
                    break;
                case 's':
                    wprintw(w_cmdline, "Show servers");
                    query_context = servers;
                    break;
                case 'd':
                    wprintw(w_cmdline, "Show databases");
                    wrefresh(w_cmdline);
                    query_context = databases;
                    break;
                case 'a':
                    wprintw(w_cmdline, "Show stats");
                    query_context = stats;
                    break;
                default:
                    wprintw(w_cmdline, "unknown command");
                    break;
            }
        } else {
            res = do_query(conns[console_index], query_context);     /* sent query to the connections using their indexes */
            print_summary(w_summary, progname);
            print_data(res, query_context, w_answer);
            wrefresh(w_cmdline);
            wclear(w_cmdline);
            sleep(1);
        }
    }

    endwin();
    close_connections(conns);
    return 0;
}

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

void create_initial_conn(int argc, char *argv[], struct conn_opts connections[])
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
                strcpy(connections[0].hostaddr, optarg);
                break;
            case 'p':
                strcpy(connections[0].port, optarg);
                break;
            case 'U':
                strcpy(connections[0].user, optarg);
                break;
            case 'd':
                strcpy(connections[0].dbname, optarg);
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
        if ( (argc - optind > 1) && strlen(connections[0].user) == 0 && strlen(connections[0].dbname) == 0 )
            strcpy(connections[0].user, argv[optind]);
        else if ( (argc - optind >= 1) && strlen(connections[0].dbname) == 0 )
            strcpy(connections[0].dbname, argv[optind]);
        else
            fprintf(stderr, "%s: warning: extra command-line argument \"%s\" ignored\n", argv[0], argv[optind]);

        optind++;
    }    

    if ( strlen(connections[0].hostaddr) == 0 )
        strcpy(connections[0].hostaddr, DEFAULT_HOSTADDR);

    if ( strlen(connections[0].port) == 0 )
        strcpy(connections[0].port, DEFAULT_PORT);

    if ( strlen(connections[0].user) == 0 ) {
        strcpy(connections[0].user, pw->pw_name);
    }

    if ( prompt_password == TRI_YES )
        strcpy(connections[0].password, simple_prompt("Password: ", 100, false));

    if ( strlen(connections[0].dbname) == 0 && strlen(connections[0].user) == 0 ) {
        strcpy(connections[0].user, pw->pw_name);
        strcpy(connections[0].dbname, DEFAULT_DBNAME);
    }
    else if ( strlen(connections[0].user) != 0 && strlen(connections[0].dbname) == 0 )
        strcpy(connections[0].dbname, connections[0].user);

    connections[0].conn_used = true;
}

int create_pgbrc_conn(int argc, char *argv[], struct conn_opts connections[], const int pos)
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
                connections[i].hostaddr, connections[i].port, connections[i].dbname, connections[i].user, connections[i].password);
            connections[i].terminal = i;
            connections[i].conn_used = true;
            i++;
        }
        fclose(fp);
        return PGBRC_READ_OK;
    } else {
        printf("WARNING: failed to open %s. Try use defaults.\n", pgbrcpath);
        return PGBRC_READ_ERR;
    }
}

void print_conn(struct conn_opts connections[])
{
    int i;
    for ( i = 0; i < MAX_CONSOLE; i++ )
        if (connections[i].conn_used)
            printf("qq: %s\n", connections[i].conninfo);
}

void prepare_conninfo(struct conn_opts connections[])
{
    int i;
    for ( i = 0; i < MAX_CONSOLE; i++ )
        if (connections[i].conn_used) {
            strcat(connections[i].conninfo, "hostaddr=");
            strcat(connections[i].conninfo, connections[i].hostaddr);
            strcat(connections[i].conninfo, " port=");
            strcat(connections[i].conninfo, connections[i].port);
            strcat(connections[i].conninfo, " user=");
            strcat(connections[i].conninfo, connections[i].user);
            strcat(connections[i].conninfo, " dbname=");
            strcat(connections[i].conninfo, connections[i].dbname);
            strcat(connections[i].conninfo, " password=");
            strcat(connections[i].conninfo, connections[i].password);
        }
}

void open_connections(struct conn_opts connections[], PGconn * conns[])
{
    int i;
    for ( i = 0; i < MAX_CONSOLE; i++ ) {
        if (connections[i].conn_used) {
            conns[i] = PQconnectdb(connections[i].conninfo);
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

void print_data(PGresult *res, enum context query_context)
{
    int    row_count, col_count, row, col, i;
    move (1, 0);
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

    attron(A_BOLD);                                 /* print header with bold */
    for ( col = 0, i = 0; col < col_count; col++, i++ )
        printw("%-*s", columns[i].width, PQfname(res,col));
    printw("\n");
    attroff(A_BOLD);                                /* disable bold */

    for ( row = 0; row < row_count; row++ ) {
        for ( col = 0, i = 0; col < col_count; col++, i++ ) {
            printw("%-*s", columns[i].width, PQgetvalue(res, row, col));
        }
    printw("\n");
    }
    printw("\n");
    refresh();
    sleep(1);
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


int main (int argc, char *argv[])
{
    PGresult    *res;                                   /* query result */
    struct conn_opts conn_opts[MAX_CONSOLE] = { 
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

    while (1) {
        if (key_is_pressed()) {
            ch = getch();
            switch (ch) {
                case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8':
                    if ( conn_opts[ch - '0' - 1].conn_used ) {
                        console_no = ch - '0', console_index = console_no - 1;
                        printw("Switch to another pgbouncer connection (console %i)\n", console_no);
                    } else                   
                        printw("Do not switch because no connection associated (stay on console %i)\n", console_no);
                    break;
                case 'N':
                    printw("Create new connection\n");
                    break;
                case 'W':
                    printw("Save connections into .pgbrc\n");
                    break;
                case 'L':
                    printw("Open current pgbouncer log\n");
                    break;
                case 'M':
                    printw("Reload current pgbouncer\n");
                    break;
                case 'P':
                    printw("Pause current pgbouncer\n");
                    break;
                case 'R':
                    printw("Resume current pgbouncer\n");
                    break;
                case 'S':
                    printw("Suspend current pgbouncer\n");
                    break;
                case 'p':
                    printw("Show pools\n");
                    query_context = pools;
                    break;
                case 'c':
                    printw("Show clients\n");
                    query_context = clients;
                    break;
                case 's':
                    printw("Show servers\n");
                    query_context = servers;
                    break;
                case 'd':
                    printw("Show databases\n");
                    query_context = databases;
                    break;
                case 'a':
                    printw("Show stats\n");
                    query_context = stats;
                    break;
                default:
                    printw("unknown command\n");
                    break;
            }
        } else {
            res = do_query(conns[console_index], query_context);     /* sent query to the connections using their indexes */
            print_data(res, query_context);
            refresh();
            sleep(1);
            clear();
        }
    }

    endwin();
    close_connections(conns);
    return 0;
}

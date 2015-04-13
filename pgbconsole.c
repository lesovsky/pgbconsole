#include "pgbconsole.h"

void create_initial_conn(int argc, char *argv[], struct conn_opts connections[])
{
    int param, option_index;
    enum trivalue prompt_password = TRI_DEFAULT;
    pw = getpwuid(getuid());     // get current user info: pw_name,pw_uid,pw_gid,pw_dir,pw_shell.

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
    FILE *fp;
    struct stat statbuf;
    char strbuf[BUFFERSIZE];
    int i = pos;
    pw = getpwuid(getuid());

    strcpy(pgbrcpath, pw->pw_dir);
    strcat(pgbrcpath, "/");
    strcat(pgbrcpath, PGBRC_FILE);

    stat(pgbrcpath, &statbuf);
    if ( statbuf.st_mode & (S_IRWXG | S_IRWXO) ) {
        printf("WARNING: %s has wrong permissions.\n", pgbrcpath);
        return PGBRC_READ_ERR;
    }

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

void open_connections(struct conn_opts connections[])
{
    int i;
    for ( i = 0; i < MAX_CONSOLE; i++ ) {
        conns[i] = PQconnectdb(connections[i].conninfo);
        if ( PQstatus(conns[i]) == CONNECTION_BAD )
            puts("We were unable to connect to the database");
    }
}

void close_connections(PGconn * conns[])
{
    int i;
    PQclear;
    for (i = 0; i < MAX_CONSOLE; i++)
        PQfinish(conns[i]);
}

PGresult * do_query(PGconn * conns[], char query[])
{
    PGresult    *res;
    res = PQexec(conns[0], "show pools");
    if ( PQresultStatus(res) != PGRES_TUPLES_OK ) {
        puts("We didn't get any data.");
        return NULL;
    }
    else
        return res;
}

void show_pools_output(PGresult *res)
{
    int    rec_count, row, col;
    move (1, 0);
    rec_count = PQntuples(res);
    attron(A_BOLD);
    printw("%s\n", SHOW_POOLS_HEADER);
    attroff(A_BOLD);
    for ( row = 0; row < rec_count; row++ ) {
        for ( col = 0; col < SHOW_POOLS_COLUMNS_NUM; col++ ) {
            printw("%12s", PQgetvalue(res, row, col));
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
    PGconn      *conn;
    PGresult    *res;
/*
 * Проверяем наличие входных параметров:
 * 1) если есть, то запоминаем их в структуру коннекта
 * 2) проверяем наличие .pgbrc
 * 2.1 если .pgbrc есть формируем дополнительные структуры
 * 2.2 если .pgbrc нет, используем те параметр что были переданы в строке запуска
 * 3) если входных параметров нет, формируем подключение из дефолтных значений
 */
    if ( argc > 1 ) {                                           /* input parameters specified */
        create_initial_conn(argc, argv, connections);           /* save input parameters */
        create_pgbrc_conn(argc, argv, connections, 1);
    } else                                                     /* input parameters not specified */
        if (create_pgbrc_conn(argc, argv, connections, 0) == PGBRC_READ_ERR)
            create_initial_conn(argc, argv, connections);

    prepare_conninfo(connections);
    print_conn(connections);
    open_connections(connections);

    initscr();
    while (1) {
        res = do_query(conns, "show pools");
        show_pools_output(res);
    }
    endwin();

    close_connections(conns);
    return 0;
}

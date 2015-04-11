#include "pgbconsole.h"

void create_initial_conn(int argc, char *argv[], struct conn_opts connections[])
{
    int param, option_index;
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
            case '?':
                printf("invoke help printing function here\n", optarg);
                exit (EXIT_SUCCESS);
                break;
            default: 
                printf("unknown option.\n");
                exit(EXIT_FAILURE);
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
    char strbuf[4096], a[4096], b[4096], c[4096], d[4096];
    int i = pos;
    pw = getpwuid(getuid());

    strcpy(pgbrcpath, pw->pw_dir);
    strcat(pgbrcpath, "/");
    strcat(pgbrcpath, PGBRC_FILE);

    if ((fp = fopen(pgbrcpath, "r")) != NULL) {
        while (fgets(strbuf, 4096, fp) != 0) {
            sscanf(strbuf, "%[^:]:%[^:]:%[^:]:%[^:\n]", a, b, c, d);
            connections[i].terminal = i;
            connections[i].conn_used = true;
            strcpy(connections[i].hostaddr, a);
            strcpy(connections[i].port, b);
            strcpy(connections[i].user, c);
            strcpy(connections[i].dbname, d);
            i++;
        }
        return PGBRC_READ_OK;
    } else {
        printf("WARNING: failed to open %s. try use defaults\n", pgbrcpath);
        return PGBRC_READ_ERR;
    }
}

void print_conn(struct conn_opts connections[])
{
    int i;
    for ( i = 0; i < MAX_CONSOLE; i++ )
        if (connections[i].conn_used)
            printf("qq: %i %s %s %s %s\n", connections[i].terminal, connections[i].hostaddr, connections[i].port, connections[i].user, connections[i].dbname);
}

int main (int argc, char *argv[])
{
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

    print_conn(connections);

    return 0;
}

#include "pgbconsole.h"

struct conn_opts create_initial_conn(int argc, char *argv[], struct conn_opts connections[])
{
    int param, option_index;
    pw = getpwuid(getuid());     // get current user info: pw_name,pw_uid,pw_gid,pw_dir,pw_shell.

    connections[0].hostaddr = "";
    connections[0].port = "";
    connections[0].user = "";
    connections[0].dbname = "";

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
                connections[0].hostaddr = optarg;
                break;
            case 'p':
                connections[0].port = optarg;
                break;
            case 'U':
                connections[0].user = optarg;
                break;
            case 'd':
                connections[0].dbname = optarg;
                break;
            case 'H':
                printf("invoke help printing function here\n", optarg);
                exit (EXIT_SUCCESS);
                break;
            case '?': default: 
                printf("unknown option.\n");
                break;
        }
    }

    while (argc - optind >= 1)
    {
        if ( (argc - optind > 1) && connections[0].user == "" && connections[0].dbname == "" )
            connections[0].user = argv[optind];
        else if ( (argc - optind >= 1) && connections[0].dbname == "" )
            connections[0].dbname = argv[optind];
        else
            fprintf(stderr, "%s: warning: extra command-line argument \"%s\" ignored\n", argv[0], argv[optind]);

        optind++;
    }    

    if ( connections[0].hostaddr == "" )
        connections[0].hostaddr = DEFAULT_HOSTADDR;

    if ( connections[0].port == "" )
        connections[0].port = DEFAULT_PORT;

    if ( connections[0].user == "" ) {
        connections[0].user = pw->pw_name;
    }

    if ( connections[0].dbname == "" && connections[0].user == "" ) {
        connections[0].user = pw->pw_name;
        connections[0].dbname = DEFAULT_DBNAME;
    }
    else if ( connections[0].user != "" && connections[0].dbname == "" )
        connections[0].dbname = connections[0].user;
}

int check_pgbrc(void)
{
    struct stat statbuf;
    pw = getpwuid(getuid());

    strcpy(pgbrcpath, pw->pw_dir);
    strcat(pgbrcpath, "/");
    strcat(pgbrcpath, PGBRC_FILE);

    if (stat(pgbrcpath, &statbuf) == -1) 
        return(PGBRC_NOT_EXIST);
    else if (statbuf.st_mode & ( S_IRWXG | S_IRWXO) )
        return(PGBRC_WRONG);
    else
        return(PGBRC_EXIST);
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
        if (check_pgbrc() == PGBRC_EXIST)
            printf("parameters specified, use them and try to read pgbrc here\n");                        /* if .pgbrc exist read them too */
    } else {                                                    /* input parameters not specified */
        switch (check_pgbrc()) {                                /* check .pgbrc */
        case PGBRC_WRONG:                                       /* .pgbrc has wrong permissions */
            printf("WARNING: .pgbrc has wrong permissions.\n");
            create_initial_conn(argc, argv, connections);       /* try use defaults for connection */
            break;
        case PGBRC_NOT_EXIST:                                   /* .pgbrc does not exist */
            create_initial_conn(argc, argv, connections);       /* try use defaults for connection */
            break;
        case PGBRC_EXIST: default:                              /* .pgbrc exist */
            printf("parameters not specidied, try read pgbrc here\n");                        /* try read him */
            break; 
        }
    }

    printf("%s %s %s %s\n", connections[0].hostaddr, connections[0].port, connections[0].user, connections[0].dbname);

    return 0;
}

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
    if ( argc == 1 )
    {
        printf("here we must check .pgbrc file, otherwise use defaults\n");
        create_initial_conn(argc, argv, connections);
    }
    else
        create_initial_conn(argc, argv, connections);

    printf("%s %s %s %s\n", connections[0].hostaddr, connections[0].port, connections[0].user, connections[0].dbname);
//    printf("%s %s %s %s\n", co1.hostaddr, co1.port, co1.user, co1.dbname);

    return 0;
}

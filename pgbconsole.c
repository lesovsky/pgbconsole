#include "pgbconsole.h"

struct conn_opts arg_parse(int argc, char *argv[], struct conn_opts *conn)
{
    struct passwd *pw = getpwuid(getuid());     // get current user info: pw_name,pw_uid,pw_gid,pw_dir,pw_shell.

    conn->hostaddr = "";
    conn->port = "";
    conn->user = "";
    conn->dbname = "";

    int param, option_index;

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
                conn->hostaddr = optarg;
                break;
            case 'p':
                conn->port = optarg;
                break;
            case 'U':
                conn->user = optarg;
                break;
            case 'd':
                conn->dbname = optarg;
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
        if ( (argc - optind > 1) && conn->user == "" && conn->dbname == "" )
            conn->user = argv[optind];
        else if ( (argc - optind >= 1) && conn->dbname == "" )
            conn->dbname = argv[optind];
        else
            fprintf(stderr, "%s: warning: extra command-line argument \"%s\" ignored\n", argv[0], argv[optind]);

        optind++;
    }    

    if ( conn->hostaddr == "" )
        conn->hostaddr = DEFAULT_HOSTADDR;

    if ( conn->port == "" )
        conn->port = DEFAULT_PORT;

    if ( conn->user == "" ) {
        conn->user = pw->pw_name;
    }

    if ( conn->dbname == "" && conn->user == "" ) {
        conn->user = pw->pw_name;
        conn->dbname = DEFAULT_DBNAME;
    }
    else if ( conn->user != "" && conn->dbname == "" )
        conn->dbname = conn->user;
}

int main (int argc, char *argv[])
{
    // Functions declaration.
    struct conn_opts arg_parse(int argc, char *argv[], struct conn_opts *conn);

    // Structures declaration.
    struct conn_opts adhoc_opts;
    struct conn_opts *conn = &adhoc_opts;
   
    // проверяем есть ли входные аргументы
    if ( argc == 1 )
    {
        printf("here we must check .pgbrc file, otherwise use defaults\n");
        arg_parse(argc, argv, conn);
    }
    else
        arg_parse(argc, argv, conn);

    printf("%s %s %s %s\n", conn->hostaddr, conn->port, conn->user, conn->dbname);


    return 0;
}

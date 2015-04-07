#include "pgbconsole.h"

void conns_create_list(void)
{
    co1.prev = '\0', co1.next = &co2;
    co2.prev = &co1, co2.next = &co3;
    co3.prev = &co2, co3.next = &co4;
    co4.prev = &co3, co4.next = &co5;
    co5.prev = &co4, co5.next = &co6;
    co6.prev = &co5, co6.next = &co7;
    co7.prev = &co6, co7.next = &co8;
    co8.prev = &co7, co8.next = '\0';

    conns_list_head = &co1;
}

struct conn_opts arg_parse(int argc, char *argv[], struct conn_opts *conns_list_ptr)
{
    pw = getpwuid(getuid());     // get current user info: pw_name,pw_uid,pw_gid,pw_dir,pw_shell.
    conns_list_ptr = conns_list_head;

    conns_list_ptr->hostaddr = "";
    conns_list_ptr->port = "";
    conns_list_ptr->user = "";
    conns_list_ptr->dbname = "";

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
                conns_list_ptr->hostaddr = optarg;
                break;
            case 'p':
                conns_list_ptr->port = optarg;
                break;
            case 'U':
                conns_list_ptr->user = optarg;
                break;
            case 'd':
                conns_list_ptr->dbname = optarg;
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
        if ( (argc - optind > 1) && conns_list_ptr->user == "" && conns_list_ptr->dbname == "" )
            conns_list_ptr->user = argv[optind];
        else if ( (argc - optind >= 1) && conns_list_ptr->dbname == "" )
            conns_list_ptr->dbname = argv[optind];
        else
            fprintf(stderr, "%s: warning: extra command-line argument \"%s\" ignored\n", argv[0], argv[optind]);

        optind++;
    }    

    if ( conns_list_ptr->hostaddr == "" )
        conns_list_ptr->hostaddr = DEFAULT_HOSTADDR;

    if ( conns_list_ptr->port == "" )
        conns_list_ptr->port = DEFAULT_PORT;

    if ( conns_list_ptr->user == "" ) {
        conns_list_ptr->user = pw->pw_name;
    }

    if ( conns_list_ptr->dbname == "" && conns_list_ptr->user == "" ) {
        conns_list_ptr->user = pw->pw_name;
        conns_list_ptr->dbname = DEFAULT_DBNAME;
    }
    else if ( conns_list_ptr->user != "" && conns_list_ptr->dbname == "" )
        conns_list_ptr->dbname = conns_list_ptr->user;
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
    conns_create_list();
    if ( argc == 1 )
    {
        printf("here we must check .pgbrc file, otherwise use defaults\n");
        arg_parse(argc, argv, conns_list_ptr);
    }
    else
        arg_parse(argc, argv, conns_list_ptr);

    conns_list_ptr = conns_list_head;
    printf("%s %s %s %s\n", conns_list_ptr->hostaddr, conns_list_ptr->port, conns_list_ptr->user, conns_list_ptr->dbname);
//    printf("%s %s %s %s\n", co1.hostaddr, co1.port, co1.user, co1.dbname);

    return 0;
}

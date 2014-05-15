#include "conf.h"

#define ZOODISR_CONFIG_LINE_MAX     1024

void load_conf_from_string(char *config)
{
    char *err = NULL;
    int linenum = 0, tlines, i;
    sds *lines;

    lines = sdssplitlen(config, strlen(config), "\n", 1, &tlines);

    for(i = 0; i < tlines; i++)
    {
        sds *argv;
        int argc;

        linenum = i+1;

        log_debug(">> %0.3d %s", linenum,  lines[i]);

        lines[i] = sdstrim(lines[i], " \t\r\n");

        // skip comments and blank lines
        if(lines[i][0] == '#' || lines[i][0] == '\0')
            continue;

        argv = sdssplitargs(lines[i], &argc);
        if(argv == NULL)
        {
            err = "Unbalanced quotes in configuration line";
            goto loaderr;
        }

        if(argc == 0)
        {
            sdsfreesplitres(argv, argc);
            continue;
        }

        sdstolower(argv[0]);

        if(!strcasecmp(argv[0], "name") && argc == 2)
        {
            server.name = sdsdup(argv[1]);
        }else if(!strcasecmp(argv[0], "bind") && argc >= 2)
        {
            server.bind_addr_count = argc-1;

            if((argc-1) > ZR_MAX_BIND_ADDR)
            {
                err = "Too many bind address specified.";
                goto loaderr;
            }
            int j;

            for(j = 0; j < server.bind_addr_count; j++)
            {
                server.bind_arr[j].addr_str = sdsnew(argv[j+1]);
                server.bind_arr[j].addr.sin_family = AF_INET;
                server.bind_arr[j].addr.sin_addr.s_addr = inet_addr(argv[j+1]);
                if(server.bind_arr[j].addr.sin_addr.s_addr == INADDR_NONE)
                {
                    err = sdscatprintf(sdsempty(), "Wrong binding ip address: %s", argv[j+1]);
                    goto loaderr;
                }
            }
        }else if(!strcasecmp(argv[0], "port") && argc == 2)
        {
            server.port = atoi(argv[1]);

            if (server.port < 0 || server.port > 65535)
            {
                err = "Invalid port"; goto loaderr;
            }

        }else if(!strcasecmp(argv[0], "req-log-uds") && argc == 2)
        {
            server.req_log_uds = sdsnew(argv[1]);

        }else if(!strcasecmp(argv[0], "client-max") && argc == 2)
        {
            server.client_max = atoi(argv[1]);
            if(server.client_max < 0)
            {
                err = "Invalid server-connection-max value.";
                goto loaderr;
            }
        } else if (!strcasecmp(argv[0],"tcp-backlog") && argc == 2)
        {
            server.tcp_backlog = atoi(argv[1]);
            if (server.tcp_backlog < 0)
            {
                err = "Invalid backlog value";
                goto loaderr;
            }
/*
        } else if (!strcasecmp(argv[0],"rr-counter-buffer") && argc == 2)
        {
            server.tcp_backlog = atoi(argv[1]);
            if (server.tcp_backlog < 0)
            {
                err = "Invalid rr-counter-buffer value";
                goto loaderr;
            }
*/
        }else if(!strcasecmp(argv[0], "log-level") && argc == 2)
        {
            server.log_level = log_get_level_str(argv[1]);
        }else if(!strcasecmp(argv[0], "log-file") && argc == 2)
        {
            server.log_file = sdsdup(argv[1]);
            if(!strcasecmp(argv[1], "stdout"))
            {
                log_set_out(stdout);
            }else if(!strcasecmp(argv[1], "stderr"))
            {
                log_set_out(stderr);
            }else
            {
                FILE *fd;
                fd = fopen(argv[1], "a");
                if(fd == NULL)
                {
                    err = sdscatprintf(sdsempty(), "Can't open the log file: %s", strerror(errno));
                    goto loaderr;
                }
                
                log_set_out(fd);
            }
        }else if(!strcasecmp(argv[0], "zookeeper-health-uri") && argc == 2)
        {
            struct z_conn_info *zcinfo = z_parse_conn_string(argv[1]);
            if(zcinfo == NULL)
            {
                err = "Invalid zookeeper-health-url(Zookeeper URI) value.\n"
//                      "Zookeeper URI must be as below.\n"
//                      " - zoo://[USER[:PASS]@]HOST_STRING/NODE/PATH\n"
//                      "   ex) zoo://192.168.1.2:2181/test/node\n"
//                      "   ex) zoo://foo@192.168.1.2:2181,192.168.1.3:2181/test/node\n"
//                      "   ex) zoo://foo:bar@192.168.1.2:2181,192.168.1.3:2181/test/node\n"
                      "";
                goto loaderr;
            }

            server.z_health_conn = zcinfo;
        }else if(!strcasecmp(argv[0], "zookeeper-timeout") && argc == 2)
        {
            server.zoo_timeout = atoi(argv[1]);
            if(server.zoo_timeout < 0)
            {
                err = "Invalid zookeeper-timeout value";
                goto loaderr;
            }

        }else if(!strcasecmp(argv[0], "service") && argc == 3)
        {
            if(strlen(argv[1]) > ZR_MAX_SVC_NAME)
            {
                err = sdscatprintf(sdsempty(), "Service name is too long, first argument(service name) must be less then %d.", ZR_MAX_SVC_NAME);
                goto loaderr;
            }

            struct z_conn_info *cinfo = z_parse_conn_string(argv[2]);
            if(!cinfo)
            {
                err = "Invalid zookeeper-health-url(Zookeeper URI) value.\n"
//                      "Zookeeper URI must be as below.\n"
//                      " - zoo://[USER[:PASS]@]HOST_STRING/NODE/PATH\n"
//                      "   ex) zoo://192.168.1.2:2181/test/node\n"
//                      "   ex) zoo://foo@192.168.1.2:2181,192.168.1.3:2181/test/node\n"
//                      "   ex) zoo://foo:bar@192.168.1.2:2181,192.168.1.3:2181/test/node\n"
                      "";
                goto loaderr;
            }

            int j;
            for(j = 0; j < server.svc_count; j++)
            {
                struct svc *tsvc = server.svc_arr[j];
                if(!sdscmp(argv[1], tsvc->name))
                {
                    err = sdscatprintf(sdsempty(), "\"%s\" service name already exists.", argv[1]);
                    goto loaderr;
                }
            }

            struct svc *svc = svc_alloc(argv[1], strlen(argv[1]), server.svc_count);
            svc->z_conn = cinfo;
            server.svc_arr[server.svc_count] = svc;
            server.svc_count++;
        }else
        {
            err = "Bad config directive or wrong number of arguments";
            goto loaderr;
        }

        sdsfreesplitres(argv,argc);
    }

    sdsfreesplitres(lines,tlines);
    return;

loaderr:
    log_eerr("CONFIG FILE ERROR.");
    log_eerr("Reading configureation file, at line %d", linenum);
    log_eerr(">> '%s'", lines[i]);
    log_eerr("%s", err);
    exit(1);
}

void load_conf(const char *file_path)
{
    sds config = sdsempty();
    char buf[ZOODISR_CONFIG_LINE_MAX+1];

    if (file_path)
    {
        FILE *fp;

        if (file_path[0] == '-' && file_path[1] == '\0')
        {
            fp = stdin;
        }else
        {
            if ((fp = fopen(file_path,"r")) == NULL)
            {
                log_err("Fatal error, can't open config file '%s'", file_path);
                exit(1);
            }
        }

        log_debug("Configure loading: %s", file_path);

        while(fgets(buf,ZOODISR_CONFIG_LINE_MAX+1,fp) != NULL)
            config = sdscat(config,buf);

        if (fp != stdin) fclose(fp);
    }

    load_conf_from_string(config);

    sdsfree(config);
}

void check_conf()
{
    int res;
    char *err = "";
    char buf[ZR_MAX_HOSTNAME_LEN+1];
    
    // ckeck mandatory options
    if(server.svc_count == 0)
    {
        err = "No any valid service configure, you must set service at least one.";
        goto checkerr;
    }

    /*
    // below block should be in last of mandatory
    if(server.req_log_sock == NULL)
    {
        server.req_log = 0;
        log_info("Request log domain socket disable.");
    }else
    {
        // Validate domain socket
        sds dname = sdsdup(server.req_log_sock);
        dname = dirname(dname);
        if(access(dname, F_OK) < 0)
        {
            err = sdscatprintf(sdsempty(), "Permission denied for req-log-sock: %s, %s", dname, strerror(errno));
            goto checkerr;
        }

        struct stat st;
        res = stat(server.req_log_sock, &st);
        if(res == 0)
        {
            log_debug("req-log-sock:%s", server.req_log_sock);
            err = sdscatprintf(sdsempty(), "Request log socket already in use: %s", server.req_log_sock);
            goto checkerr;
        }
        sdsfree(dname);
    }
    // end ckeck mandatory options
    */

    log_set_level(server.log_level);
    z_set_log_level(server.log_level);
    zoo_set_log_stream(log_get_out());

    // check optional options
    if(server.name == NULL)
    {
        res = gethostname(buf, ZR_MAX_HOSTNAME_LEN);
        if(res < 0)
        {
            err = sdscatprintf(sdsempty(), "Cann't get hostname %s", strerror(errno));
            goto checkerr;
        }

        server.name = sdscatprintf(sdsnew(buf), "-%d", getpid());
        log_info("Set name by default: %s", server.name);
    }

    if(server.bind_addr_count == 0)
    {
        server.bind_addr_count = 1;
        server.bind_arr[0].addr_str = sdsnew("INADDR_ANY");
        server.bind_arr[0].addr.sin_family = AF_INET;
        server.bind_arr[0].addr.sin_addr.s_addr = htonl(INADDR_ANY);

        log_info("Set bind by default: %s", "INADDR_ANY");
    }

    if(server.port == 0)
    {
        server.port = ZR_PORT;
        log_info("Set port by default: %d", server.port);
    }

    if(server.client_max == 0)
    {
        server.client_max = ZR_CONN_MAX;
        log_info("Set connection-max by default: %d", server.client_max);
    }

    if(server.tcp_backlog == 0)
    {
        server.tcp_backlog = 500;
        log_info("Set tcp-backlog by default: %d", server.tcp_backlog);
    }

    if(server.z_health_conn == NULL)
    {
        server.z_health = 0;
    }else
    {
        server.z_health = 1;
        server.z_health_node = sdscatprintf(sdsempty(), "%s/%s", server.z_health_conn->node, server.name);
    }

    if(server.zoo_timeout == 0)
    {
        server.zoo_timeout = ZR_ZOO_TIMEOUT;
    }

    if(server.req_log_uds == NULL)
    {
        server.req_log_uds = sdsnew(ZR_DEFAULT_REQ_LOG_UDS);
    }

    return;

checkerr:
    log_eerr("CONFIG FILE CHECK ERROR.");
    log_eerr("%s", err);
    exit(1);
}

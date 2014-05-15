#include "zoodisr.h"
#include "zoo_util.h"

int main(void)
{
    log_init();
    log_set_level_debug();
    struct z_conn_info *info;
    
    info = z_parse_conn_string("zoo://hello:pass@192.168.1.2:2000,192.168.1.3:2000/ESOCache/routers");
    if(info == NULL)
    {
        log_err("Not matched.");
        exit(1);
    }

    log_err("Info zoo://%s:%s@%s%s", info->user, info->pass, info->hosts, info->node);

    info = z_parse_conn_string("zoo://192.168.1.2:2000,192.168.1.3:2000/ESOCache/routers");
    if(info == NULL)
    {
        log_err("Not matched.");
        exit(1);
    }

    log_err("Info zoo://%s:%s@%s%s", info->user, info->pass, info->hosts, info->node);
    //log_err("Info %d zoo://%s%s", info->with_user_info, info->hosts, info->node);

    info = z_parse_conn_string("zoo://help:@192.168.1.2:2000,192.168.1.3:2000/ESOCache/routers");
    if(info == NULL)
    {
        log_err("Not matched.");
        exit(1);
    }

    log_err("Info zoo://%s:%s@%s%s", info->user, info->pass, info->hosts, info->node);

    return 0;
}

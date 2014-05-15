#include "zoodisr.h"

int main(int argc, const char *argv[])
{
    log_init(LOG_DEFAULT);
    signal(SIGPIPE, SIG_IGN);

    event_set_log_callback(event_log_callback);

    log_msg("Starting zoodis router...");

    server_init_default();
    load_conf("../conf/zoodisr.conf");
    check_conf();

    server_run_prepare();

    server_evbase_init();
    server_binding();
    server_rl_binding();

    server_run();

    //sleep(100000);

    return 0;
}


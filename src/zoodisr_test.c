#include <signal.h>

#include "zoodisr.h"

#define MAX_BUF_LEN 8196

static int read_cnt;
static char *read_ptr;
static char read_buf[MAX_BUF_LEN];

static ssize_t my_read(int fd, char *ptr);
ssize_t readline(int fd, void *vptr, size_t maxlen);
ssize_t readlinebuf(void **vptrptr);

struct msg* read_msg(int fd);
struct msg* read_msg2(int fd);
static void write_msg_ping(int fd);
static void write_msg_get_stats(int fd);
static void write_msg_get_node(int fd);
static pthread_mutex_t lock;

struct thread_info
{
    pthread_t   thread;
    int         count;
    int         ec;
    uint64_t    t_total;
    int         no;
};

static int threads = 1, finished = 0;
static struct thread_info **ttinfo;
static char *ip;
static int port, count = 1, printable = 1;

void print_result()
{
    int i;
    uint64_t h, m, s, us, elapsed_time = 0;
    for(i = 0; i < threads; i++)
    {
        elapsed_time += ttinfo[i]->t_total;
        printf("Thread %d, elapsed %"PRIu64"\n", i, ttinfo[i]->t_total);
    }

    h = elapsed_time / ((uint64_t)3600*(uint64_t)1000000);
    m = (elapsed_time % ((uint64_t)3600*(uint64_t)1000000)) / ((uint64_t)60*(uint64_t)1000000);
    s = ((elapsed_time % ((uint64_t)3600*(uint64_t)1000000)) % ((uint64_t)60*(uint64_t)1000000)) / 1000000;
    us = elapsed_time % 1000000;

    printf("Total threads: %d\n", threads);
    printf("Total count: %d\n", count);
    printf("Time elapsed total: %"PRIu64"h %"PRIu64"m %"PRIu64"s %"PRIu64"us\n", h, m, s, us);
    //printf("Time elapsed avg: %"PRIu64"us\n", elapsed_time / count);
    printf("Time elapsed avg: %"PRIu64"us\n", elapsed_time / (count*3));
}

void* execute(void *data)
{
    struct thread_info *tinfo = (struct thread_info*) data;
    tinfo->t_total = 0;
    int client_len;
    int client_sockfd;

    struct sockaddr_in clientaddr;

    //static sigset_t  signal_mask;
    //sigemptyset (&signal_mask);
    //sigaddset (&signal_mask, SIGALRM);
    //sigaddset (&signal_mask, SIGPIPE);
    //pthread_sigmask(SIG_UNBLOCK, &signal_mask, NULL);

    client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    clientaddr.sin_family = AF_INET;
    clientaddr.sin_addr.s_addr = inet_addr(ip);
    clientaddr.sin_port = htons(port);

    client_len = sizeof(clientaddr);

    if (connect(client_sockfd, (struct sockaddr *)&clientaddr, client_len) < 0)
    {
        printf("Connect error.\n");
        exit(0);
    }

    struct msg *rmsg = NULL;

    uint64_t st, et;

    int i;
    for(i = 0; i < tinfo->count; i++)
    {

        st = utime_time();
        write_msg_ping(client_sockfd);

        rmsg = read_msg2(client_sockfd);
        msg_free(rmsg);
        et = utime_time();
        tinfo->t_total += et - st;
        if(printable) printf("Ping cmd Time %"PRIu64"uc\n", et - st);

        st = utime_time();
        write_msg_get_stats(client_sockfd);

        rmsg = read_msg2(client_sockfd);
        msg_free(rmsg);
        et = utime_time();
        tinfo->t_total += et - st;
        if(printable) printf("GetStat cmd Time %"PRIu64"uc\n", et - st);


        st = utime_time();
        write_msg_get_node(client_sockfd);

        rmsg = read_msg2(client_sockfd);
        msg_free(rmsg);
        et = utime_time();
        tinfo->t_total += et - st;
        if(printable) printf("GetNode cmd Time %"PRIu64"uc\n", et - st);
        tinfo->ec++;
    }

    //printf("Close socket.\n");
    close(client_sockfd);
    pthread_mutex_lock(&lock);
    finished--;
    pthread_mutex_unlock(&lock);

//    printf("### threads:%d finished:%d\n", threads, finished);

    pthread_exit(NULL);
    return NULL;
}


int main(int argc, char *argv[])
{
    log_init(LOG_INFO);

    signal(SIGPIPE, SIG_IGN);

    pthread_mutex_init(&lock, NULL);

    int i;
    
    if(argc < 3)
    {
        printf("Usage : ]$ %s IP PORT [ROUNDS] [THREADS]\n", argv[0]);
        exit(1);
    }

    ip = argv[1];
    port  = atoi(argv[2]);

    if(argc > 3)
        count = atoi(argv[3]);
    
    if(argc > 4)
        threads = atoi(argv[4]);

    if(argc > 5)
        printable = atoi(argv[5]);


    int count_per_thread = count / threads;

    ttinfo = (struct thread_info**) calloc(sizeof(struct thread_info*), threads);

    printf("Threads:%d, Count:%d, Count per thread:%d\n", threads, count, count_per_thread);
    
    for(i = 0; i < threads; i++)
    {
        //printf("Start thread %d\n", i);
        ttinfo[i] = calloc(sizeof(struct thread_info), 1);

        
        if(i == 0)
            ttinfo[i]->count = count_per_thread + (count % threads);
        else
            ttinfo[i]->count = count_per_thread;

        ttinfo[i]->t_total = 0;
        ttinfo[i]->ec = 0;
        ttinfo[i]->no = i;
        pthread_create(&(ttinfo[i]->thread), NULL, execute, (void *)ttinfo[i]);
        pthread_detach(ttinfo[i]->thread);

        pthread_mutex_lock(&lock);
        finished++;
        pthread_mutex_unlock(&lock);
    }

    /*
    for(i = 0; i < threads; i++)
    {
        struct thread_info *info;
        pthread_join(ttinfo[i]->thread, (void**)&info);
    }
    */


    while(finished)
    {
        usleep(10);
    }

    print_result();

    pthread_exit(NULL);
    exit(0);
}

struct msg* read_msg2(int fd)
{
    int len;
    char buf[4096];
    len = recv(fd, buf, 4095, 0);
    struct msg *msg = NULL;

//    printf("=== %d %.*s", len, len, buf);

    if(len < 0)
        printf("Recv error, %s\n", strerror(errno));
    else
        msg = msg_alloc_with_line(buf, len);

    return msg;
}

struct msg* read_msg(int fd)
{
    char buf[4096];
    int len;
    struct msg *msg = msg_alloc();

    while(1)
    {
        len = readline(fd, buf, 4095);
        if(len == 0)
        {
            printf("Connection closed. %s\n", strerror(errno));
            exit(1);
        }
        if(len == 2 && (buf[0] == '\r' || buf[0] == '\n'))
        {
            if(msg) return msg;

        }
        msg_append_dup(msg, buf, len);
    }
    return msg;
}

static void write_msg_ping(int fd)
{
    send(fd, "+Ping\r\n\r\n", 9, 0);
}

static void write_msg_get_stats(int fd)
{
    send(fd, "+GetStats\r\n\r\n", 13, 0);
}

static void write_msg_get_node(int fd)
{
    send(fd, "+GetNode\r\n0\r\n1\r\n\r\n", 18, 0);
}

static ssize_t my_read(int fd, char *ptr)
{
    if (read_cnt <= 0)
    {
        again:
        if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0)
        {
            if (errno == EINTR)
                goto again;
            return (-1);
        } else
        {
            if (read_cnt == 0)
            return (0);
        }
        read_ptr = read_buf;
    }

    read_cnt--;
    *ptr = *read_ptr++;
    return (1);
}

ssize_t readline(int fd, void *vptr, size_t maxlen)
{
    ssize_t n, rc;
    char    c, *ptr;

    ptr = vptr;
    for (n = 1; n < maxlen; n++) {
        if ( (rc = my_read(fd, &c)) == 1) {
            *ptr++ = c;
            if (c  == '\n')
                break;          /* newline is stored, like fgets() */
        } else if (rc == 0) {
            *ptr = 0;
            return (n - 1);     /* EOF, n - 1 bytes were read */
        } else
            return (-1);        /* error, errno set by read() */
    }

    *ptr  = 0;                  /* null terminate like fgets() */
    return (n);
}

ssize_t readlinebuf(void **vptrptr)
{
    if (read_cnt)
        *vptrptr = read_ptr;
    return (read_cnt);
}

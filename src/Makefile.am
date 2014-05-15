bin_PROGRAMS = zoodisr zoodisr-req-log zoodisr-test
lib_LIBRARIES = libzoodisr.a
libzoodisr_a_SOURCES = \
                       countque.c \
                       commands.c \
                       sds.c \
                       md5.c \
                       logging.c \
                       utime.c \
                       ketama.c \
                       dict.c \
                       svc.c \
                       server.c \
                       event_util.c \
                       client.c \
                       rlclient.c \
                       node.c \
                       msg.c \
                       rcounter.c \
                       zoo_util.c \
                       conf.c
libzoodisr_a_CFLAGS = -Wall -g
zoodisr_SOURCES = libzoodisr.a zoodisr.c
zoodisr_LDFLAGS = 
zoodisr_CFLAGS = -Wall -g
zoodisr_LDADD = libzoodisr.a

zoodisr_req_log_SOURCES = libzoodisr.a zoodisr_req_log.c
zoodisr_req_log_LDFLAGS = 
zoodisr_req_log_CFLAGS = -Wall -g
zoodisr_req_log_LDADD = libzoodisr.a

zoodisr_test_SOURCES = libzoodisr.a zoodisr_test.c
zoodisr_test_LDFLAGS = 
zoodisr_test_CFLAGS = -Wall -g
zoodisr_test_LDADD = libzoodisr.a

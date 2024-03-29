#ifndef STATUS_H
#define STATUS_H

typedef void* status_t;

#define ALL_OK              0
#define EIO                 1
#define EINVARG             2
#define ENOMEM              3
#define EINVPATH            4
#define EFSNOTSUPPORTED     5
#define EREADONLY           6
#define ETOOMANYPROCESSES   7
#define EPAGEFAULT          8
#define ENOCALLBACK         9
#define ETOOMANYDRIVERS     10
#define EFILENOTSUPPORTED   11
#define ETOOMANYPROCMALLOCS 12
#define ETOOMANYARGS        13

#define ERROR(v) ((void*)(-v))

#endif

#ifndef AUDIOPLAYER_WLQUEUE_H
#define AUDIOPLAYER_WLQUEUE_H

#include "queue"
#include "pthread.h"
#include "WlPlaystatus.h"
#include <android/log.h>

extern "C" {
#include "libavcodec/avcodec.h"
};

class WlQueue {
public:
    std::queue<AVPacket *> queuePacket;
    pthread_mutex_t mutexPacket;
    pthread_cond_t condPacket;
    WlPlaystatus *playstatus = NULL;

public:
    WlQueue(WlPlaystatus *playstatus);
    ~WlQueue();

    int putAvpacket(AVPacket *packet);
    int getAvpacket(AVPacket *packet);

    int getQueueSize();
    void clearAvpacket();
};

#endif //AUDIOPLAYER_WLQUEUE_H

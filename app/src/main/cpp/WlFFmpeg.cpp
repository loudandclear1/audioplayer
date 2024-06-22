#include "WlFFmpeg.h"

WlFFmpeg::WlFFmpeg(WlPlaystatus *playstatus, WlCallJava *callJava, const char *url) {
    this->playstatus = playstatus;
    this->callJava = callJava;
    this->url = url;
    exit = false;
    pthread_mutex_init(&init_mutex, NULL);
}

void *decodeFFmpeg(void *data) {
    WlFFmpeg *wlFFmpeg = (WlFFmpeg *) data;
    wlFFmpeg->decodeFFmpegThread();
    pthread_exit(&wlFFmpeg->decodeThread);
}

void WlFFmpeg::prepared() {
    pthread_create(&decodeThread, NULL, decodeFFmpeg, this);
}

int avformat_callback(void *ctx) {
    WlFFmpeg *fFmpeg = (WlFFmpeg *) ctx;
    if (fFmpeg->playstatus->exit) {
        return AVERROR_EOF;
    }
    return 0;
}

void WlFFmpeg::decodeFFmpegThread() {
    pthread_mutex_lock(&init_mutex);
    avformat_network_init();
    pFormatCtx = avformat_alloc_context();
    if (!pFormatCtx) {
        LOGE("Could not allocate AVFormatContext");
        return;
    }

    pFormatCtx->interrupt_callback.callback = avformat_callback;
    pFormatCtx->interrupt_callback.opaque = this;

    if (avformat_open_input(&pFormatCtx, url, NULL, NULL) != 0) {
        if(LOG_DEBUG)
        {
            LOGE("can not open url :%s", url);
        }
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        if(LOG_DEBUG)
        {
            LOGE("can not find streams from %s", url);
        }
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }
    for (int i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (audio == NULL) {
                audio = new WlAudio(playstatus, pFormatCtx->streams[i]->codecpar->sample_rate,
                                    callJava);
                audio->streamIndex = i;
                audio->codecpar = pFormatCtx->streams[i]->codecpar;
                audio->duration = pFormatCtx->duration / AV_TIME_BASE;
                audio->time_base = pFormatCtx->streams[i]->time_base;

            }
        }
    }

    AVCodec *codec = const_cast<AVCodec *>(avcodec_find_decoder(audio->codecpar->codec_id));
    if (!codec) {
        if(LOG_DEBUG)
        {
            LOGE("can not find decoder");
        }
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    audio->avCodecContext = avcodec_alloc_context3(codec);
    if (!audio->avCodecContext) {
        if(LOG_DEBUG)
        {
            LOGE("can not alloc new decodecctx");
        }
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    if (avcodec_parameters_to_context(audio->avCodecContext, audio->codecpar) < 0) {
        if(LOG_DEBUG)
        {
            LOGE("can not fill decodecctx");
        }
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    if (avcodec_open2(audio->avCodecContext, codec, 0) != 0) {
        if(LOG_DEBUG)
        {
            LOGE("cant not open audio strames");
        }
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    if (callJava != NULL) {
        if (playstatus != NULL && !playstatus->exit) {
            callJava->onCallPrepared(CHILD_THREAD);
        } else {
            exit = true;
        }
    }
    pthread_mutex_unlock(&init_mutex);
}

void WlFFmpeg::start() {
    if (audio == NULL) {
        if(LOG_DEBUG)
        {
            LOGE("audio is null");
            return;
        }
        return;
    }
    audio->play();
    while (playstatus != NULL && !playstatus->exit) {
        AVPacket *avPacket = av_packet_alloc();
        if (av_read_frame(pFormatCtx, avPacket) == 0) {
            if (avPacket->stream_index == audio->streamIndex) {
                audio->queue->putAvpacket(avPacket);
            } else {
                av_packet_free(&avPacket);
                av_free(avPacket);
            }
        } else {
            av_packet_free(&avPacket);
            av_free(avPacket);
            while (playstatus != NULL && !playstatus->exit) {
                if (audio->queue->getQueueSize() > 0) {
                    continue;
                } else {
                    playstatus->exit = true;
                    break;
                }
            }
            break;
        }
    }
    exit = true;
    if(LOG_DEBUG)
    {
        LOGD("解码完成");
    }
}

void WlFFmpeg::pause() {
    if (audio != NULL) {
        audio->pause();
    }
}

void WlFFmpeg::resume() {
    if (audio != NULL) {
        audio->resume();
    }
}

void WlFFmpeg::release() {
    if(LOG_DEBUG)
    {
        LOGE("开始释放Ffmpe");
    }
    if (playstatus->exit) {
        return;
    }
    if(LOG_DEBUG)
    {
        LOGE("开始释放ffmpe");
    }
    playstatus->exit = true;

    pthread_mutex_lock(&init_mutex);
    int sleepCount = 0;
    while (!exit) {
        if (sleepCount > 1000) {
            exit = true;
        }
        if(LOG_DEBUG)
        {
            LOGE("wait ffmpeg  exit %d", sleepCount);
        }
        sleepCount++;
        av_usleep(1000 * 10);
    }
    if(LOG_DEBUG)
    {
        LOGE("释放 Audio");
    }

    if (audio != NULL) {
        audio->release();
        delete (audio);
        audio = NULL;
    }
    if(LOG_DEBUG)
    {
        LOGE("释放 封装格式上下文");
    }
    if (pFormatCtx != NULL) {
        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);
        pFormatCtx = NULL;
    }
    if(LOG_DEBUG)
    {
        LOGE("释放 callJava");
    }
    if (callJava != NULL) {
        callJava = NULL;
    }
    if(LOG_DEBUG)
    {
        LOGE("释放 playstatus");
    }
    if (playstatus != NULL) {
        playstatus = NULL;
    }

    pthread_mutex_unlock(&init_mutex);
}

WlFFmpeg::~WlFFmpeg() {
    pthread_mutex_destroy(&init_mutex);
}
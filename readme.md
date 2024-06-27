### 目标：实现Android播放器

已经实现基础的播放、暂停和停止功能

从音频播放到视频播放

完成音视频同步

```c++
int num = pFormatCtx->streams[i]->avg_frame_rate.num;
int den = pFormatCtx->streams[i]->avg_frame_rate.den;
if (num != 0 && den != 0) {
    int fps = num / den;
    video->defaultDelayTime = 1.0 / fps;
}
```

这段代码，avg_frame_rate是帧率，即每秒多少帧，fps即为求出的每秒帧数，那么每一帧的延迟时长即为fps的倒数。主要弄明白avg_frame_rate的含义就好了。



#### seek功能

现在视频暂停，播放，停止功能都已经实现，进度条也可以显示，但是seek有bug



知道怎么回事了。。。这是原来的写法，我个fool

```c++
avformat_seek_file(pFormatCtx, -1, INT64_MIN, rel, INT16_MAX, 0);
```

正确写法

```c++
avformat_seek_file(pFormatCtx, -1, INT64_MIN, rel, INT64_MAX, 0);
```

还在找什么地方没加锁，什么地方没释放锁，反复查看seek逻辑，结果发现是自己写错了一个单词。。。



### 代码执行流程



#### 开始

MainActivity.java

```java
public void begin(View view) {
        wlPlayer.prepared();
    }
```



Wlplayer.jaba

```java
new Thread(new Runnable() {
            @Override
            public void run() {
                n_prepared(source);
            }
        }).start();

private native void n_prepared(String source);
```



native-lib.cpp

```cpp
extern "C"
JNIEXPORT void JNICALL
Java_com_hgz_audioplayer_player_WlPlayer_n_1prepared(JNIEnv *env, jobject thiz, jstring source_) {
    const char *source = env->GetStringUTFChars(source_, 0);

    if (fFmpeg == NULL) {
        if (callJava == NULL) {
            callJava = new WlCallJava(javaVM, env, &thiz);
        }
        callJava->onCallLoad(MAIN_THREAD, true);
        playstatus = new WlPlaystatus();
        fFmpeg = new WlFFmpeg(playstatus, callJava, source);
        fFmpeg->prepared();
    }
}
```

prepared会初始化WlCallJava，并调用WlFFmpeg.cpp的prepared函数



WlCallJava.cpp 中的java方法签名

```cpp
jmid_prepared = env->GetMethodID(jlz, "onCallPrepared", "()V");
jmid_load = env->GetMethodID(jlz, "onCallLoad", "(Z)V");
jmid_timeinfo = env->GetMethodID(jlz, "onCallTimeInfo", "(II)V");
jmid_renderyuv = env->GetMethodID(jlz, "onCallRenderYUV", "(II[B[B[B)V");
```



WlFFmpeg.cpp

```cpp
void WlFFmpeg::prepared() {
    pthread_create(&decodeThread, NULL, decodeFFmpeg, this);
}

void *decodeFFmpeg(void *data) {
    WlFFmpeg *wlFFmpeg = (WlFFmpeg *) data;
    wlFFmpeg->decodeFFmpegThread();
    pthread_exit(&wlFFmpeg->decodeThread);
}
```

decodeFFmpegThread()中主要是读取流，把视频流和音频流分别写入队列中，初始化音视频时间基，并且计算defaultDelayTime，为以后得音视频同步和计算播放时间做准备。

getCodecContext()设置音视频解码器上下文

回调函数

```cpp
callJava->onCallPrepared(CHILD_THREAD);
```



WlPlayer.java

```java
public void onCallPrepared() {
        if (wlOnPreparedListener != null) {
            wlOnPreparedListener.onPrepared();
        }
    }
```



MainActivity.java

```java
wlPlayer.setWlOnPreparedListener(new WlOnPreparedListener() {
            @Override
            public void onPrepared() {
                Log.d("hgz", "准备好了，可以开始播放声音了");
                wlPlayer.start();
            }
        });
```



WlPlayer.java

```java
new Thread(new Runnable() {
            @Override
            public void run() {
                n_start();
            }
        }).start();
```



native-lib.cpp

```cpp
extern "C"
JNIEXPORT void JNICALL
Java_com_hgz_audioplayer_player_WlPlayer_n_1start(JNIEnv *env, jobject thiz) {
    if (fFmpeg != NULL) {
        pthread_create(&thread_start, NULL, startCallBack, fFmpeg);
    }
}

void *startCallBack(void *data) {
    WlFFmpeg *fFmpeg = (WlFFmpeg *) data;
    fFmpeg->start();
    pthread_exit(&thread_start);
}
```



WlFFmpeg.cpp

```cpp
audio->play();
video->play();
```

循环解码，取出AVPacket，转成AVFrame（还没有硬解码）



##### 音频播放逻辑

WlAudio.cpp

```cpp
void WlAudio::play() {
    pthread_create(&thread_play, NULL, decodePlay, this);
}

void *decodePlay(void *data) {
    WlAudio *wlAudio = (WlAudio *) data;
    wlAudio->initOpenSLES();
    pthread_exit(&wlAudio->thread_play);
}
```

初始化OpenSLES，播放音频

回调函数

```cpp
void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf, void *context) {
    WlAudio *wlAudio = (WlAudio *) context;
    if (wlAudio != NULL) {
        int buffersize = wlAudio->resampleAudio();
        if (buffersize > 0) {
            wlAudio->clock += buffersize / ((double) (wlAudio->sample_rate * 2 * 2));
            if (wlAudio->clock - wlAudio->last_time >= 0.1) {
                wlAudio->last_time = wlAudio->clock;
                wlAudio->callJava->onCallTimeInfo(CHILD_THREAD, wlAudio->clock, wlAudio->duration);
            }
            (*wlAudio->pcmBufferQueue)->Enqueue(wlAudio->pcmBufferQueue, (char *) wlAudio->buffer,
                                                buffersize);
        }
    }
}
```

循环解码，并且计算每次解码大小，从而计算播放时间

回调函数

```cpp
wlAudio->callJava->onCallTimeInfo(CHILD_THREAD, wlAudio->clock, wlAudio->duration);
```



WlCallJava.cpp

```cpp
void WlCallJava::onCallTimeInfo(int type, int curr, int total) {
    if (type == MAIN_THREAD) {
        jniEnv->CallVoidMethod(jobj, jmid_timeinfo, curr, total);
    } else if (type == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            if (LOG_DEBUG) {
                LOGE("call onCallTimeInfo wrong");
            }
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_timeinfo, curr, total);
        javaVM->DetachCurrentThread();
    }
}
```

更新播放时间



WlPlayer.java

```java
    public void onCallTimeInfo(int currentTime, int totalTime) {
        if (wlOnTimeInfoListener != null) {
            if (wlTimeInfoBean == null) {
                wlTimeInfoBean = new WlTimeInfoBean();
            }
            duration = totalTime;
            wlTimeInfoBean.setCurrentTime(currentTime);
            wlTimeInfoBean.setTotalTime(totalTime);
            wlOnTimeInfoListener.onTimeInfo(wlTimeInfoBean);
        }
    }
```



##### 视频播放逻辑

WlFFmpeg.java

```cpp
void WlVideo::play() {
    pthread_create(&thread_play, NULL, playVideo, this);
}
```

playVideo()中从队列中取出AVpacket

```cpp
avcodec_send_packet(video->avCodecContext, avPacket)
avcodec_receive_frame(video->avCodecContext, avFrame)
```

循环解码

调用getDelayTime()，将视频流同步到音频流

```cpp
double diff = video->getFrameDiffTime(avFrame);
LOGE("diff is %f", diff);
av_usleep(video->getDelayTime(diff) * 1000000);
```

```cpp
double WlVideo::getFrameDiffTime(AVFrame *avFrame) {

    double pts = avFrame->best_effort_timestamp;
    if (pts == AV_NOPTS_VALUE) {
        pts = 0;
    }
    pts *= av_q2d(time_base);

    if (pts > 0) {
        clock = pts;
    }

    double diff = audio->clock - clock;

    return diff;
}

double WlVideo::getDelayTime(double diff) {

    if (diff > 0.003) {
        delayTime = delayTime * 2 / 3;
        if (delayTime < defaultDelayTime / 2) {
            delayTime = defaultDelayTime / 2;
        } else if (delayTime > defaultDelayTime * 2) {
            delayTime = defaultDelayTime * 2;
        }
    } else if (diff < -0.003) {
        delayTime = delayTime * 3 / 2;
        if (delayTime > defaultDelayTime * 2) {
            delayTime = defaultDelayTime * 2;
        } else if (delayTime < defaultDelayTime / 2) {
            delayTime = defaultDelayTime / 2;
        }
    }

    if (diff >= 0.5) {
        delayTime = defaultDelayTime / 4;
    } else if (diff <= -0.5) {
        delayTime = defaultDelayTime * 4;
    }

    return delayTime;
}
```





并且判断是否是yuv420p格式，如果不是，对视频进行重采样

```cpp
SwsContext *sws_ctx = sws_getContext(
                    video->avCodecContext->width,
                    video->avCodecContext->height,
                    video->avCodecContext->pix_fmt,
                    video->avCodecContext->width,
                    video->avCodecContext->height,
                    AV_PIX_FMT_YUV420P,
                    SWS_BICUBIC, NULL, NULL, NULL);
```



渲染视频

```cpp
video->wlCallJava->onCallRenderYUV
```

调用WlCallJava.cpp

```cpp
void WlCallJava::onCallRenderYUV(int width, int height, uint8_t *fy, uint8_t *fu, uint8_t *fv) {

    JNIEnv *jniEnv;
    if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
        if (LOG_DEBUG) {
            LOGE("call onCallComplete worng");
        }
        return;
    }

    jbyteArray y = jniEnv->NewByteArray(width * height);
    jniEnv->SetByteArrayRegion(y, 0, width * height, reinterpret_cast<const jbyte *>(fy));

    jbyteArray u = jniEnv->NewByteArray(width * height / 4);
    jniEnv->SetByteArrayRegion(u, 0, width * height / 4, reinterpret_cast<const jbyte *>(fu));

    jbyteArray v = jniEnv->NewByteArray(width * height / 4);
    jniEnv->SetByteArrayRegion(v, 0, width * height / 4, reinterpret_cast<const jbyte *>(fv));

    jniEnv->CallVoidMethod(jobj, jmid_renderyuv, width, height, y, u, v);

    jniEnv->DeleteLocalRef(y);
    jniEnv->DeleteLocalRef(u);
    jniEnv->DeleteLocalRef(v);

    javaVM->DetachCurrentThread();
}
```

WlPlayer.java

```java
wlGLSurfaceView.setYUVData(width, height, y, u, v);
```

播放视频


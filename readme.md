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


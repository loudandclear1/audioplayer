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


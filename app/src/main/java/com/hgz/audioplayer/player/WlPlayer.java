package com.hgz.audioplayer.player;

import android.text.TextUtils;
import android.util.Log;

import com.hgz.audioplayer.listener.WlOnLoadListener;
import com.hgz.audioplayer.listener.WlOnPauseResumeListener;
import com.hgz.audioplayer.listener.WlOnPreparedListener;
import com.hgz.audioplayer.listener.WlOnTimeInfoListener;
import com.hgz.audioplayer.WlTimeInfoBean;

public class WlPlayer {

    static {
        System.loadLibrary("audioplayer");
    }

    private String source;//数据源
    private static WlTimeInfoBean wlTimeInfoBean;
    private WlOnPreparedListener wlOnPreparedListener;
    private WlOnLoadListener wlOnLoadListener;
    private WlOnPauseResumeListener wlOnPauseResumeListener;
    private WlOnTimeInfoListener wlOnTimeInfoListener;


    public WlPlayer() {
    }

    /**
     * 设置数据源
     *
     * @param source
     */
    public void setSource(String source) {
        this.source = source;
    }

    /**
     * 设置准备接口回调
     *
     * @param wlOnPreparedListener
     */
    public void setWlOnPreparedListener(WlOnPreparedListener wlOnPreparedListener) {
        this.wlOnPreparedListener = wlOnPreparedListener;
    }

    public void setWlOnLoadListener(WlOnLoadListener wlOnLoadListener) {
        this.wlOnLoadListener = wlOnLoadListener;
    }

    public void setWlOnPauseResumeListener(WlOnPauseResumeListener wlOnPauseResumeListener) {
        this.wlOnPauseResumeListener = wlOnPauseResumeListener;
    }

    public void setWlOnTimeInfoListener(WlOnTimeInfoListener wlOnTimeInfoListener) {
        this.wlOnTimeInfoListener = wlOnTimeInfoListener;
    }

    public void prepared() {
        if (TextUtils.isEmpty(source)) {
            Log.d("hgz", "source is not empty");
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_prepared(source);
            }
        }).start();

    }

    public void start() {
        if (TextUtils.isEmpty(source)) {
            Log.d("hgz", "source is empty");
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_start();
            }
        }).start();
    }

    public void pause() {
        n_pause();
        if (wlOnPauseResumeListener != null) {
            wlOnPauseResumeListener.onPause(true);
        }
    }

    public void resume() {
        n_resume();
        if (wlOnPauseResumeListener != null) {
            wlOnPauseResumeListener.onPause(false);
        }
    }

    public void stop() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_stop();
            }
        }).start();
    }


    /**
     * c++回调java的方法
     */
    public void onCallPrepared() {
        if (wlOnPreparedListener != null) {
            wlOnPreparedListener.onPrepared();
        }
    }

    public void onCallLoad(boolean load) {
        if (wlOnLoadListener != null) {
            wlOnLoadListener.onLoad(load);
        }
    }

    public void onCallTimeInfo(int currentTime, int totalTime) {
        if (wlOnTimeInfoListener != null) {
            if (wlTimeInfoBean == null) {
                wlTimeInfoBean = new WlTimeInfoBean();
            }
            wlTimeInfoBean.setCurrentTime(currentTime);
            wlTimeInfoBean.setTotalTime(totalTime);
            wlOnTimeInfoListener.onTimeInfo(wlTimeInfoBean);
        }
    }


    private native void n_prepared(String source);

    private native void n_start();

    private native void n_pause();

    private native void n_resume();

    private native void n_stop();


}

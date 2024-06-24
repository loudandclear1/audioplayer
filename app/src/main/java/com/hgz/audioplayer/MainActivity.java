package com.hgz.audioplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.annotation.SuppressLint;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;

import com.hgz.audioplayer.listener.WlOnLoadListener;
import com.hgz.audioplayer.listener.WlOnPauseResumeListener;
import com.hgz.audioplayer.listener.WlOnPreparedListener;
import com.hgz.audioplayer.listener.WlOnTimeInfoListener;
import com.hgz.audioplayer.opengl.WlGLSurfaceView;
import com.hgz.audioplayer.player.WlPlayer;
import com.hgz.audioplayer.util.WlTimeUtil;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'audioplayer' library on application startup.
    static {
        System.loadLibrary("audioplayer");
    }

    private WlPlayer wlPlayer;
    private TextView tvTime;

    private WlGLSurfaceView wlGLSurfaceView;

    private SeekBar seekBar;

    private int position;
    private boolean seek = false;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        tvTime = findViewById(R.id.tv_time);
        wlGLSurfaceView = findViewById(R.id.wlglsurfaceview);
        seekBar = findViewById(R.id.seekbar);
        wlPlayer = new WlPlayer();
        wlPlayer.setWlGLSurfaceView(wlGLSurfaceView);

        wlPlayer.setWlOnPreparedListener(new WlOnPreparedListener() {
            @Override
            public void onPrepared() {
                Log.d("hgz", "准备好了，可以开始播放声音了");
                wlPlayer.start();
            }
        });
        wlPlayer.setWlOnLoadListener(new WlOnLoadListener() {
            @Override
            public void onLoad(boolean load) {
                Log.d("hgz", "加载中");
            }
        });

        wlPlayer.setWlOnPauseResumeListener(new WlOnPauseResumeListener() {
            @Override
            public void onPause(boolean pause) {
                if (pause) {
                    Log.d("hgz", "暂停中...");
                } else {
                    Log.d("hgz", "播放中...");
                }
            }
        });

        wlPlayer.setWlOnTimeInfoListener(new WlOnTimeInfoListener() {
            @Override
            public void onTimeInfo(WlTimeInfoBean timeInfoBean) {
                Message message = Message.obtain();
                message.what = 1;
                message.obj = timeInfoBean;
                handler.sendMessage(message);
            }
        });

        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                position = progress * wlPlayer.getDuration() / 100;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                seek = true;
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                wlPlayer.seek(position);
                seek = false;
            }
        });

    }

    @SuppressLint("SdCardPath")
    public void begin(View view) {
        wlPlayer.setSource("/sdcard/1.mp4");
        wlPlayer.prepared();
    }

    public void pause(View view) {
        wlPlayer.pause();
    }

    public void resume(View view) {
        wlPlayer.resume();
    }

    @SuppressLint("HandlerLeak")
    Handler handler = new Handler() {
        @SuppressLint("SetTextI18n")
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            if (msg.what == 1) {
                WlTimeInfoBean wlTimeInfoBean = (WlTimeInfoBean) msg.obj;
                tvTime.setText(WlTimeUtil.secdsToDateFormat(wlTimeInfoBean.getTotalTime(), wlTimeInfoBean.getTotalTime())
                        + "/" + WlTimeUtil.secdsToDateFormat(wlTimeInfoBean.getCurrentTime(), wlTimeInfoBean.getTotalTime()));


                if (!seek && wlTimeInfoBean.getTotalTime() > 0) {
                    seekBar.setProgress(wlTimeInfoBean.getCurrentTime() * 100 / wlTimeInfoBean.getTotalTime());
                }
            }
        }
    };

    public void stop(View view) {
        wlPlayer.stop();
    }
}
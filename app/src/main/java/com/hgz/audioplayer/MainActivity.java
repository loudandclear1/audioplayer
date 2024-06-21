package com.hgz.audioplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.widget.TextView;

import com.hgz.audioplayer.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'audioplayer' library on application startup.
    static {
        System.loadLibrary("audioplayer");
    }

    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

    }

    /**
     * A native method that is implemented by the 'audioplayer' native library,
     * which is packaged with this application.
     */
}
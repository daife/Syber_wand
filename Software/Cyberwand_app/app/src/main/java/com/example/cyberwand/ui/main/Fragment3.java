package com.example.cyberwand.ui.main;

import android.os.Bundle;
import android.os.Handler;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;

import com.example.cyberwand.MainActivity;
import com.example.cyberwand.R;

public class Fragment3 extends Fragment {
    private TextView textView;
    private Handler handler;
    private Runnable updateRunnable;

    public static Fragment3 newInstance() {
        return new Fragment3();
    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.fragment3, container, false);
        textView = view.findViewById(R.id.textView);
        return view;
    }

    @Override
    public void onResume() {
        super.onResume();
        // 初始化Handler和Runnable
        handler = new Handler();
        updateRunnable = new Runnable() {
            @Override
            public void run() {
                if (textView != null) {
                    textView.setText(((MainActivity) getActivity()).getSharedString());
                }
                // 每隔一段时间执行一次，这里设置为500毫秒
                handler.postDelayed(this, 500);
            }
        };
        // 启动循环
        handler.post(updateRunnable);
    }

    private void updateTextView() {
        // 由于已经在onResume中启动了循环，这里不再需要
    }

    @Override
    public void onPause() {
        super.onPause();
        // 停止循环
        if (handler != null) {
            handler.removeCallbacks(updateRunnable);
        }
    }
}
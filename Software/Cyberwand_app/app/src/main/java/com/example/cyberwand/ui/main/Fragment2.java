package com.example.cyberwand.ui.main;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;

import com.example.cyberwand.MainActivity;
import com.example.cyberwand.R;

public class Fragment2 extends Fragment {
    public static Fragment2 newInstance() {
        return new Fragment2();
    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.fragment2, container, false);

        // Initialize buttons and set their onClick listeners
        for (int i = 0; i < 14; i++) {
            int buttonId = getResources().getIdentifier("button" + (i + 1), "id", getContext().getPackageName());
            Button button = view.findViewById(buttonId);
            button.setOnClickListener(new View.OnClickListener() {
                @Override
        public void onClick(View v) {
            // 获取按钮的文本
            String buttonText = ((Button) v).getText().toString();
                        int code=255;
                        switch(buttonText){
                            case "玛卡巴卡":
                            code=0x00;
                            break;
                            case "库尔塔丝":
                            code=0x01;
                            break;
                            case "洛克莫特":
                            code=0x02;
                            break;
                            case "基尼太美":
                            code=0x03;
                            break;
                            case "速速退下":
                            code=0x04;
                            break;
                            case "ON":
                            code=0x05;
                            break;
                            case "OFF":
                            code=0x06;
                            break;
                            case "+":
                            code=0x07;
                            break;
                            case "-":
                            code=0x08;
                            break;
                            case "上下扫风":
                            code=0x09;
                            break;
                            case "模式":
                            code=0x0A;
                            break;
                            case "风速":
                            code=0x0B;
                            break;
                            case "密斯卡莫斯卡":
                            code=0x0C;
                            break;
                            case "软复位":
                            code=0x0E;
                            break;
                        }
                         ((MainActivity) getActivity()).sendData(code);
            // 将按钮的文本传递给updateString方法
            ((MainActivity) getActivity()).updateString(buttonText+" clicked", true);
        }
    });
        }

        return view;
    }
}
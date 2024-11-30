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

public class Fragment1 extends Fragment {
    public static Fragment1 newInstance() {
        return new Fragment1();
    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.fragment1, container, false);

        // Initialize buttons and set their onClick listeners
        for (int i = 0; i < 25; i++) {
    int buttonId = getResources().getIdentifier("button" + (i + 1), "id", getContext().getPackageName());
    Button button = view.findViewById(buttonId);
    button.setOnClickListener(new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            // 获取按钮的文本
            String buttonText = ((Button) v).getText().toString();
                        int code=255;//啥都没有
                        switch(buttonText){
                            case "aux1":
                            code=101;
                            break;
                            case "aux2":
                            code=102;
                            break;
                            case "aux3":
                            code=103;
                            break;
                            case "aux4":
                            code=104;
                            break;
                            case "None":
                            code=105;
                            break;
                            case "gree1":
                            code=111;
                            break;
                            case "gree2":
                            code=112;
                            break;
                            case "gree3":
                            code=113;
                            break;
                            case "gree4":
                            code=114;
                            break;
                            case "gree5":
                            code=115;
                            break;
                            case "haier1":
                            code=121;
                            break;
                            case "haier2":
                            code=122;
                            break;
                            case "haier3":
                            code=123;
                            break;
                            case "haier4":
                            code=124;
                            break;
                            case "haier5":
                            code=125;
                            break;
                            case "meidi1":
                            code=131;
                            break;
                            case "meidi2":
                            code=132;
                            break;
                            case "meidi3":
                            code=133;
                            break;
                            case "meidi4":
                            code=134;
                            break;
                            case "meidi5":
                            code=135;
                            break;
                            case "haixin1":
                            code=141;
                            break;
                            case "haixin2":
                            code=142;
                            break;
                            case "haixin3":
                            code=143;
                            break;
                            case "haixin4":
                            code=144;
                            break;
                            case "haixin5":
                            code=145;
                            break;
                        }
                        if(code!=105||code!=255){
                         ((MainActivity) getActivity()).sendData(code);}
                        
            // 将按钮的文本传递给updateString方法
            ((MainActivity) getActivity()).updateString(buttonText+" clicked", true);
        }
    });
}

        return view;
    }
}
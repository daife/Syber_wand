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
        for (int i = 0; i < 5; i++) {
            int buttonId = getResources().getIdentifier("button" + (i + 1), "id", getContext().getPackageName());
            Button button = view.findViewById(buttonId);
            button.setOnClickListener(new View.OnClickListener() {
                @Override
        public void onClick(View v) {
            // 获取按钮的文本
            String buttonText = ((Button) v).getText().toString();
            // 将按钮的文本传递给updateString方法
            ((MainActivity) getActivity()).updateString(buttonText+" clicked", true);
        }
    });
        }

        return view;
    }
}
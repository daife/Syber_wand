package com.example.cyberwand;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.viewpager.widget.ViewPager;
import androidx.appcompat.widget.Toolbar;

import com.google.android.material.snackbar.Snackbar;
import com.example.cyberwand.databinding.ActivityMainBinding;
import com.example.cyberwand.ui.main.SectionsPagerAdapter;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.UUID;

public class MainActivity extends AppCompatActivity {
    private ActivityMainBinding binding;
    private String sharedString = "初始化Ok了";
    private static int staticStatus = 1;

    private static final UUID MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB"); // 通用UUID
    private static final int REQUEST_ENABLE_BT = 1;
    private static final int REQUEST_PERMISSION_BT = 2;
    private BluetoothAdapter bluetoothAdapter = null;
    private BluetoothSocket bluetoothSocket = null;
    private OutputStream outputStream = null;
    private void startBluetoothListener() {
    new Thread(new Runnable() {
    @Override
    public void run() {
        try {
            InputStream inputStream = bluetoothSocket.getInputStream();
            byte[] buffer = new byte[1024];
            int bytesRead;
            while ((bytesRead = inputStream.read(buffer)) != -1) {
                // 将接收到的字节转换为字符串
                String receivedData = new String(buffer, 0, bytesRead);
                // 调用getReceivedData函数处理数据
               // String response = getReceivedData(receivedData);
                // 更新UI或其他操作
                updateString(receivedData, false);
            }
        } catch (IOException e) {
            e.printStackTrace();
            updateString("Error reading from Bluetooth", false);
        }
    }
}).start();
}

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
        
        setSupportActionBar(binding.toolbar);

        SectionsPagerAdapter sectionsPagerAdapter = new SectionsPagerAdapter(this, getSupportFragmentManager());
        binding.viewPager.setAdapter(sectionsPagerAdapter);
        binding.tabs.setupWithViewPager(binding.viewPager);

//        binding.fab.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG)
//                        .setAction("Action", null).show();
//            }
//        });

        // 初始化蓝牙适配器
        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (bluetoothAdapter == null) {
            Toast.makeText(this, "Bluetooth is not available", Toast.LENGTH_SHORT).show();
            finish();
        }

        // 检查蓝牙权限
        checkBluetoothPermissions();
    }

    private void checkBluetoothPermissions() {
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH) != PackageManager.PERMISSION_GRANTED ||
            ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_ADMIN) != PackageManager.PERMISSION_GRANTED ||
            ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this,
                    new String[]{
                            Manifest.permission.BLUETOOTH,
                            Manifest.permission.BLUETOOTH_ADMIN,
                            Manifest.permission.ACCESS_FINE_LOCATION
                    },
                    REQUEST_PERMISSION_BT);
        } else {
            checkBluetoothEnabled();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_PERMISSION_BT) {
            boolean allPermissionsGranted = true;
            for (int result : grantResults) {
                if (result != PackageManager.PERMISSION_GRANTED) {
                    allPermissionsGranted = false;
                    break;
                }
            }
            if (allPermissionsGranted) {
                checkBluetoothEnabled();
            } else {
                Toast.makeText(this, "Bluetooth permissions not granted", Toast.LENGTH_SHORT).show();
            }
        }
    }

    private void checkBluetoothEnabled() {
        if (!bluetoothAdapter.isEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
        } else {
            connectToDevice("00:23:00:01:65:17"); // 替换为单片机MAC地址
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQUEST_ENABLE_BT) {
            if (resultCode == RESULT_OK) {
                connectToDevice("00:23:00:01:65:17"); // 替换为您的设备MAC地址
            } else {
                Toast.makeText(this, "Bluetooth not enabled", Toast.LENGTH_SHORT).show();
            }
        }
    }

    private void connectToDevice(String deviceAddress) {
    BluetoothDevice device = bluetoothAdapter.getRemoteDevice(deviceAddress);
    try {
        bluetoothSocket = device.createRfcommSocketToServiceRecord(MY_UUID);
        bluetoothSocket.connect();
        outputStream = bluetoothSocket.getOutputStream();
        updateString("Connected to device: " + device.getName(), false);
        // 启动蓝牙监听器
        startBluetoothListener();
    } catch (IOException e) {
        e.printStackTrace();
        updateString("Could not connect to device", false);
    }
}
    public String getReceivedData(int receivedData) {
    switch (receivedData) {
        case 1:
            return "Data 1 received";
        case 2:
            return "Data 2 received";
        case 3:
            return "Data 3 received";
        // 从单片机收到整数数据，转化为打印输出
        default:
            return "Unknown data received";
    }
}
    
    public String getSendedData(int sendedData) {
    switch (sendedData) {
        case 1:
            return "Data 1 sended";
        case 2:
            return "Data 2 received";
        case 3:
            return "Data 3 received";
        // 将发送数据转化为打印输出
        default:
            return "Unknown data received";
    }
}

    public void sendData(int data) {
        if (outputStream != null) {
            try {
            	outputStream.write(data);
                updateString(getSendedData(data),true);
            } catch(Exception err) {
            	
            }
                
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        try {
            if (bluetoothSocket != null) {
                bluetoothSocket.close();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void updateString(String newString, boolean prependWithLessThan) {
    // 定义最大行数
    final int MAX_LINES = 500;
    // 计算当前sharedString的行数
    int currentLines = sharedString.split("\n").length;

    // 如果行数达到最大值，则删除第一行
    if (currentLines >= MAX_LINES) {
        sharedString = sharedString.replaceFirst("^.*\n", "");
    }

    // 在新字符串前加上'<'或'>'
    String prefix = prependWithLessThan ? "<" : ">";
    String updatedString = sharedString + "\n" + prefix + newString;

    // 更新sharedString
    sharedString = updatedString;
}

    public String getSharedString() {
        return sharedString;
    }
}
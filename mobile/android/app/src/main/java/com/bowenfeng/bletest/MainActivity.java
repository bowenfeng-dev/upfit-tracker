package com.bowenfeng.bletest;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.widget.Button;
import android.widget.TextView;

import java.util.UUID;

public class MainActivity extends Activity {
    private static final int REQUEST_ENABLE_BT = 10101;

    private TextView textView;
    private TextView statusView;

    private int number = 0;
    private long lastPressure = 0;

    private boolean isStarted = false;
    private boolean isUpwards = false;

    private static UUID SERVICE_ID = UUID.fromString("713d0000-503e-4c75-ba94-3148f18d941e");
    private static UUID CHAR_ID = UUID.fromString("713d0002-503e-4c75-ba94-3148f18d941e");
    private static UUID DESC_ID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");

    private BluetoothGattCharacteristic mBleChar;

    private BluetoothAdapter mBluetoothAdapter;
    private BluetoothAdapter.LeScanCallback mLeScanCallback = (device, rssi, scanRecord) -> {
        System.out.println(device.toString() + ":" + device.getName() + ":" + rssi);
        onDeviceDiscovered(device, rssi);
    };
    private BluetoothGatt mBluetoothGatt;
    private final BluetoothGattCallback mGattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            System.out.println("onConnectionStateChange:" + newState);
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                statusView.setText(R.string.sensor_status_connected);
                mBluetoothGatt.discoverServices();
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                statusView.setText(R.string.sensor_status_disconnected);
                scanBleDeivce();
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            initServiceAndCharacteristic();
            enableGattNotification();
        }

        @Override
        public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            System.out.println("onCharacteristicRead:" + characteristic);
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            if (!isStarted) {
                return;
            }
            long pressure = getPressure(characteristic.getValue());
            long diff = pressure - lastPressure;
            lastPressure = pressure;
            if (Math.abs(diff) > 4) {
                // not a push up.
            } else if (Math.abs(diff) >= 1) {
                // There is some vertical movement
                if (diff > 0) {
                    isUpwards = false;
                } else if (!isUpwards) {
                    isUpwards = true;
                    ++number;
                    textView.setText(Integer.toString(number));
                }
            }
        }
    };

    private long getPressure(byte[] value) {
        return (((value[3] & 0xFF) * 256L + (value[2] & 0xFF)) * 256L + (value[1] & 0xFF)) * 256L + (value[0] & 0xFF);
    }

    private void initServiceAndCharacteristic() {
        BluetoothGattService mBleService = mBluetoothGatt.getService(SERVICE_ID);
        mBleChar = mBleService.getCharacteristic(CHAR_ID);
    }

    private void enableGattNotification() {
        mBluetoothGatt.setCharacteristicNotification(mBleChar, true);
        BluetoothGattDescriptor descriptor = mBleChar.getDescriptor(DESC_ID);
        descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
        mBluetoothGatt.writeDescriptor(descriptor);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        textView = findViewById(R.id.textView);
        statusView = findViewById(R.id.statusView);

        final Button button = findViewById(R.id.button);
        button.setOnClickListener(v -> {
            isStarted = !isStarted;
            button.setText(isStarted ? "Stop" : "Start");
            number = isStarted ? 0 : number;
            textView.setText(Integer.toString(number));
        });

        initBluetooth();
        scanBleDeivce();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
    }

    private void initBluetooth() {
        final BluetoothManager bluetoothManager =
                (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        mBluetoothAdapter = bluetoothManager.getAdapter();

        if (mBluetoothAdapter == null || !mBluetoothAdapter.isEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
        }
    }

    private void scanBleDeivce() {
        mBluetoothAdapter.startLeScan(mLeScanCallback);
    }

    private void onDeviceDiscovered(BluetoothDevice device, int rssi) {
        if ("DPS310_NANO2".equals(device.getName())) {
            System.out.println("Found device. Stop LE Scan.");
            mBluetoothAdapter.stopLeScan(mLeScanCallback);
            connectDevice(device);
        }
    }

    private void connectDevice(BluetoothDevice device) {
        mBluetoothGatt = device.connectGatt(this, false, mGattCallback);
    }
}

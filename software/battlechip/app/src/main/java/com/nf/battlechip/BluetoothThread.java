package com.nf.battlechip;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Optional;
import java.util.Set;
import java.util.UUID;

public class BluetoothThread {

    private static final String BLUETOOTH_DEBUG = "Bluetooth";
    private static BluetoothThread instance = null;
    private static Thread readingThread = null;

    private static final Set<String> CHIP_ONE_MAC_IDS = new HashSet<>(Arrays.asList("20:18:11:21:24:72", "B8:9A:2A:30:2B:35")); // TODO: rely only on device name?
    private static final Set<String> CHIP_ONE_DEVICE_NAMES = new HashSet<>(Arrays.asList("hc01.com HC-05"));
    private static final Set<String> CHIP_TWO_MAC_IDS = new HashSet<>(); // TODO: fill
    private static final Set<String> CHIP_TWO_DEVICE_NAMES = new HashSet<>(); // TODO: fill
    private static final UUID SERVICE_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    private BluetoothSocket bluetoothSocket;
    private InputStream inputStream;
    private OutputStream outputStream;
    private int chipId;

    BluetoothThread(int chipId) {
        BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
        // List all devices
        for (BluetoothDevice device : adapter.getBondedDevices()) {
            Log.d(BLUETOOTH_DEBUG, device.getAddress() + " " + device.getName());
        }
        Optional<BluetoothDevice> device = adapter.getBondedDevices().stream()
                .filter(chipId == 1 ? BluetoothThread::isChipOneDevice : BluetoothThread::isChipTwoDevice).findAny();
        bluetoothSocket = null;
        inputStream = null;
        outputStream = null;

        // create socket
        try {
            bluetoothSocket = device.isPresent() ? device.get().createRfcommSocketToServiceRecord(SERVICE_UUID)
                    : null;
            Log.d(BLUETOOTH_DEBUG, device.isPresent() ? "Connected " + bluetoothSocket.toString() : "Failed");
        } catch (IOException e) {
            Log.d(BLUETOOTH_DEBUG, "Socket creation failed\n" + e.toString());
        }

        // connect socket
        if (bluetoothSocket != null) {
            try {
                bluetoothSocket.connect();
                // get streams
                try {
                    inputStream = bluetoothSocket.getInputStream();
                    outputStream = bluetoothSocket.getOutputStream();
                } catch (IOException e) {
                    Log.d(BLUETOOTH_DEBUG, "Exception getting streams\n" + e.toString());
                }
            } catch (IOException exception) {
                close();
            }

        }
    }

    private static boolean isChipOneDevice(BluetoothDevice device) {
        return CHIP_ONE_MAC_IDS.contains(device.getAddress())
                || CHIP_ONE_DEVICE_NAMES.contains(device.getName());
    }

    private static boolean isChipTwoDevice(BluetoothDevice device) {
        return CHIP_TWO_MAC_IDS.contains(device.getAddress())
                || CHIP_TWO_DEVICE_NAMES.contains(device.getName());
    }

    public static void createInstance(int chipId) {
        if (instance != null) {
            instance.close();
        }
        instance = new BluetoothThread(chipId);
    }

    public static BluetoothThread getInstance() {
        if (instance != null && !instance.isValidThread()) {
            instance = new BluetoothThread(instance.chipId);
        }
        return instance;
    }

    public void startReading() {
        if (readingThread == null) {
            if (bluetoothSocket != null && bluetoothSocket.isConnected()) {
                readingThread = new Thread(this::read);
                readingThread.start();
            } else {
                Log.d(BLUETOOTH_DEBUG, "Can't read, socket not connected");
            }
        }
    }

    public void read() {
        Log.d(BLUETOOTH_DEBUG, "Starting thread");
        try {
            byte[] readBuffer = new byte[1024];
            StringBuilder builder = new StringBuilder();

            while (inputStream != null) {
                int bytesRead = inputStream.read(readBuffer); // possible to read portions of the message at a time
                String readCharacters = new String(readBuffer).substring(0, bytesRead);
                builder.append(readCharacters);
                Log.d(BLUETOOTH_DEBUG, "Read: " + readCharacters);

                // Only send to Bluetooth if entire message has been read
                if (builder.lastIndexOf("~") == builder.length() - 1) {
                    UnityMessage.processBluetoothMessage(builder.toString());
                    builder = new StringBuilder();
                }
            }
        } catch (IOException e) {
            close();
            readingThread = null;
            Log.d(BLUETOOTH_DEBUG, "Read failed, exiting\n" + e.toString());
        }
    }

    public boolean isValidThread() {
        return bluetoothSocket != null && inputStream != null && outputStream != null;
    }

    public void write(byte[] bytes) {
        if (!isValidThread()) {
            Log.d(BLUETOOTH_DEBUG, "Can't write, invalid thread");
        } else {
            try {
                Log.d(BLUETOOTH_DEBUG, "Write: " + new String(bytes));
                outputStream.write(bytes);
            } catch (IOException e) {
                close();
                Log.d(BLUETOOTH_DEBUG, "Write failed\n" + e.toString());
            }
        }
    }

    public void close() {
        if (bluetoothSocket != null) {
            try {
                bluetoothSocket.close();
            } catch (IOException e) {
                Log.d(BLUETOOTH_DEBUG, "Failed to close socket\n" + e.toString());
            } finally {
                instance = null;
                inputStream = null;
                outputStream = null;
                bluetoothSocket = null;
            }
        }
    }
}


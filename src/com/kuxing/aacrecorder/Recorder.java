package com.kuxing.aacrecorder;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder.AudioSource;

public class Recorder {
	// Constructor
    public Recorder() {
        int err = this.initNative();
        if (err != 0) {
            throw new RuntimeException(String.format("Can not initialize recoder: %d", err));
        }
    }
    // Release
    @Override
    protected void finalize() throws Throwable {
        release();
        super.finalize();
    }
    // Interfaces
    public int start(String url) {
    	return startNative(url);
    }
    public void stop() {
    	stopNative();
    }
    public void release() {
        stop();
        releaseNative();
    }
    public int getVolumn() {
    	return getVolumnNative();
    }
    
    // JNI functions
    static private AudioRecord createAudioTrack(int sampleRateInHz,
            int numberOfChannels) {
        // 获得缓冲区字节大小  
        int bufferSizeInBytes = AudioRecord.getMinBufferSize(sampleRateInHz,  
        		numberOfChannels, AudioFormat.ENCODING_PCM_16BIT);  
         
        // 创建AudioRecord对象  
        AudioRecord audioRecord = new AudioRecord(AudioSource.MIC, sampleRateInHz,  
        		numberOfChannels, AudioFormat.ENCODING_PCM_16BIT, bufferSizeInBytes); 
        return audioRecord;

    }
    // Native functions
    private native int initNative();

    private native void releaseNative();

    private native int startNative(String url);

    private native void stopNative();

    private native int getVolumnNative();
    
    // Native members
    private int mNativeRecoder;
    
    // Load library
    static {
		System.loadLibrary("aacrecorder");
	}
}

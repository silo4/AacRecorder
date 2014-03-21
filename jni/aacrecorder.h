/*
 * aacrecorder.h
 *
 *  Created on: 2014-1-23
 *      Author: peihao
 */

#ifndef AACRECORDER_H_
#define AACRECORDER_H_

#include <stdio.h>

#include "javamethod.h"
#include "aacplus.h"


static char *recorder_class_path_name = "com/kuxing/aacrecorder/Recorder";
static JavaField	recorder_m_native_recorder = {"mNativeRecorder", "I"};
static JavaMethod	recorder_create_audio_track = {"createAudioTrack", "(II)Landroid/media/AudioRecord;"};

// AudioRecord
static char *android_record_class_path_name = "android/media/AudioRecord";
static JavaMethod audio_record_read = {"read", "([BII)I"};
static JavaMethod audio_record_start = {"startRecording", "()V"};
static JavaMethod audio_record_stop = {"stop", "()V"};
static JavaMethod audio_record_release = {"release", "()V"};

struct Recorder {
};


enum RecorderState {
	RECORDER_STATE_IDLE = 0,
	RECORDER_STATE_RECORDING,
	RECORDER_STATE_TO_RELEASE,
};

struct RecorderContext {
	// Context
	JavaVM          *javavm;

	// Encoder
	aacplusEncHandle hEncoder;

	// Record
	jclass          audioRecordClass;
	jobject			audioRecord;

	// Audio functions
	jmethodID audio_record_read_method;
	jmethodID audio_record_start_method;
	jmethodID audio_record_stop_method;
	jmethodID audio_record_release_method;

	// Recorder functions
	jmethodID recorder_create_audio_record_method;

	// Main thread
	pthread_t mainThread;

	// Audio class
	jclass         recorderClass;
	jobject        recorderObject;

	// State
	int             state;
	pthread_mutex_t stateLock;

	// Member
	char			   *outputFile;
	unsigned long		inputSamples;
	unsigned long		maxOutputBytes;
	int32_t				bitrate;
	uint8_t			   *outputBuffer;
	int32_t			   *inputPcm;
	int32_t				inputPcmOffset;
	uint32_t			channels;
	uint32_t			sampleRate;
	uint32_t			blockSize;
	int32_t				volumn;
};

#endif /* AACRECORDER_H_ */

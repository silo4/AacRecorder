/*
 * Player.java
 * Copyright (c) 2013 ZhangPeihao@Kuxing
 */
#include <jni.h>
#include <pthread.h>

#include "log.h"
#include "com_kuxing_aacrecorder_Recorder.h"
#include "aacrecorder.h"
#include "javamethod.h"

void* mainLoop(void*);

char* jstringTostring(JNIEnv* env, jstring jstr)
{
	char* rtn = NULL;
	jclass clsstring = (*env)->FindClass(env, "java/lang/String");
	jstring strencode = (*env)->NewStringUTF(env, "utf-8");
	jmethodID mid = (*env)->GetMethodID(env, clsstring, "getBytes", "(Ljava/lang/String;)[B");
	jbyteArray barr= (jbyteArray)(*env)->CallObjectMethod(env, jstr, mid, strencode);
	jsize alen = (*env)->GetArrayLength(env, barr);
	jbyte* ba = (*env)->GetByteArrayElements(env, barr, JNI_FALSE);
	if (alen > 0)
	{
		rtn = (char*)malloc(alen + 1);
		memcpy(rtn, ba, alen);
		rtn[alen] = 0;
	}
	(*env)->ReleaseByteArrayElements(env, barr, ba, 0);
	return rtn;
}

struct RecorderContext * get_context(JNIEnv *env, jobject thiz) {
	jfieldID m_native_layer_field = java_get_field(env, recorder_class_path_name,
			recorder_m_native_recorder);
	struct RecorderContext * recorderContext = (struct RecorderContext *) (*env)->GetIntField(env, thiz,
			m_native_layer_field);
	return recorderContext;
}

int set_context(JNIEnv *env, jobject thiz, struct RecorderContext *recorderContext) {
	if (recorderContext->recorderClass == NULL) {
		return -1;
	}

	jfieldID recorder_m_native_recorder_field = java_get_field(env,
			recorder_class_path_name, recorder_m_native_recorder);
	if (recorder_m_native_recorder_field == NULL) {
		return -1;
	}

	(*env)->SetIntField(env, thiz, recorder_m_native_recorder_field,
				(jint) recorderContext);
	return 0;
}

void release_context(JNIEnv *env, jobject thiz, struct RecorderContext *recorderContext) {
	if (recorderContext->outputFile) {
		free(recorderContext->outputFile);
		recorderContext->outputFile = NULL;
	}
	if (recorderContext->audioRecordClass) {
		(*env)->DeleteGlobalRef(env, recorderContext->audioRecordClass);
	}
	if (recorderContext->recorderObject) {
		(*env)->DeleteGlobalRef(env, recorderContext->recorderObject);
	}
	if (recorderContext->recorderClass) {
		(*env)->DeleteGlobalRef(env, recorderContext->recorderClass);
	}
	pthread_mutex_destroy(&recorderContext->stateLock);
	free(recorderContext);
	set_context(env, thiz, NULL);
}

int create_audio_record(struct RecorderContext *recorderContext, JNIEnv * env) {
	//creating audioRecord
	jobject audio_record;

	LOGI("create_audio_record begin");

	audio_record = (*env)->CallStaticObjectMethod(env, recorderContext->recorderClass, recorderContext->recorder_create_audio_record_method, recorderContext->sampleRate, recorderContext->channels);

	LOGD("create_audio_record 1");
	jthrowable exc = (*env)->ExceptionOccurred(env);
	if (exc) {
		(*env)->ExceptionClear(env);
		LOGE("create_audio_record CallObjectMethod error");
		return -1;
	}
	if (audio_record == NULL) {
		LOGE("create_audio_record audio_record == NULL");
		return -1;
	}

	LOGD("create_audio_record 2");
	if (recorderContext->audioRecord != NULL) {
		(*env)->CallVoidMethod(env, recorderContext->audioRecord,	recorderContext->audio_record_stop_method);
		(*env)->CallVoidMethod(env, recorderContext->audioRecord,	recorderContext->audio_record_release_method);
		(*env)->DeleteGlobalRef(env, recorderContext->audioRecord);
	}
	recorderContext->audioRecord = (*env)->NewGlobalRef(env, audio_record);
	(*env)->DeleteLocalRef(env, audio_record);
	if (recorderContext->audioRecord == NULL) {
		LOGE("create_audio_record NewGlobalRef error");
		return -1;
	}
	return 0;
}

int delete_audio_record(struct RecorderContext *recorderContext, JNIEnv * env) {
	LOGI("delete_audio_record begin");
	if (recorderContext->audioRecord != NULL) {
		LOGI("delete_audio_record free_audio_record_ref");
		(*env)->CallVoidMethod(env, recorderContext->audioRecord,	recorderContext->audio_record_stop_method);
		(*env)->CallVoidMethod(env, recorderContext->audioRecord,	recorderContext->audio_record_release_method);
		(*env)->DeleteGlobalRef(env, recorderContext->audioRecord);
		recorderContext->audioRecord = NULL;
	}
	LOGI("delete_audio_record end");
	return 0;
}

jint Java_com_kuxing_aacrecorder_Recorder_initNative
  (JNIEnv* env, jobject thiz) {
	int ret;
	unsigned long inputSamples = 0;
	unsigned long maxOutputBytes = 0;
	struct RecorderContext *recorderContext = NULL;
	aacplusEncConfiguration *cfg = NULL;
	jclass audio_record_class = NULL;
	jobject audioTrack = NULL;
	LOGI("Java_com_kuxing_aacrecorder_Recoder_initNative() begin");

	// Allocate
	LOGI( "Java_com_kuxing_aacrecorder_Recoder_initNative() Allocate context");
	recorderContext = (struct RecorderContext*)malloc(sizeof(struct RecorderContext));
	if (recorderContext == NULL) {
		LOGE("Java_com_kuxing_aacrecorder_Recoder_initNative() allocate error");
		goto FAILED;
	}
	pthread_mutex_init(&recorderContext->stateLock, NULL);
	memset(recorderContext, 0, sizeof(struct RecorderContext));
	recorderContext->bitrate = 24000;
	recorderContext->channels = 2;
	recorderContext->sampleRate = 44100;
	recorderContext->inputPcmOffset = 0;
	LOGI( "Java_com_kuxing_ffmpeg_Player_initNative() Allocate context success");

	LOGI("Java_com_kuxing_aacrecorder_Recoder_initNative() Load Recorder APIs");
	{
		jclass recorder_class = (*env)->FindClass(env, recorder_class_path_name);
		if (recorder_class == NULL) {
			LOGE("Java_com_kuxing_aacrecorder_Recoder_initNative() Get player class error");
			goto FAILED;
		}

		recorderContext->recorderClass = (*env)->NewGlobalRef(env, recorder_class);
		recorderContext->recorderObject = (*env)->NewGlobalRef(env, thiz);

		recorderContext->recorder_create_audio_record_method = java_get_static_method(env, recorder_class, recorder_create_audio_track);
		if (recorderContext->recorder_create_audio_record_method == NULL) {
			LOGE("Java_com_kuxing_ffmpeg_Player_initNative() Get prepare_audio_track_method error");
			goto FAILED;
		}
	}
	LOGI("Java_com_kuxing_aacrecorder_Recoder_initNative() Load Recorder APIs success");

	LOGI("Java_com_kuxing_aacrecorder_Recoder_initNative() Create AudioRecord");
	{
		if (create_audio_record(recorderContext, env)) {
			LOGE("Java_com_kuxing_ffmpeg_Player_initNative() create_audio_record error");
			goto FAILED;
		}
	}
	LOGI("Java_com_kuxing_aacrecorder_Recoder_initNative() Create AudioRecord success");

	LOGI("Java_com_kuxing_aacrecorder_Recoder_initNative() Load AudioRecord APIs");
	// Get audio record
	{
		audio_record_class = (*env)->FindClass(env, android_record_class_path_name);
		if (audio_record_class == NULL) {
			LOGE("Java_com_kuxing_aacrecorder_Recoder_initNative audio_record_class error");
			goto FAILED;
		}

		recorderContext->audioRecordClass = (*env)->NewGlobalRef(env,
				audio_record_class);
		if (recorderContext->audioRecordClass == NULL) {
			LOGE("Java_com_kuxing_aacrecorder_Recoder_initNative audioRecordClass error");
			goto FAILED;
		}

		recorderContext->audio_record_read_method = java_get_method(env,
				recorderContext->audioRecordClass, audio_record_read);
		if (recorderContext->audio_record_read_method == NULL) {
			LOGE("Java_com_kuxing_aacrecorder_Recoder_initNative audio_record_read_method error");
			goto FAILED;
		}

		recorderContext->audio_record_start_method = java_get_method(env,
				recorderContext->audioRecordClass, audio_record_start);
		if (recorderContext->audio_record_start_method == NULL) {
			LOGE("Java_com_kuxing_aacrecorder_Recoder_initNative audio_track_start_method error");
			goto FAILED;
		}

		recorderContext->audio_record_stop_method = java_get_method(env,
				recorderContext->audioRecordClass, audio_record_stop);
		if (recorderContext->audio_record_stop_method == NULL) {
			LOGE("Java_com_kuxing_aacrecorder_Recoder_initNative audio_record_stop_method error");
			goto FAILED;
		}

		recorderContext->audio_record_release_method = java_get_method(env,
				recorderContext->audioRecordClass, audio_record_release);
		if (recorderContext->audio_record_release_method == NULL) {
			LOGE("Java_com_kuxing_aacrecorder_Recoder_initNative audio_record_release_method error");
			goto FAILED;
		}

		(*env)->DeleteLocalRef(env, audio_record_class);
		audio_record_class = NULL;
	}
	LOGI( "Java_com_kuxing_aacrecorder_Recoder_initNative() Load AudioRecord APIs success");

	LOGI( "Java_com_kuxing_aacrecorder_Recoder_initNative() Create encoder");
	recorderContext->hEncoder = aacplusEncOpen(recorderContext->sampleRate, recorderContext->channels,
			&inputSamples, &maxOutputBytes);
	recorderContext->inputSamples = inputSamples;
	recorderContext->maxOutputBytes = maxOutputBytes;

	cfg = aacplusEncGetCurrentConfiguration(recorderContext->hEncoder);
	cfg->bitRate = recorderContext->bitrate;
	cfg->bandWidth = 0;
	cfg->outputFormat = 1;
	cfg->nChannelsOut = recorderContext->channels;
	cfg->inputFormat = AACPLUS_INPUT_16BIT;

	if ((ret = aacplusEncSetConfiguration(recorderContext->hEncoder, cfg)) == 0) {
		LOGE("Java_com_kuxing_aacrecorder_Recoder_initNative() set encoder configuration error");
		goto FAILED;
	}
	LOGI("Java_com_kuxing_aacrecorder_Recoder_initNative() set encoder configuration success");
	LOGI( "Java_com_kuxing_ffmpeg_Player_initNative() Create encoder success.");

	LOGI("Java_com_kuxing_aacrecorder_Recoder_initNative() allocate buffers");
	recorderContext->outputBuffer = (uint8_t*)malloc(maxOutputBytes);
	recorderContext->blockSize = recorderContext->inputSamples * sizeof(short);
	recorderContext->inputPcm = (int32_t*)malloc(recorderContext->blockSize);
	LOGI("Java_com_kuxing_aacrecorder_Recoder_initNative() allocate buffers success.");

	if (set_context(env, thiz, recorderContext)) {
		LOGE("Java_com_kuxing_aacrecorder_Recoder_initNative() set_context error");
	}
	return 0;
FAILED:
	if (audio_record_class) {
		(*env)->DeleteLocalRef(env, audio_record_class);
		audio_record_class = NULL;
	}
	if (recorderContext) {
		release_context(env, thiz, recorderContext);
	}
	return -1;
}
void Java_com_kuxing_aacrecorder_Recorder_releaseNative
  (JNIEnv* env, jobject thiz) {
	struct RecorderContext *recorderContext = NULL;
	LOGI("Java_com_kuxing_aacrecorder_Recoder_releaseNative() end");

	recorderContext = (struct RecorderContext*)malloc(sizeof(struct RecorderContext));
	if (recorderContext == NULL) {
		return;
	}
	release_context(env, thiz, recorderContext);
	LOGI("Java_com_kuxing_aacrecorder_Recoder_releaseNative() end");
}
jint Java_com_kuxing_aacrecorder_Recorder_startNative
  (JNIEnv* env, jobject thiz, jstring url) {
	struct RecorderContext * recorderContext = NULL;
	int err = 0;
	int ret;
	recorderContext = get_context(env, thiz);
	if (recorderContext == NULL) {
		err = -1;
		LOGE("Java_com_kuxing_aacrecorder_Recoder_startNative() get_player_context error");
		return err;
	}
	// Set file path
	recorderContext->outputFile = jstringTostring(env, url);
	// Start main thread
	LOGI("Java_com_kuxing_aacrecorder_Recoder_startNative() Start main thread");
	pthread_mutex_lock(&recorderContext->stateLock);
	if (recorderContext->mainThread != NULL) {
		recorderContext->state = RECORDER_STATE_RECORDING;
		pthread_attr_t attr;
		ret = pthread_attr_init(&attr);
		if (ret) {
			err = -1;
			recorderContext->state = RECORDER_STATE_IDLE;
			LOGE("Java_com_kuxing_aacrecorder_Recoder_startNative() pthread_attr_init error");
		} else {
			ret = pthread_create(&recorderContext->mainThread, &attr,
					mainLoop, recorderContext);
			if (ret) {
				err = -1;
				recorderContext->state = RECORDER_STATE_IDLE;
				LOGE("Java_com_kuxing_aacrecorder_Recoder_startNative() pthread_create error");
			}
			pthread_attr_destroy(&attr);
		}
	}
	pthread_mutex_unlock(&recorderContext->stateLock);
	return err;
}
void Java_com_kuxing_aacrecorder_Recorder_stopNative
  (JNIEnv* env, jobject thiz) {
	struct RecorderContext * recorderContext = NULL;
	recorderContext = get_context(env, thiz);
	if (recorderContext == NULL) {
		LOGE("Java_com_kuxing_aacrecorder_Recoder_startNative() get_player_context error");
		return;
	}
	pthread_mutex_lock(&recorderContext->stateLock);
	if (recorderContext->mainThread != NULL) {
		if (recorderContext->state != RECORDER_STATE_TO_RELEASE) {
			recorderContext->state = RECORDER_STATE_TO_RELEASE;
		}
		pthread_join(recorderContext->mainThread, NULL);
		recorderContext->mainThread = NULL;
		recorderContext->volumn = 0;
	}
	pthread_mutex_unlock(&recorderContext->stateLock);
}
jint Java_com_kuxing_aacrecorder_Recorder_getVolumnNative
  (JNIEnv* env, jobject thiz) {
	return 0;
}

void * mainLoop(void *context){
	struct RecorderContext *recorderContext = (struct RecorderContext *)context;
	JNIEnv * env;
	char thread_title[256];
	JavaVMAttachArgs thread_spec = { JNI_VERSION_1_4, thread_title, NULL };
	int ret;
	jint jret;
	FILE* file = NULL;
	LOGI("mainLoop begin");

	sprintf(thread_title, "mainloop");
	jret = (*recorderContext->javavm)->AttachCurrentThread(recorderContext->javavm, &env, &thread_spec);
	if (jret || env == NULL) {
		LOGE("mainLoop AttachCurrentThread failed");
		goto EXIT;
	}

	file = fopen(recorderContext->outputFile, "wb");
	if (file == NULL) {
		LOGE("mainLoop create output file(%s) failed", recorderContext->outputFile);
		goto EXIT;
	}

	while(recorderContext->state != RECORDER_STATE_TO_RELEASE) {
		// read
		ret = (*env)->CallIntMethod(env, recorderContext->audioRecord,	recorderContext->audio_record_read_method,
				(char *)recorderContext->inputPcm, 0, recorderContext->blockSize);
		if (ret < 0) {
			LOGE("mainLoop read AudioRecord failed");
			break;
		}
		LOGD("ret: %d", ret);
		// encode

		// save
	}
EXIT:
	if (file) {
		fflush(file);
		fclose(file);
	}
	if (env) {
		(*recorderContext->javavm)->DetachCurrentThread(recorderContext->javavm);
	}
	LOGI("mainLoop end");
	return NULL;
}

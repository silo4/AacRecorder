/*
 * Player.java
 * Copyright (c) 2013 ZhangPeihao@Kuxing
 */
#include <android/log.h>
#ifndef _Included_com_kuxing_aacrecoder_Recoder_log_h_
#define _Included_com_kuxing_aacrecoder_Recoder_log_h_

#define LOG_LEVEL ANDROID_LOG_INFO
#define LOG_TAG "aacrecoder"
#define LOG(level, ...) if (level >= LOG_LEVEL) {__android_log_print(level, LOG_TAG, __VA_ARGS__);}
#define LOGD(...) if (ANDROID_LOG_DEBUG >= LOG_LEVEL) {__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__);}
#define LOGI(...) if (ANDROID_LOG_INFO >= LOG_LEVEL) {__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__);}
#define LOGW(...) if (ANDROID_LOG_WARN >= LOG_LEVEL) {__android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__);}
#define LOGE(...) if (ANDROID_LOG_ERROR >= LOG_LEVEL) {__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__);}

#endif

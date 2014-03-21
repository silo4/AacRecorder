/*
 * Player.java
 * Copyright (c) 2013 ZhangPeihao@Kuxing
 */
#ifndef _Included_com_kuxing_ffmpeg_Player_javamethod_h_
#define _Included_com_kuxing_ffmpeg_Player_javamethod_h_

#define KX_FORCE_NO_VIDEO
typedef struct {
    const char* name;
    const char* signature;
} JavaMethod;

typedef struct {
    char* name;
    char* signature;
} JavaField;

jfieldID  java_get_field(JNIEnv *env, char * class_name, JavaField field);
jmethodID java_get_method(JNIEnv *env, jclass class, JavaMethod method);
jmethodID java_get_static_method(JNIEnv *env, jclass class, JavaMethod method);

#endif

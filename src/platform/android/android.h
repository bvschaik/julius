#ifndef PLATFORM_ANDROID_H
#define PLATFORM_ANDROID_H

#ifdef __ANDROID__

#include <jni.h>

int android_check_rw_permissions(void);
void android_toast_message(const char *messge);
void Java_bvschaik_julius_JuliusSDL2Activity_setJavaVMForJNI(JNIEnv *env, jobject obj, jint depth);
void android_show_c3_path_dialog(void);
const char* android_get_c3_path(void);

#endif // __ANDROID__
#endif // PLATFORM_ANDROID_H

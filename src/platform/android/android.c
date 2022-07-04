#include "android.h"

#include "assets/assets.h"
#include "core/file.h"
#include "platform/android/asset_handler.h"
#include "platform/android/jni.h"
#include "platform/file_manager.h"

#include "SDL.h"

#include <string.h>

static int has_directory;
static char path[FILE_NAME_MAX];

static const char *get_c3_path(void)
{
    jni_function_handler handler;
    if (!jni_get_static_method_handler(CLASS_FILE_MANAGER, "getC3Path", "()Ljava/lang/String;", &handler)) {
        jni_destroy_function_handler(&handler);
        return NULL;
    }

    jobject result = (*handler.env)->CallStaticObjectMethod(handler.env, handler.class, handler.method);
    const char *temp_path = (*handler.env)->GetStringUTFChars(handler.env, (jstring) result, NULL);
    strncpy(path, temp_path, FILE_NAME_MAX - 1);
    (*handler.env)->ReleaseStringUTFChars(handler.env, (jstring) result, temp_path);
    (*handler.env)->DeleteLocalRef(handler.env, result);
    jni_destroy_function_handler(&handler);

    return *path ? path : NULL;
}

const char *android_show_c3_path_dialog(int again)
{
    jni_function_handler handler;
    if (jni_get_method_handler(CLASS_AUGUSTUS_ACTIVITY, "showDirectorySelection", "(Z)V", &handler)) {
        (*handler.env)->CallVoidMethod(handler.env, handler.activity, handler.method,
            again ? JNI_TRUE : JNI_FALSE);
    }
    jni_destroy_function_handler(&handler);

    has_directory = 0;

    while (!has_directory) {
        SDL_WaitEventTimeout(NULL, 2000);
    }

    return get_c3_path();
}

float android_get_screen_density(void)
{
    jni_function_handler handler;
    float result = 1.0f;
    if (jni_get_method_handler(CLASS_AUGUSTUS_ACTIVITY, "getScreenDensity", "()F", &handler)) {
        result = (float) (*handler.env)->CallFloatMethod(handler.env, handler.activity, handler.method);
    }
    jni_destroy_function_handler(&handler);
    return result;
}

int android_get_file_descriptor(const char *filename, const char *mode)
{
    int result = 0;
    jni_function_handler handler;
    if (!jni_get_static_method_handler(CLASS_FILE_MANAGER, "openFileDescriptor",
        "(Lcom/github/Keriew/augustus/AugustusMainActivity;Ljava/lang/String;Ljava/lang/String;)I", &handler)) {
        jni_destroy_function_handler(&handler);
        return 0;
    }
    jstring jfilename = (*handler.env)->NewStringUTF(handler.env, filename);
    jstring jmode = (*handler.env)->NewStringUTF(handler.env, mode);
    result = (int) (*handler.env)->CallStaticIntMethod(
        handler.env, handler.class, handler.method, handler.activity, jfilename, jmode);
    (*handler.env)->DeleteLocalRef(handler.env, jfilename);
    (*handler.env)->DeleteLocalRef(handler.env, jmode);
    jni_destroy_function_handler(&handler);

    return result;
}

void *android_open_asset(const char *asset, const char *mode)
{
    return asset_handler_open_asset(asset, mode);
}

int android_set_base_path(const char *path)
{
    int result = 0;
    jni_function_handler handler;
    if (!jni_get_static_method_handler(CLASS_FILE_MANAGER, "setBaseUri", "(Ljava/lang/String;)I", &handler)) {
        jni_destroy_function_handler(&handler);
        return 0;
    }
    jstring jpath = (*handler.env)->NewStringUTF(handler.env, path);
    result = (int) (*handler.env)->CallStaticIntMethod(handler.env, handler.class, handler.method, jpath);
    (*handler.env)->DeleteLocalRef(handler.env, jpath);
    jni_destroy_function_handler(&handler);

    return result;
}

int android_get_directory_contents(const char *dir, int type, const char *extension, int (*callback)(const char *))
{
    if (strcmp(dir, ASSETS_DIRECTORY) == 0) {
        return asset_handler_get_directory_contents(type, extension, callback);
    }
    jni_function_handler handler;
    if (!jni_get_static_method_handler(CLASS_FILE_MANAGER, "getDirectoryFileList",
        "(Lcom/github/Keriew/augustus/AugustusMainActivity;Ljava/lang/String;ILjava/lang/String;)[Ljava/lang/String;",
        &handler)) {
        jni_destroy_function_handler(&handler);
        return LIST_ERROR;
    }
    jstring jdir = (*handler.env)->NewStringUTF(handler.env, dir);
    jstring jextension = (*handler.env)->NewStringUTF(handler.env, extension);
    jobjectArray result = (jobjectArray) (*handler.env)->CallStaticObjectMethod(
        handler.env, handler.class, handler.method, handler.activity, jdir, type, jextension);
    (*handler.env)->DeleteLocalRef(handler.env, jdir);
    (*handler.env)->DeleteLocalRef(handler.env, jextension);
    int match = LIST_NO_MATCH;
    int len = (*handler.env)->GetArrayLength(handler.env, result);
    for (int i = 0; i < len; ++i) {
        jstring jfilename = (jstring) (*handler.env)->GetObjectArrayElement(handler.env, result, i);
        const char *filename = (*handler.env)->GetStringUTFChars(handler.env, jfilename, NULL);
        match = callback(filename);
        (*handler.env)->ReleaseStringUTFChars(handler.env, (jstring) jfilename, filename);
        (*handler.env)->DeleteLocalRef(handler.env, jfilename);
        if (match == LIST_MATCH) {
            break;
        }
    }
    (*handler.env)->DeleteLocalRef(handler.env, result);
    jni_destroy_function_handler(&handler);
    return match;
}

int android_remove_file(const char *filename)
{
    int result = 0;
    jni_function_handler handler;
    if (!jni_get_static_method_handler(CLASS_FILE_MANAGER, "deleteFile",
        "(Lcom/github/Keriew/augustus/AugustusMainActivity;Ljava/lang/String;)Z", &handler)) {
        jni_destroy_function_handler(&handler);
        return 0;
    }
    jstring jfilename = (*handler.env)->NewStringUTF(handler.env, filename);
    result = (int) (*handler.env)->CallStaticBooleanMethod(
        handler.env, handler.class, handler.method, handler.activity, jfilename);
    (*handler.env)->DeleteLocalRef(handler.env, jfilename);
    jni_destroy_function_handler(&handler);

    return result;
}

JNIEXPORT void JNICALL Java_com_github_Keriew_augustus_AugustusMainActivity_gotDirectory(JNIEnv *env, jobject thiz)
{
    has_directory = 1;
}

void platform_show_virtual_keyboard(void)
{
    if (!SDL_IsTextInputActive()) {
        SDL_StartTextInput();
    }
}

void platform_hide_virtual_keyboard(void)
{
    if (SDL_IsTextInputActive()) {
        SDL_StopTextInput();
    }
}

#include "android.h"

#include "platform/file_manager.h"

#include "SDL.h"

#include <jni.h>
#include <string.h>

typedef struct {
    JNIEnv *env;
    jclass class;
    jobject activity;
    jmethodID method;
} java_function_handler;

#define GAME_PATH_MAX 300

static int has_directory;
static char path[GAME_PATH_MAX];

static int startup_java_function_handler(const char *class_name, java_function_handler *handler)
{
    handler->env = SDL_AndroidGetJNIEnv();
    if (handler->env == NULL) {
        SDL_Log("Problem setting up JNI environment");
        return 0;
    }
    handler->class = (*handler->env)->FindClass(handler->env, class_name);
    if (handler->class == NULL) {
        SDL_Log("Problem loading class '%s'.", class_name);
        return 0;
    }
    handler->activity = (jobject) SDL_AndroidGetActivity();
    if (handler->activity == NULL) {
        SDL_Log("Problem loading the activity.");
        return 0;
    }
    return 1;
}

static int request_java_static_function_handler(const char *class_name, const char *method_name, const char *method_signature, java_function_handler *handler)
{
    if (!startup_java_function_handler(class_name, handler)) {
        return 0;
    }
    handler->method = (*handler->env)->GetStaticMethodID(handler->env, handler->class, method_name, method_signature);
    if (handler->method == NULL) {
        SDL_Log("Problem loading static method '%s' from class '%s'.", method_name, class_name);
        return 0;
    }
    return 1;
}

static int request_java_class_function_handler(const char *class_name, const char *method_name, const char *method_signature, java_function_handler *handler)
{
    if (!startup_java_function_handler(class_name, handler)) {
        return 0;
    }
    handler->method = (*handler->env)->GetMethodID(handler->env, handler->class, method_name, method_signature);
    if (handler->method == NULL) {
        SDL_Log("Problem loading method '%s' from class '%s'.", method_name, class_name);
        return 0;
    }
    return 1;
}

static void destroy_java_function_handler(java_function_handler *handler)
{
    if (handler->env) {
        if (handler->activity) {
            (*handler->env)->DeleteLocalRef(handler->env, handler->activity);
        }
        if (handler->class) {
            (*handler->env)->DeleteLocalRef(handler->env, handler->class);
        }
    }
    handler->env = NULL;
    handler->class = NULL;
    handler->activity = NULL;
    handler->method = NULL;
}

static const char *get_c3_path(void)
{
    java_function_handler handler;
    if (!request_java_static_function_handler("com/github/bvschaik/julius/FileManager", "getC3Path", "()Ljava/lang/String;", &handler)) {
        destroy_java_function_handler(&handler);
        return NULL;
    }

    jobject result = (*handler.env)->CallStaticObjectMethod(handler.env, handler.class, handler.method);
    const char *temp_path = (*handler.env)->GetStringUTFChars(handler.env, (jstring) result, NULL);
    strncpy(path, temp_path, GAME_PATH_MAX - 1);
    (*handler.env)->ReleaseStringUTFChars(handler.env, (jstring) result, temp_path);
    (*handler.env)->DeleteLocalRef(handler.env, result);
    destroy_java_function_handler(&handler);

    return path;
}

const char *android_show_c3_path_dialog(void)
{
    java_function_handler handler;
    if (request_java_class_function_handler("com/github/bvschaik/julius/JuliusMainActivity", "showDirectorySelection", "()V", &handler)) {
        (*handler.env)->CallVoidMethod(handler.env, handler.activity, handler.method);
    }
    destroy_java_function_handler(&handler);

    has_directory = 0;

    while (!has_directory) {
        SDL_WaitEventTimeout(NULL, 2000);
    }

    return get_c3_path();
}

void android_toast_message(const char *message)
{
    java_function_handler handler;
    if (request_java_class_function_handler("com/github/bvschaik/julius/JuliusMainActivity", "toastMessage", "(Ljava/lang/String;)V", &handler)) {
        jstring jmessage = (*handler.env)->NewStringUTF(handler.env, message);
        (*handler.env)->CallVoidMethod(handler.env, handler.activity, handler.method, jmessage);
        (*handler.env)->DeleteLocalRef(handler.env, jmessage);
    }
    destroy_java_function_handler(&handler);
}

float android_get_screen_scale(void)
{
    java_function_handler handler;
    float result = 1.0f;
    if (request_java_class_function_handler("com/github/bvschaik/julius/JuliusMainActivity", "getScreenScale", "()F", &handler)) {
        result = (float) (*handler.env)->CallFloatMethod(handler.env, handler.activity, handler.method);
    }
    destroy_java_function_handler(&handler);
    return result;
}

int android_get_file_descriptor(const char *filename, const char *mode)
{
    int result = 0;
    java_function_handler handler;
    if (!request_java_static_function_handler("com/github/bvschaik/julius/FileManager", "openFileDescriptor", "(Lcom/github/bvschaik/julius/JuliusMainActivity;Ljava/lang/String;Ljava/lang/String;)I", &handler)) {
        destroy_java_function_handler(&handler);
        return 0;
    }
    jstring jfilename = (*handler.env)->NewStringUTF(handler.env, filename);
    jstring jmode = (*handler.env)->NewStringUTF(handler.env, mode);
    result = (int) (*handler.env)->CallStaticIntMethod(handler.env, handler.class, handler.method, handler.activity, jfilename, jmode);
    (*handler.env)->DeleteLocalRef(handler.env, jfilename);
    (*handler.env)->DeleteLocalRef(handler.env, jmode);
    destroy_java_function_handler(&handler);

    return result;
}

int android_set_base_path(const char *path)
{
    int result = 0;
    java_function_handler handler;
    if (!request_java_static_function_handler("com/github/bvschaik/julius/FileManager", "setBaseUri", "(Ljava/lang/String;)I", &handler)) {
        destroy_java_function_handler(&handler);
        return 0;
    }
    jstring jpath = (*handler.env)->NewStringUTF(handler.env, path);
    result = (int) (*handler.env)->CallStaticIntMethod(handler.env, handler.class, handler.method, jpath);
    (*handler.env)->DeleteLocalRef(handler.env, jpath);
    destroy_java_function_handler(&handler);

    return result;
}

int android_get_directory_contents(const char *dir, int type, const char *extension, int (*callback)(const char *))
{
    java_function_handler handler;
    if (!request_java_static_function_handler("com/github/bvschaik/julius/FileManager", "getDirectoryFileList", "(Lcom/github/bvschaik/julius/JuliusMainActivity;Ljava/lang/String;ILjava/lang/String;)[Ljava/lang/String;", &handler)) {
        destroy_java_function_handler(&handler);
        return LIST_ERROR;
    }
    jstring jdir = (*handler.env)->NewStringUTF(handler.env, dir);
    jstring jextension = (*handler.env)->NewStringUTF(handler.env, extension);
    jobjectArray result = (jobjectArray) (*handler.env)->CallStaticObjectMethod(handler.env, handler.class, handler.method, handler.activity, jdir, type, jextension);
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
    destroy_java_function_handler(&handler);
    return match;
}

int android_remove_file(const char *filename)
{
    int result = 0;
    java_function_handler handler;
    if (!request_java_static_function_handler("com/github/bvschaik/julius/FileManager", "deleteFile", "(Lcom/github/bvschaik/julius/JuliusMainActivity;Ljava/lang/String;)Z", &handler)) {
        destroy_java_function_handler(&handler);
        return 0;
    }
    jstring jfilename = (*handler.env)->NewStringUTF(handler.env, filename);
    result = (int) (*handler.env)->CallStaticBooleanMethod(handler.env, handler.class, handler.method, handler.activity, jfilename);
    (*handler.env)->DeleteLocalRef(handler.env, jfilename);
    destroy_java_function_handler(&handler);

    return result;
}

int android_is_mouse_in_use(void)
{
    return SDL_GetDefaultCursor() != 0 && !SDL_IsDeXMode() && !SDL_IsChromebook();
}

JNIEXPORT void JNICALL Java_com_github_bvschaik_julius_JuliusMainActivity_gotDirectory(JNIEnv *env)
{
    has_directory = 1;
}

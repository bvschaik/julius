#include "android.h"
#include "platform/file.h"
#include "SDL.h"

typedef struct
{
    JNIEnv *env;
    jclass class;
    jobject activity;
    jmethodID method;
} java_function_handler;

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
    handler->activity = (jobject)SDL_AndroidGetActivity();
    if (handler->activity == NULL) {
        SDL_Log("Problem loading the activity.");
        return 0;
    }
    return 1;
}

static int request_java_static_function_handler(const char *class_name, const char *method_name, const char *method_signature, java_function_handler *handler)
{
    if(!startup_java_function_handler(class_name, handler)) {
        return 0;
    }
    handler->method = (*handler->env)->GetStaticMethodID(handler->env, handler->class, method_name, method_signature);
    if (handler->method == NULL) {
        SDL_Log("Problem loading method '%s' from class '%s'.", method_name, class_name);
        return 0;
    }
    return 1;
}

static int request_java_class_function_handler(const char *class_name, const char *method_name, const char *method_signature, java_function_handler *handler)
{
    if(!startup_java_function_handler(class_name, handler)) {
        return 0;
    }
    handler->method = (*handler->env)->GetMethodID(handler->env, handler->class, method_name, method_signature);
    if (handler->method == NULL) {
        SDL_Log("Problem loading method '%s' from class '%s'.", method_name, class_name);
        return 0;
    }
    return 1;
}

static void destroy_java_function_handler(java_function_handler* handler)
{
    if(handler->env && handler->activity) {
        (*handler->env)->DeleteLocalRef(handler->env, handler->activity);
    }
    handler->env = NULL;
    handler->class = NULL;
    handler->activity = NULL;
    handler->method = NULL;
}

static void wait_on_pause(void)
{
    java_function_handler handler;
    if (request_java_class_function_handler("bvschaik/julius/JuliusSDL2Activity", "waitOnPause", "()V", &handler)) {
        (*handler.env)->CallVoidMethod(handler.env, handler.activity, handler.method);
    }
    destroy_java_function_handler(&handler);
}

int android_check_rw_permissions(void)
{
    java_function_handler handler;
    int result = 0;
    if (request_java_static_function_handler("bvschaik/julius/PermissionsManager", "HasWriteAccess", "(Lbvschaik/julius/JuliusSDL2Activity;)Z", &handler)) {
        result = (*handler.env)->CallStaticBooleanMethod(handler.env, handler.class, handler.method, handler.activity);
    }
    destroy_java_function_handler(&handler);
    return result;
}

void android_request_rw_permissions(void)
{
    java_function_handler handler;
    int result = 0;
    if (request_java_class_function_handler("bvschaik/julius/JuliusSDL2Activity", "requestPermissions", "()V", &handler)) {
        (*handler.env)->CallVoidMethod(handler.env, handler.activity, handler.method);
    }
    destroy_java_function_handler(&handler);
    wait_on_pause();
}

void android_toast_message(const char *message)
{
    java_function_handler handler;
    if (request_java_class_function_handler("bvschaik/julius/JuliusSDL2Activity", "toastMessage", "(Ljava/lang/String;)V", &handler)) {
        jstring jmessage = (*handler.env)->NewStringUTF(handler.env, message);
        (*handler.env)->CallVoidMethod(handler.env, handler.activity, handler.method, jmessage);
    }
    destroy_java_function_handler(&handler);
}

void android_show_c3_path_dialog(void)
{
    java_function_handler handler;
    if (request_java_class_function_handler("bvschaik/julius/JuliusSDL2Activity", "showDirectorySelection", "()V", &handler)) {
        (*handler.env)->CallVoidMethod(handler.env, handler.activity, handler.method);
    }
    destroy_java_function_handler(&handler);
}

const char* android_get_c3_path(void)
{
    java_function_handler handler;
    if (!request_java_class_function_handler("bvschaik/julius/JuliusSDL2Activity", "getC3Path", "()Ljava/lang/String;", &handler)) {
        destroy_java_function_handler(&handler);
        return NULL;
    }

    jobject result = (*handler.env)->CallObjectMethod(handler.env, handler.activity, handler.method);
    const char *path = (*handler.env)->GetStringUTFChars(handler.env, (jstring)result, NULL);
    destroy_java_function_handler(&handler);

    return path;
}

float android_get_screen_scale(void)
{
    java_function_handler handler;
    float result = 1.0f;
    if (request_java_class_function_handler("bvschaik/julius/JuliusSDL2Activity", "getScreenScale", "()F", &handler)) {
        result = (float) (*handler.env)->CallFloatMethod(handler.env, handler.activity, handler.method);
    }
    destroy_java_function_handler(&handler);
    return result;
}

JNIEXPORT void JNICALL Java_bvschaik_julius_JuliusSDL2Activity_informCurrentRWPermissions(JNIEnv* env, jobject obj, jboolean hasWriteAccess)
{
    platform_set_file_access_permissions((int)hasWriteAccess);
}

#include "android.h"
#include "SDL.h"

static JavaVM *java_vm = NULL;

typedef struct
{
    JavaVM *vm;
    JNIEnv *env;
    jclass class;
    jmethodID method;
    int env_stat;
} java_function_handler;

static void invalidate_java_function_handler(java_function_handler *handler)
{
    handler->vm = NULL;
    handler->env = NULL;
    handler->class = NULL;
    handler->method = NULL;
    handler->env_stat = JNI_EINVAL;
}

static int request_java_static_function_handler(const char *class_name, const char *method_name, const char *method_signature, java_function_handler *handler)
{
    if (!java_vm) {
        SDL_Log("Problem configuring Java virtual machine.");
        return 0;
    }
    handler->vm = java_vm;
    handler->env_stat = (*handler->vm)->GetEnv(handler->vm, (void**)&handler->env, JNI_VERSION_1_6);
    if (handler->env_stat == JNI_EDETACHED) {
        if ((*handler->vm)->AttachCurrentThread(handler->vm, &handler->env, NULL) != 0) {
            handler->env_stat = JNI_EINVAL;
            SDL_Log("Problem configuring Java virtual machine.");
            return 0;
        }
    } else if (handler->env_stat == JNI_EVERSION) {
        SDL_Log("Incompatible Java version.");
        return 0;
    }
    handler->class = (*handler->env)->FindClass(handler->env, class_name);
    if (handler->class == NULL) {
        SDL_Log("Problem loading class '%s'.", class_name);
        return 0;
    }
    handler->method = (*handler->env)->GetStaticMethodID(handler->env, handler->class, method_name, method_signature);
    if (handler->method == NULL) {
        SDL_Log("Problem loading method '%s' from class '%s'.", method_name, class_name);
        return 0;
    }
    return 1;
}

static void destroy_java_function_handler(java_function_handler *handler)
{
    if (handler->vm && handler->env_stat == JNI_EDETACHED) {
        (*handler->vm)->DetachCurrentThread(handler->vm);
    }
    invalidate_java_function_handler(handler);
}

int android_check_rw_permissions(void)
{
    java_function_handler handler;
    int result = 0;
    if (request_java_static_function_handler("bvschaik/julius/PermissionsManager", "HasWriteAccess", "()Z", &handler)) {
        result = (*handler.env)->CallStaticBooleanMethod(handler.env, handler.class, handler.method);
    }
    destroy_java_function_handler(&handler);
    return result;
}

void android_toast_message(const char *message)
{
    java_function_handler handler;
    if (request_java_static_function_handler("bvschaik/julius/JuliusSDL2Activity", "ToastMessage", "(Ljava/lang/String;)V", &handler)) {
        jstring jmessage = (*handler.env)->NewStringUTF(handler.env, message);
        (*handler.env)->CallStaticVoidMethod(handler.env, handler.class, handler.method, jmessage);
    }
    destroy_java_function_handler(&handler);
}

void android_show_c3_path_dialog(void)
{
    java_function_handler handler;
    if (request_java_static_function_handler("bvschaik/julius/JuliusSDL2Activity", "ShowDirectorySelection", "()V", &handler)) {
        (*handler.env)->CallStaticVoidMethod(handler.env, handler.class, handler.method);
    }
    destroy_java_function_handler(&handler);
}

const char* android_get_c3_path(void)
{
    java_function_handler handler;
    if (!request_java_static_function_handler("bvschaik/julius/JuliusSDL2Activity", "GetC3Path", "()Ljava/lang/String;", &handler)) {
        destroy_java_function_handler(&handler);
        return NULL;
    }

    jobject result = (*handler.env)->CallStaticObjectMethod(handler.env, handler.class, handler.method);
    const char *path = (*handler.env)->GetStringUTFChars(handler.env, (jstring)result, NULL);
    destroy_java_function_handler(&handler);

    return path;
}

JNIEXPORT void JNICALL Java_bvschaik_julius_JuliusSDL2Activity_setJavaVMForJNI(JNIEnv* env, jobject obj, jint depth)
{
    if ((*env)->GetJavaVM(env, &java_vm) != JNI_OK)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Could not get the Java interface.\n\nJulius will not be able to run.", NULL);
        return;
    }
}

#include "asset_handler.h"

#include "assets/assets.h"
#include "core/file.h"
#include "platform/android/android.h"
#include "platform/android/jni.h"
#include "platform/file_manager.h"

#include "SDL.h"

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <stdio.h>
#include <string.h>

static enum {
    ASSETS_LOCATION_NONE = 0,
    ASSETS_LOCATION_DIRECTORY = 1,
    ASSETS_LOCATION_APK = 2
} assets_location;

static jobject java_asset_manager;
static AAssetManager *asset_manager;

static int assets_directory_found(const char *dummy)
{
    return LIST_MATCH;
}

static void determine_assets_location(void)
{
    if (android_get_directory_contents(ASSETS_DIR_NAME, TYPE_FILE, "xml", assets_directory_found) == LIST_MATCH) {
        assets_location = ASSETS_LOCATION_DIRECTORY;
    } else {
        assets_location = ASSETS_LOCATION_APK;
    }
}

static AAssetManager *get_asset_manager(void)
{
    if (asset_manager) {
        return asset_manager;
    }
    jni_function_handler handler;
    if (jni_get_method_handler(0, "getAssets", "()Landroid/content/res/AssetManager;", &handler)) {
        jobject local_asset_manager = (jobject) (*handler.env)->CallObjectMethod(handler.env, handler.activity, handler.method);
        java_asset_manager = (jobject) (*handler.env)->NewGlobalRef(handler.env, local_asset_manager);
        asset_manager = AAssetManager_fromJava(handler.env, java_asset_manager);
        if (!asset_manager) {
            (*handler.env)->DeleteGlobalRef(handler.env, java_asset_manager);
        }
    }
    jni_destroy_function_handler(&handler);
    return asset_manager;
}

// Mask AAsset handler as file pointer
// Courtesy of http://www.50ply.com/blog/2013/01/19/loading-compressed-android-assets-with-file-pointer/
static int asset_read(void *asset, char *buf, int size) {
    return AAsset_read((AAsset *)asset, buf, size);
}

static fpos_t asset_seek(void *asset, fpos_t offset, int whence) {
    return AAsset_seek((AAsset *)asset, offset, whence);
}

static int asset_close(void *asset) {
    AAsset_close((AAsset *)asset);
    return 0;
}

void *asset_handler_open_asset(const char *asset_name, const char *mode)
{
    static char location[FILE_NAME_MAX];
    static int assets_dir_length;
    if(!assets_dir_length) {
        assets_dir_length = strlen(ASSETS_DIR_NAME) + 1;
        strncpy(location, ASSETS_DIR_NAME, FILE_NAME_MAX);
        location[assets_dir_length - 1] = '/';
    }

    switch (assets_location) {
        case ASSETS_LOCATION_DIRECTORY:
            strncpy(location + assets_dir_length, asset_name, FILE_NAME_MAX - assets_dir_length - 1);
            int fd = android_get_file_descriptor(location, mode);
            if (!fd) {
                return NULL;
            }
            return fdopen(fd, mode);
        case ASSETS_LOCATION_APK:
            if (!get_asset_manager()) {
                return 0;
            }
            AAsset *asset = AAssetManager_open(get_asset_manager(), asset_name, AASSET_MODE_STREAMING);
            return asset ? funopen(asset, asset_read, 0, asset_seek, asset_close) : 0;
        default:
            return 0;
    }
}

int asset_handler_get_directory_contents(int type, const char *extension, int (*callback)(const char *))
{
    if (assets_location == ASSETS_LOCATION_NONE) {
        determine_assets_location();
    }
    if (assets_location == ASSETS_LOCATION_DIRECTORY) {
        return android_get_directory_contents(ASSETS_DIR_NAME, type, extension, callback);
    }
    AAssetManager *manager = get_asset_manager();
    if (!manager) {
        return LIST_ERROR;
    }
    AAssetDir *dir = AAssetManager_openDir(manager, "");
    int match = LIST_NO_MATCH;
    const char *asset_name = 0;
    while ((asset_name = AAssetDir_getNextFileName(dir))) {
        char *asset_extension = strrchr(asset_name, '.');
        if (asset_extension && strcmp(asset_extension + 1, extension) == 0) {
            match = callback(asset_name);
        }
        if (match == LIST_MATCH) {
            break;
        }
    }
    AAssetDir_close(dir);
    return match;
}

JNIEXPORT void JNICALL Java_com_github_Keriew_augustus_AugustusMainActivity_releaseAssetManager(JNIEnv *env, jobject thiz)
{
    if (asset_manager) {
        JNIEnv *env = SDL_AndroidGetJNIEnv();
        (*env)->DeleteGlobalRef(env, java_asset_manager);
        asset_manager = 0;
    }
}

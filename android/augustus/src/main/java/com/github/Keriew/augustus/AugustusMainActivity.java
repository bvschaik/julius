package com.github.Keriew.augustus;

import android.content.Intent;
import android.net.Uri;

import org.libsdl.app.SDLActivity;

public class AugustusMainActivity extends SDLActivity {
    private static final int GET_FOLDER_RESULT = 500;

    @Override
    public void onStop() {
        super.onStop();
        releaseAssetManager();
        FileManager.clearCache();
    }

    @Override
    protected String[] getLibraries() {
        return new String[]{
                "SDL2",
                "SDL2_mixer",
                "augustus"
        };
    }

    @SuppressWarnings("unused")
    public void showDirectorySelection(boolean again) {
        startActivityForResult(DirectorySelectionActivity.newIntent(this, again), GET_FOLDER_RESULT);
    }

    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == GET_FOLDER_RESULT) {
            if (resultCode == RESULT_OK && data != null && data.getData() != null) {
                FileManager.setBaseUri(data.getData());
            } else {
                FileManager.setBaseUri(Uri.EMPTY);
            }
            gotDirectory();
        } else {
            super.onActivityResult(requestCode, resultCode, data);
        }
    }

    public float getScreenDensity() {
        return getResources().getDisplayMetrics().density;
    }

    private native void gotDirectory();
    private native void releaseAssetManager();
}

package com.github.bvschaik.julius;

import android.content.Intent;
import android.net.Uri;
import android.widget.Toast;

import org.libsdl.app.SDLActivity;

public class JuliusMainActivity extends SDLActivity {
    private static final int GET_FOLDER_RESULT = 500;

    @Override
    public void onStop() {
        super.onStop();
        FileManager.clearCache();
    }

    @Override
    protected String[] getLibraries() {
        return new String[]{
                "SDL2",
                "SDL2_mixer",
                "julius"
        };
    }

    public void showDirectorySelection() {
        Intent intent = new Intent(this, DirectorySelectionActivity.class);
        startActivityForResult(intent, GET_FOLDER_RESULT);
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

    public void toastMessage(final String message) {
        runOnUiThread(new Runnable() {
            public void run() {
                Toast.makeText(getApplicationContext(), message, Toast.LENGTH_SHORT).show();
            }
        });
    }

    public float getScreenScale() {
        return getResources().getDisplayMetrics().density;
    }

    private native void gotDirectory();
}

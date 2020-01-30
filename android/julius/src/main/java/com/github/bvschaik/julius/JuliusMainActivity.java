package com.github.bvschaik.julius;

import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.widget.Toast;
import org.libsdl.app.SDLActivity;

public class JuliusMainActivity extends SDLActivity {
    private static final int GET_FOLDER_RESULT = 500;
    private static final int RW_FLAGS_PERMISSION = Intent.FLAG_GRANT_READ_URI_PERMISSION
                                                 | Intent.FLAG_GRANT_WRITE_URI_PERMISSION;

    @Override
    public void onStop()
    {
        super.onStop();
        FileManager.folderStructureCache.clear();
    }

    @Override
    protected String[] getLibraries() {
        return new String[]{
            "SDL2",
            "SDL2_mixer",
            "julius"
        };
    }

    public void showDirectorySelection()
    {
        // Wait before showing window
        new Handler(Looper.getMainLooper()).postDelayed(new Runnable()
        {
            public void run() {
                Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
                intent.addFlags(
                        RW_FLAGS_PERMISSION
                                | Intent.FLAG_GRANT_PREFIX_URI_PERMISSION
                                | Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION
                );
                intent.putExtra("android.content.extra.SHOW_ADVANCED", true);
                intent.putExtra("android.content.extra.FANCY", true);
                intent.putExtra("android.content.extra.SHOW_FILESIZE", true);
                startActivityForResult(intent, GET_FOLDER_RESULT);
            }
        }, 1000);
    }

    protected void onActivityResult(int requestCode, int resultCode, Intent data)
    {
        if (requestCode == GET_FOLDER_RESULT) {
            if (data.getData() == null) {
                this.gotDirectory();
                return;
            }

            getContentResolver().takePersistableUriPermission(data.getData(), data.getFlags() & RW_FLAGS_PERMISSION);
            FileManager.setBaseUri(data.getData());
            this.gotDirectory();
        }
    }

    public void toastMessage(final String message)
    {
        new Handler(Looper.getMainLooper()).post(new Runnable()
        {
            public void run()
            {
                Toast.makeText(getApplicationContext(), message, Toast.LENGTH_SHORT).show();
            }
        });
    }

    public float getScreenScale()
    {
        return getResources().getDisplayMetrics().density;
    }

    private native void gotDirectory();
}

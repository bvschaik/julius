package bvschaik.julius;

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.widget.Toast;
import org.libsdl.app.SDLActivity;

public class JuliusSDL2Activity extends SDLActivity
{
    private Uri C3Path = Uri.EMPTY;
    private static final int GET_FOLDER_RESULT = 500;
    public boolean paused = true;

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        informCurrentRWPermissions(PermissionsManager.HasWriteAccess(this));
    }

    @Override
    public void onStart()
    {
        super.onStart();
        paused = false;
    }

    @Override
    protected String[] getLibraries() {
        return new String[]{
            "SDL2",
            "SDL2_mixer",
            "julius"
        };
    }

    @Override
    protected void onPause()
    {
        super.onPause();
        paused = true;
    }
    @Override
    protected void onResume()
    {
        super.onResume();
        paused = false;
    }

    private void showDirectoryIntent()
    {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        startActivityForResult(intent, GET_FOLDER_RESULT);
        paused = true;
    }

    protected void onActivityResult(int requestCode, int resultCode, Intent data)
    {
        switch(requestCode) {
            case GET_FOLDER_RESULT:
                C3Path = data.getData();
                break;
        }
    }

    public void toastMessage(final String message)
    {
        Handler handler = new Handler(Looper.getMainLooper());
        handler.post(new Runnable()
        {
            public void run()
            {
                Toast.makeText(getApplicationContext(), message, Toast.LENGTH_SHORT).show();
            }
        });
    }

    public void showDirectorySelection()
    {
        // Wait before showing window
        Sleep(1000);
        showDirectoryIntent();
    }

    public void requestPermissions()
    {
        if(PermissionsManager.RequestPermissions(this)) {
            paused = true;
        }
    }

    public String getC3Path()
    {
        // Halt startup until directory is selected
        waitOnPause();
        return DirectoryHelper.GetPathFromUri(C3Path);
    }

    public void waitOnPause()
    {
        while(paused) {
            Sleep(1000);
        }
    }

    public float getScreenScale()
    {
        return this.getResources().getDisplayMetrics().density;
    }

    static private void Sleep(int ms)
    {
        try {
            Thread.sleep(ms);
        } catch(InterruptedException e) {}
    }

    public native void informCurrentRWPermissions(boolean hasWriteAccess);
}

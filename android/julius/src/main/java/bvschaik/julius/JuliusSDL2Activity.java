package bvschaik.julius;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.widget.Toast;
import org.libsdl.app.SDLActivity;

public class JuliusSDL2Activity extends SDLActivity
{
    public static JuliusSDL2Activity m_activity;
    public static Context m_context;
    private static Uri C3Path = Uri.EMPTY;
    private static final int GET_FOLDER_RESULT = 500;

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        m_activity = this;
        m_context = getApplicationContext();
        if (savedInstanceState == null) {
            PermissionsManager.CheckPermissions(this);
        }
        setJavaVMForJNI();
    }

    @Override
    protected String[] getLibraries() {
        return new String[]{
            "SDL2",
            "SDL2_mixer",
            "julius"
        };
    }

    public void showDirectoryIntent()
    {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        startActivityForResult(intent, GET_FOLDER_RESULT);
    }

    public void onActivityResult(int requestCode, int resultCode, Intent data)
    {
        switch(requestCode) {
            case GET_FOLDER_RESULT:
                C3Path = data.getData();
                break;
        }
    }

    static public void ToastMessage(final String message)
    {
        Handler handler = new Handler(Looper.getMainLooper());
        handler.post(new Runnable()
        {
            public void run()
            {
                Toast.makeText(m_context, message, Toast.LENGTH_SHORT).show();
            }
        });
    }

    static public void ShowDirectorySelection()
    {
        // Wait before showing window
        Sleep(1000);
        m_activity.showDirectoryIntent();
    }

    static public String GetC3Path()
    {
        // Halt startup until directory is selected
        while(C3Path == Uri.EMPTY) {
            Sleep(1000);
        }
        return DirectoryHelper.GetPathFromUri(m_context, C3Path);
    }

    static private void Sleep(int ms)
    {
        try {
            Thread.sleep(ms);
        } catch(InterruptedException e) {}
    }

    public native void setJavaVMForJNI();
}

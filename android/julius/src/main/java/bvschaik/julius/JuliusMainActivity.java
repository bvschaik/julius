package bvschaik.julius;

import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.widget.Toast;
import org.libsdl.app.SDLActivity;

public class JuliusMainActivity extends SDLActivity
{
    private static final int GET_FOLDER_RESULT = 500;
    private static final int rwFlagsPermission = Intent.FLAG_GRANT_READ_URI_PERMISSION
                                               | Intent.FLAG_GRANT_WRITE_URI_PERMISSION;
    public static boolean paused = true;

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
    }

    @Override
    public void onStart()
    {
        super.onStart();
        paused = false;
    }

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

    public void showDirectorySelection()
    {
        // Wait before showing window
        Sleep(1000);

        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        intent.addFlags(
                rwFlagsPermission
                        | Intent.FLAG_GRANT_PREFIX_URI_PERMISSION
                        | Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION
        );
        startActivityForResult(intent, GET_FOLDER_RESULT);
        paused = true;
    }

    protected void onActivityResult(int requestCode, int resultCode, Intent data)
    {
        if (requestCode == GET_FOLDER_RESULT) {
            if (data.getData() == null) {
                return;
            }

            getContentResolver().takePersistableUriPermission(data.getData(), data.getFlags() & rwFlagsPermission);
            FileManager.setBaseUri(data.getData());
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

    static public void waitOnPause()
    {
        while(paused) {
            Sleep(1000);
        }
    }

    public float getScreenScale()
    {
        return getResources().getDisplayMetrics().density;
    }

    static private void Sleep(int ms)
    {
        try {
            Thread.sleep(ms);
        } catch(InterruptedException e) {}
    }
}

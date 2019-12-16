package bvschaik.julius;

import android.content.pm.PackageManager;
import android.Manifest;
import android.os.Build;
import android.support.v4.content.ContextCompat;

public class PermissionsManager
{
    private static final int REQUEST_CODE_WRITE_PERMISSION = 500;

    public static boolean RequestPermissions(final JuliusSDL2Activity activity)
    {
        if (HasWriteAccess(activity)) {
            return false;
        }
        activity.requestPermissions(new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, REQUEST_CODE_WRITE_PERMISSION);
        return true;
    }

    public static boolean HasWriteAccess(final JuliusSDL2Activity activity)
    {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            return ContextCompat.checkSelfPermission(activity, Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED;
        }
        return true;
    }
}

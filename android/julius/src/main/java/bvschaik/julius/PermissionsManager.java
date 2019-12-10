package bvschaik.julius;

import android.app.Activity;
import android.content.pm.PackageManager;
import android.Manifest;
import android.os.Build;
import android.support.v4.content.ContextCompat;

public class PermissionsManager
{
    private static final int REQUEST_CODE_WRITE_PERMISSION = 500;
    private static Activity m_activity;

    public static boolean CheckPermissions(final Activity activity)
    {
        m_activity = activity;
        if (HasWriteAccess()) {
            return true;
        }
        activity.requestPermissions(new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, REQUEST_CODE_WRITE_PERMISSION);
        return false;
    }

    public static boolean HasWriteAccess()
    {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            return ContextCompat.checkSelfPermission(m_activity, Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED;
        }
        return true;
    }
}

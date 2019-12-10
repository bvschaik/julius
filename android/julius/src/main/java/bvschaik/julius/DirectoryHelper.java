package bvschaik.julius;

import android.content.Context;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;

public class DirectoryHelper
{    
    public static String GetPathFromUri(final Context context, final Uri uri)
    {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.KITKAT) {
            return "";
        }
        if ("com.android.externalstorage.documents".equals(uri.getAuthority())) {
            final String docId = uri.getLastPathSegment();
            final String[] split = docId.split(":");
            final String type = split[0];

            if ("primary".equalsIgnoreCase(type)) {
                if (split.length > 1) {
                    return Environment.getExternalStorageDirectory() + "/" + split[1] + "/";
                } else {
                    return Environment.getExternalStorageDirectory() + "/";
                }
            } else {
                return "/storage" + "/" + docId.replace(":", "/");
            }
        }
        return "";
    }
}

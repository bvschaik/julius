package bvschaik.julius;

import android.app.Activity;
import android.database.Cursor;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.provider.DocumentsContract;
import android.util.Log;

import java.util.ArrayList;
import java.util.HashMap;

public class FileManager
{
    private static Uri baseUri = Uri.EMPTY;
    static HashMap<Uri, HashMap<String, FileInfo>> folderStructureCache = new HashMap<>();

    public static String getC3Path()
    {
        // Halt startup until directory is selected
        JuliusMainActivity.waitOnPause();
        return baseUri.toString();
    }

    public static int setBaseUri(String path)
    {
        folderStructureCache.clear();
        if(baseUri != Uri.EMPTY) {
            return 1;
        }
        return setBaseUri(Uri.parse(path));
    }

    static int setBaseUri(Uri newUri)
    {
        try {
            baseUri = newUri;
            FileInfo.base = new FileInfo(DocumentsContract.getTreeDocumentId(newUri), null, DocumentsContract.Document.MIME_TYPE_DIR, Uri.EMPTY);
            return 1;
        } catch(Exception e) {
            Log.e("julius", "Error in setBaseUri: " + e);
            return 0;
        }
    }

    private static HashMap<String, FileInfo> getFolderContents(Activity activity, FileInfo folder)
    {
        if(!folder.isDirectory()) {
            return new HashMap<>();
        }
        return getFolderContents(activity, folder.getUri());
    }

    private static HashMap<String, FileInfo> getFolderContents(Activity activity, Uri folder)
    {
        HashMap<String, FileInfo> result = folderStructureCache.get(folder);
        if(result != null) {
            return result;
        }
        result = new HashMap<>();
        Uri children = DocumentsContract.buildChildDocumentsUriUsingTree(folder, DocumentsContract.getDocumentId(folder));
        String[] columns = new String[] {
                DocumentsContract.Document.COLUMN_DOCUMENT_ID,
                DocumentsContract.Document.COLUMN_DISPLAY_NAME,
                DocumentsContract.Document.COLUMN_MIME_TYPE
        };
        Cursor cursor = activity.getContentResolver().query(children, columns, null, null, null);
        if(cursor != null) {
            while (cursor.moveToNext()) {
                result.put(cursor.getString(1).toLowerCase(), new FileInfo(cursor.getString(0), cursor.getString(1), cursor.getString(2), folder));
            }
            cursor.close();
        }
        folderStructureCache.put(folder, result);
        return result;
    }

    private static FileInfo findFile(Activity activity, FileInfo folder, String fileName)
    {
        return getFolderContents(activity, folder).get(fileName.toLowerCase());
    }

    private static FileInfo getFolderFromPath(Activity activity, String[] path)
    {
        FileInfo currentDir = FileInfo.base;

        for (int i = 0; i < path.length - 1; ++i) {
            currentDir = findFile(activity, currentDir, path[i]);
            if(currentDir == null || !currentDir.isDirectory()) {
                return null;
            }
        }
        return currentDir;
    }

    private static FileInfo getFileFromPath(JuliusMainActivity activity, String filePath)
    {
        try {
            if(baseUri == Uri.EMPTY) {
                return null;
            }
            String[] filePart = filePath.split("[\\\\/]");
            FileInfo folderInfo = getFolderFromPath(activity, filePart);
            if(folderInfo == null) {
                return null;
            }
            return findFile(activity, folderInfo, filePart[filePart.length - 1]);
        } catch(Exception e) {
            Log.e("julius", "Error in getFileFromPath: " + e);
            return null;
        }
    }

    public static String[] getFilesByExtension(JuliusMainActivity activity, String ext)
    {
        ArrayList<String> fileList = new ArrayList<>();

        if(baseUri == Uri.EMPTY) {
            return new String[0];
        }

        for (FileInfo file : getFolderContents(activity, FileInfo.base).values()) {
            if(file.isDirectory()) {
                continue;
            }
            String fileName = file.getName();
            int extCharPos = fileName.lastIndexOf('.');
            if (extCharPos == -1 && ext.isEmpty()) {
                fileList.add(fileName);
                continue;
            }
            String currentExtension = fileName.substring(extCharPos + 1);
            if(currentExtension.equalsIgnoreCase(ext)) {
                fileList.add(fileName);
            }
        }
        String[] result = new String[fileList.size()];
        return fileList.toArray(result);
    }

    public static boolean fileExists(JuliusMainActivity activity, String filePath)
    {
        return getFileFromPath(activity, filePath) != null;
    }

    public static boolean deleteFile(JuliusMainActivity activity, String filePath)
    {
        try {
            FileInfo fileInfo = getFileFromPath(activity, filePath);
            if(fileInfo == null) {
                return false;
            }
            HashMap<String, FileInfo> folderList = folderStructureCache.get(fileInfo.getParentUri());
            if(folderList != null) {
                folderList.remove(fileInfo.getName().toLowerCase());
            }
            return DocumentsContract.deleteDocument(activity.getContentResolver(), fileInfo.getUri());
        } catch(Exception e) {
            Log.e("julius", "Error in deleteFile: " + e);
            return false;
        }
    }

    public static int openFileDescriptor(JuliusMainActivity activity, String filePath, String mode)
    {
        try {
            if(baseUri == Uri.EMPTY) {
                return 0;
            }
            String internalMode = "";
            boolean isWrite = false;
            // Either "r" or "w"
            if(mode.indexOf('r') != -1) {
                internalMode += "r";
            }
            if(mode.indexOf('w') != -1) {
                internalMode += "w";
                isWrite = true;
            }

            String[] fileParts = filePath.split("[\\\\/]");
            String fileName = fileParts[fileParts.length - 1];
            FileInfo folderInfo = getFolderFromPath(activity, fileParts);
            if(folderInfo == null) {
                return 0;
            }
            FileInfo fileInfo = findFile(activity, folderInfo, fileName);
            Uri fileUri;
            if(fileInfo == null) {
                if(!isWrite) {
                    return 0;
                } else {
                    fileUri = DocumentsContract.createDocument(activity.getContentResolver(), folderInfo.getUri(), "application/octet-stream", fileName);
                    if(fileUri == null) {
                        return 0;
                    }
                    HashMap<String, FileInfo> folderCache = folderStructureCache.get(folderInfo.getUri());
                    if(folderCache != null) {
                        fileInfo = new FileInfo(DocumentsContract.getDocumentId(fileUri), fileName, "application/octet-stream", folderInfo.getUri());
                        folderCache.put(fileName.toLowerCase(), fileInfo);
                    }
                }
            } else {
                fileUri = fileInfo.getUri();
            }
            ParcelFileDescriptor pfd = activity.getContentResolver().openFileDescriptor(fileUri, internalMode);
            return (pfd == null) ? 0 : pfd.detachFd();
        } catch (Exception e) {
            Log.e("julius", "Error in openFileDescriptor: " + e);
            return 0;
        }
    }

    private static class FileInfo
    {
        static FileInfo base;
        private String documentId;
        private String name;
        private String mimeType;
        private Uri parent;
        private Uri uri;

        private FileInfo(String documentId, String name, String mimeType, Uri parent)
        {
            this.documentId = documentId;
            this.name = name;
            this.mimeType = mimeType;
            this.parent = parent;
            this.uri = Uri.EMPTY;
        }

        public String getName()
        {
            return name;
        }

        public boolean isDirectory()
        {
            return mimeType.equals(DocumentsContract.Document.MIME_TYPE_DIR);
        }

        Uri getUri()
        {
            if(baseUri != Uri.EMPTY && uri == Uri.EMPTY) {
                uri = DocumentsContract.buildDocumentUriUsingTree(baseUri, documentId);
            }
            return uri;
        }

        Uri getParentUri()
        {
            return parent;
        }
    }
}

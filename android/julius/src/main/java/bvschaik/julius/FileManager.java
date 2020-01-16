package bvschaik.julius;

import android.app.Activity;
import android.database.Cursor;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.provider.DocumentsContract;
import android.util.Log;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

public class FileManager
{
    private static Uri baseUri = Uri.EMPTY;
    static HashMap<Uri, List<FileInfo>> folderStructureCache = new HashMap<>();

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
            FileInfo.base = new FileInfo(DocumentsContract.getTreeDocumentId(newUri), null, DocumentsContract.Document.MIME_TYPE_DIR);
            return 1;
        } catch(Exception e) {
            Log.e("julius", "Error in setBaseUri: " + e);
            return 0;
        }
    }

    private static List<FileInfo> getFolderFileList(Activity activity, FileInfo folder)
    {
        if(!folder.isDirectory()) {
            return new ArrayList<>();
        }
        return getFolderFileList(activity, folder.getUri());
    }

    private static List<FileInfo> getFolderFileList(Activity activity, Uri folder)
    {
        List<FileInfo> result = folderStructureCache.get(folder);
        if(result != null) {
            return result;
        }
        result = new ArrayList<>();
        Uri children = DocumentsContract.buildChildDocumentsUriUsingTree(folder, DocumentsContract.getDocumentId(folder));
        String[] columns = new String[] {
                DocumentsContract.Document.COLUMN_DOCUMENT_ID,
                DocumentsContract.Document.COLUMN_DISPLAY_NAME,
                DocumentsContract.Document.COLUMN_MIME_TYPE
        };
        Cursor cursor = activity.getContentResolver().query(children, columns,null,null,null);
        if(cursor != null) {
            while (cursor.moveToNext()) {
                result.add(new FileInfo(cursor.getString(0), cursor.getString(1), cursor.getString(2)));
            }
            cursor.close();
        }
        folderStructureCache.put(folder, result);
        return result;
    }

    private static FileInfo findFile(Activity activity, FileInfo folder, String fileName)
    {
        for(FileInfo file : getFolderFileList(activity, folder)) {
            if(fileName.equalsIgnoreCase(file.getName())) {
                return file;
            }
        }
        return null;
    }

    private static FileInfo getPathFolder(Activity activity, String[] path)
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

    private static FileInfo getFile(JuliusMainActivity activity, String filename)
    {
        try {
            if(baseUri == Uri.EMPTY) {
                return null;
            }
            String[] filepart = filename.split("[\\\\/]");
            FileInfo folderInfo = getPathFolder(activity, filepart);
            if(folderInfo == null) {
                return null;
            }
            return findFile(activity, folderInfo, filepart[filepart.length - 1]);
        } catch(Exception e) {
            Log.e("julius", "Error in getFile: " + e);
            return null;
        }
    }

    public static String[] getFilesByExtension(JuliusMainActivity activity, String ext)
    {
        List<String> fileList = new ArrayList<>();

        if(baseUri == Uri.EMPTY) {
            return new String[0];
        }

        for (FileInfo file : getFolderFileList(activity, FileInfo.base)) {
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

    public static boolean fileExists(JuliusMainActivity activity, String filename)
    {
        return getFile(activity, filename) != null;
    }

    public static boolean deleteFile(JuliusMainActivity activity, String filename)
    {
        try {
            FileInfo fileInfo = getFile(activity, filename);
            if(fileInfo == null) {
                return false;
            }
            folderStructureCache.clear();
            return DocumentsContract.deleteDocument(activity.getContentResolver(), fileInfo.getUri());
        } catch(Exception e) {
            Log.e("julius", "Error in deleteFile: " + e);
            return false;
        }
    }

    public static int openFileDescriptor(JuliusMainActivity activity, String filename, String mode)
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

            String[] filepart = filename.split("[\\\\/]");
            FileInfo folderInfo = getPathFolder(activity, filepart);
            if(folderInfo == null) {
                return 0;
            }
            FileInfo fileInfo = findFile(activity, folderInfo, filepart[filepart.length - 1]);
            Uri fileUri;
            if(fileInfo == null) {
                if(!isWrite) {
                    return 0;
                } else {
                    folderStructureCache.remove(folderInfo.getUri());
                    fileUri = DocumentsContract.createDocument(activity.getContentResolver(), folderInfo.getUri(), "application/octet-stream", filepart[filepart.length - 1]);
                    if(fileUri == null) {
                        return 0;
                    }
                }
            } else {
                fileUri = fileInfo.getUri();
            }
            ParcelFileDescriptor pfd = activity.getContentResolver().openFileDescriptor(fileUri, internalMode);
            return pfd.detachFd();
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
        private Uri uri;
        private String mimetype;

        private FileInfo(String documentId, String name, String mimetype)
        {
            this.documentId = documentId;
            this.name = name;
            this.mimetype = mimetype;
            this.uri = Uri.EMPTY;
        }

        public String getName()
        {
            return name;
        }

        public boolean isDirectory()
        {
            return mimetype.equals(DocumentsContract.Document.MIME_TYPE_DIR);
        }

        Uri getUri()
        {
            if(baseUri != Uri.EMPTY && uri == Uri.EMPTY) {
                uri = DocumentsContract.buildDocumentUriUsingTree(baseUri, documentId);
            }
            return uri;
        }
    }
}

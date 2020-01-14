package bvschaik.julius;

import android.app.Activity;
import android.database.Cursor;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.provider.DocumentsContract;
import android.support.v4.provider.DocumentFile;

import java.util.ArrayList;
import java.util.List;

public class FileManager
{
    private static Uri baseUri = Uri.EMPTY;

    public static String getC3Path()
    {
        // Halt startup until directory is selected
        JuliusSDL2Activity.waitOnPause();
        return baseUri.toString();
    }

    public static int setBaseUri(String path)
    {
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
            return 0;
        }
    }

    private static List<FileInfo> getFolderFileList(Activity activity, FileInfo folder)
    {
        List<FileInfo> result = new ArrayList<>();
        if(!folder.isDirectory()) {
            return result;
        }
        return getFolderFileList(activity, folder.getUri());
    }

    private static List<FileInfo> getFolderFileList(Activity activity, Uri folder)
    {
        List<FileInfo> result = new ArrayList<>();
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
    
    private static FileInfo getFile(JuliusSDL2Activity activity, String filename)
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
            return null;
        }
    }

    public static String[] getFilesByExtension(JuliusSDL2Activity activity, String ext)
    {
        List<String> fileList = new ArrayList<>();

        if(baseUri == Uri.EMPTY) {
            return new String[0];
        }

        for (FileInfo file : getFolderFileList(activity, baseUri)) {
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

    public static boolean fileExists(JuliusSDL2Activity activity, String filename)
    {
        return getFile(activity, filename) != null;
    }

    public static boolean deleteFile(JuliusSDL2Activity activity, String filename)
    {
        try {
            FileInfo fileInfo = getFile(activity, filename);
            if(fileInfo == null) {
                return false;
            }
            DocumentFile file = fileInfo.generateDocumentFile(activity);
            return file.delete();
        } catch(Exception e) {
            return false;
        }   
    }

    public static int openFileDescriptor(JuliusSDL2Activity activity, String filename, String mode)
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
                    DocumentFile folder = folderInfo.generateDocumentFile(activity);
                    DocumentFile file = folder.createFile("application/octet-stream", filepart[filepart.length - 1]);
                    fileUri = file.getUri();
                }
            } else {
                fileUri = fileInfo.getUri();
            }
            ParcelFileDescriptor pfd = activity.getContentResolver().openFileDescriptor(fileUri, internalMode);
            return pfd.detachFd();
        } catch (Exception e) {
            return 0;
        }
    }

    private static class FileInfo
    {
        public static FileInfo base;
        private String documentId;
        private String name;
        private DocumentFile file = null;
        private String mimetype;

        private FileInfo(String documentId, String name, String mimetype)
        {
            this.documentId = documentId;
            this.name = name;
            this.mimetype = mimetype;
        }

        public String getName()
        {
            return name;
        }

        public boolean isDirectory()
        {
            return mimetype.equals(DocumentsContract.Document.MIME_TYPE_DIR);
        }

        public Uri getUri()
        {
            return DocumentsContract.buildDocumentUriUsingTree(baseUri, documentId);
        }

        private DocumentFile generateDocumentFile(Activity activity)
        {
            if(baseUri == null) {
                return null;
            }
            if(file == null) {
                if (isDirectory()) {
                    file = DocumentFile.fromTreeUri(activity, getUri());
                } else {
                    file = DocumentFile.fromSingleUri(activity, getUri());
                }
            }
            return file;
        }
    }
}
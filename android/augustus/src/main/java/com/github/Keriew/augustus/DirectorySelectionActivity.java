package com.github.Keriew.augustus;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

public class DirectorySelectionActivity extends AppCompatActivity {
    private static final String ARG_SKIP_INSTRUCTIONS = "arg_skip_instructions";
    private static final int RC_GET_EXTERNAL_DIR = 501;
    private static final int RW_FLAGS_PERMISSION = Intent.FLAG_GRANT_READ_URI_PERMISSION
            | Intent.FLAG_GRANT_WRITE_URI_PERMISSION;

    public static Intent newIntent(Context context, boolean skipInstructions) {
        Intent intent = new Intent(context, DirectorySelectionActivity.class);
        intent.putExtra(ARG_SKIP_INSTRUCTIONS, skipInstructions);
        return intent;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_directory_selection);

        Button button = findViewById(R.id.directory_selector_button);
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                openDirectorySelector();
            }
        });

        if (savedInstanceState == null && getIntent().getBooleanExtra(ARG_SKIP_INSTRUCTIONS, false)) {
            openDirectorySelector();
        }
    }

    private void openDirectorySelector() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        intent.addFlags(
                RW_FLAGS_PERMISSION
                        | Intent.FLAG_GRANT_PREFIX_URI_PERMISSION
                        | Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION
        );
        intent.putExtra("android.content.extra.SHOW_ADVANCED", true);
        intent.putExtra("android.content.extra.FANCY", true);
        intent.putExtra("android.content.extra.SHOW_FILESIZE", true);
        startActivityForResult(intent, RC_GET_EXTERNAL_DIR);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        if (requestCode == RC_GET_EXTERNAL_DIR) {
            if (resultCode == RESULT_OK && data != null && data.getData() != null) {
                getContentResolver().takePersistableUriPermission(
                        data.getData(),
                        data.getFlags() & RW_FLAGS_PERMISSION
                );
                FileManager.setBaseUri(data.getData());
                Intent result = new Intent();
                result.setData(data.getData());
                setResult(RESULT_OK, result);
                finish();
            }
        } else {
            super.onActivityResult(requestCode, resultCode, data);
        }
    }
}
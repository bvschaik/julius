package com.github.Keriew.augustus;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.widget.Button;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;

public class DirectorySelectionActivity extends AppCompatActivity {
    private static final String ARG_SKIP_INSTRUCTIONS = "arg_skip_instructions";
    private static final int RW_FLAGS_PERMISSION = Intent.FLAG_GRANT_READ_URI_PERMISSION
            | Intent.FLAG_GRANT_WRITE_URI_PERMISSION;

    public static Intent newIntent(Context context, boolean skipInstructions) {
        Intent intent = new Intent(context, DirectorySelectionActivity.class);
        intent.putExtra(ARG_SKIP_INSTRUCTIONS, skipInstructions);
        return intent;
    }

    private final ActivityResultLauncher<Uri> directorySelectionLauncher = registerForActivityResult(
            new ActivityResultContracts.OpenDocumentTree() {
                @Override
                @NonNull
                public Intent createIntent(@NonNull Context context, Uri input) {
                    Intent intent = super.createIntent(context, input);
                    intent.addFlags(RW_FLAGS_PERMISSION |
                            Intent.FLAG_GRANT_PREFIX_URI_PERMISSION | Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION);
                    intent.putExtra("android.content.extra.SHOW_ADVANCED", true);
                    intent.putExtra("android.content.extra.FANCY", true);
                    intent.putExtra("android.content.extra.SHOW_FILESIZE", true);
                    return intent;
                }
            },
            uri -> {
                if (uri != null) {
                    getContentResolver().takePersistableUriPermission(uri, RW_FLAGS_PERMISSION);
                    Intent result = new Intent();
                    result.setData(uri);
                    setResult(RESULT_OK, result);
                    finish();
                }
            });

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_directory_selection);

        Button button = findViewById(R.id.directory_selector_button);
        button.setOnClickListener(v -> directorySelectionLauncher.launch(Uri.EMPTY));

        if (savedInstanceState == null && getIntent().getBooleanExtra(ARG_SKIP_INSTRUCTIONS, false)) {
            directorySelectionLauncher.launch(Uri.EMPTY);
        }
    }
}

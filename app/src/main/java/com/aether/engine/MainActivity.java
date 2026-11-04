package com.aether.engine;

import android.app.Activity;
import android.content.Intent;
import android.content.res.AssetManager;
import android.graphics.Color;
import android.graphics.Typeface;
import android.graphics.drawable.GradientDrawable;
import android.net.Uri;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.view.Gravity;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * AETHER is a game player, not an in-app editor.
 * The game shipped under app/src/main/assets/game is mounted and launched automatically.
 */
public final class MainActivity extends Activity {
    private static final int REQUEST_EXPORT_LOGS = 41;
    private static final String ASSET_GAME_VERSION = BuildConfig.VERSION_NAME;

    private GLSurfaceView surface;
    private TextView debugStats;
    private final Handler handler = new Handler();
    private File gameRoot;
    private boolean statsVisible = true;
    private boolean gridVisible = false;

    private final Runnable statsRefresh = new Runnable() {
        @Override public void run() {
            if (BuildConfig.AETHER_DEBUG_TOOLS && statsVisible && debugStats != null) {
                debugStats.setText(nativeGetStats());
                handler.postDelayed(this, 500);
            }
        }
    };

    static { System.loadLibrary("aether"); }

    private native void nativeOnSurfaceCreated(AssetManager assets, String projectRoot);
    private native void nativeOnSurfaceChanged(int width, int height);
    private native void nativeOnDrawFrame();
    private native void nativeSetDebug(boolean enabled);
    private native String nativeGetStats();

    @Override protected void onCreate(Bundle state) {
        super.onCreate(state);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        getWindow().setStatusBarColor(Color.BLACK);
        getWindow().setNavigationBarColor(Color.BLACK);

        try {
            gameRoot = preparePackagedGame();
        } catch (Exception error) {
            Toast.makeText(this, "AETHER game assets error: " + error.getMessage(),
                Toast.LENGTH_LONG).show();
            finish();
            return;
        }

        FrameLayout root = new FrameLayout(this);
        surface = new GLSurfaceView(this);
        surface.setEGLContextClientVersion(3);
        surface.setPreserveEGLContextOnPause(true);
        surface.setEGLConfigChooser(8, 8, 8, 0, 16, 0);
        surface.setRenderer(new GLSurfaceView.Renderer() {
            @Override public void onSurfaceCreated(GL10 gl, EGLConfig config) {
                nativeOnSurfaceCreated(getAssets(), gameRoot.getAbsolutePath());
            }
            @Override public void onSurfaceChanged(GL10 gl, int width, int height) {
                nativeOnSurfaceChanged(width, height);
            }
            @Override public void onDrawFrame(GL10 gl) {
                nativeOnDrawFrame();
            }
        });
        surface.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        root.addView(surface, new FrameLayout.LayoutParams(-1, -1));

        // Release is a clean fullscreen player. Diagnostics exist only in Debug builds.
        if (BuildConfig.AETHER_DEBUG_TOOLS) root.addView(buildDebugTools(), debugLayout());

        setContentView(root);
        hideSystemUi();
        if (BuildConfig.AETHER_DEBUG_TOOLS) handler.post(statsRefresh);
    }

    private File preparePackagedGame() throws Exception {
        File root = new File(getFilesDir(), "AetherRuntime/Game");
        File versionFile = new File(root, ".aether_asset_version");
        String installedVersion = "";
        if (versionFile.isFile()) {
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(
                    new FileInputStream(versionFile), "UTF-8"))) {
                installedVersion = reader.readLine();
            }
        }

        // The APK's assets/game directory is the source of truth. On app updates,
        // refresh the private runtime copy so stale projects from older builds cannot run.
        if (!new File(root, "project.json").isFile()
                || !ASSET_GAME_VERSION.equals(installedVersion)) {
            deleteRecursively(root);
            if (!root.mkdirs() && !root.isDirectory())
                throw new IllegalStateException("Cannot create game workspace");
            copyAssetTree("game", root);
            try (FileOutputStream marker = new FileOutputStream(versionFile)) {
                marker.write(ASSET_GAME_VERSION.getBytes(java.nio.charset.StandardCharsets.UTF_8));
            }
        }

        File manifest = new File(root, "project.json");
        if (!manifest.isFile()) throw new IllegalStateException(
            "assets/game/project.json is missing from this APK");

        installBundledExtension(root);
        prepareNativeModules(root);
        return root;
    }

    private void copyAssetTree(String assetPath, File destination) throws Exception {
        String[] children = getAssets().list(assetPath);
        if (children != null && children.length > 0) {
            if (!destination.exists() && !destination.mkdirs())
                throw new IllegalStateException("Cannot create " + destination.getName());
            for (String child : children)
                copyAssetTree(assetPath + "/" + child, new File(destination, child));
            return;
        }
        File parent = destination.getParentFile();
        if (parent != null && !parent.exists() && !parent.mkdirs())
            throw new IllegalStateException("Cannot create asset directory");
        try (InputStream in = getAssets().open(assetPath);
             OutputStream out = new FileOutputStream(destination)) {
            byte[] buffer = new byte[16384];
            int count;
            while ((count = in.read(buffer)) != -1) out.write(buffer, 0, count);
        }
    }

    private void installBundledExtension(File projectRoot) {
        File source = new File(getApplicationInfo().nativeLibraryDir,
            "libaether_sample_extension.so");
        File folder = new File(projectRoot, "extensions/exe");
        File destination = new File(folder, "libaether_sample_extension.so");
        if (destination.isFile() || !source.isFile()) return;
        folder.mkdirs();
        try (InputStream in = new FileInputStream(source);
             OutputStream out = new FileOutputStream(destination)) {
            byte[] buffer = new byte[8192];
            int count;
            while ((count = in.read(buffer)) != -1) out.write(buffer, 0, count);
        } catch (Exception error) {
            android.util.Log.e("AETHER", "Could not stage bundled native extension", error);
        }
    }

    private void prepareNativeModules(File folder) {
        File[] children = folder.listFiles();
        if (children == null) return;
        for (File child : children) {
            if (child.isDirectory()) prepareNativeModules(child);
            else if (child.getName().endsWith(".so")) {
                child.setReadable(true, false);
                child.setWritable(false, false);
            }
        }
    }

    private View buildDebugTools() {
        LinearLayout panel = new LinearLayout(this);
        panel.setOrientation(LinearLayout.VERTICAL);
        panel.setPadding(dp(10), dp(8), dp(10), dp(8));
        panel.setBackground(panelBackground(0xE610182B, 0x887A91D9, 12));

        LinearLayout heading = new LinearLayout(this);
        heading.setGravity(Gravity.CENTER_VERTICAL);
        TextView brand = text("AETHER  /  DEBUG RUNTIME", 12, Color.WHITE, true);
        brand.setTypeface(Typeface.create("sans-serif-black", Typeface.BOLD));
        heading.addView(brand, new LinearLayout.LayoutParams(0, -2, 1));
        TextView build = text("DEV", 9, 0xFF78E0C1, true);
        build.setTypeface(Typeface.MONOSPACE);
        heading.addView(build);
        panel.addView(heading);

        LinearLayout actions = new LinearLayout(this);
        actions.setGravity(Gravity.CENTER_VERTICAL);
        actions.addView(actionButton("METRICS", () -> {
            statsVisible = !statsVisible;
            debugStats.setVisibility(statsVisible ? View.VISIBLE : View.GONE);
            handler.removeCallbacks(statsRefresh);
            if (statsVisible) handler.post(statsRefresh);
        }));
        actions.addView(actionButton("GRID", () -> {
            gridVisible = !gridVisible;
            surface.queueEvent(() -> nativeSetDebug(gridVisible));
        }));
        actions.addView(actionButton("EXPORT LOG", this::chooseLogExport));
        panel.addView(actions);

        debugStats = text("Starting runtime diagnostics...", 10, 0xFFB8C9E8, false);
        debugStats.setTypeface(Typeface.MONOSPACE);
        debugStats.setPadding(0, dp(5), 0, 0);
        panel.addView(debugStats);
        return panel;
    }

    private Button actionButton(String label, Runnable action) {
        Button button = new Button(this);
        button.setText(label);
        button.setTextSize(9);
        button.setTextColor(Color.WHITE);
        button.setTypeface(Typeface.create("sans-serif-medium", Typeface.BOLD));
        button.setAllCaps(false);
        button.setMinHeight(dp(34));
        button.setMinimumHeight(dp(34));
        button.setPadding(dp(9), 0, dp(9), 0);
        button.setBackground(panelBackground(0xFF202D49, 0x667F9BDF, 8));
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(-2, dp(34));
        params.setMargins(0, dp(5), dp(6), 0);
        button.setLayoutParams(params);
        button.setOnClickListener(view -> action.run());
        return button;
    }

    private TextView text(String value, float size, int color, boolean bold) {
        TextView view = new TextView(this);
        view.setText(value);
        view.setTextSize(size);
        view.setTextColor(color);
        if (bold) view.setTypeface(Typeface.create("sans-serif-medium", Typeface.BOLD));
        return view;
    }

    private GradientDrawable panelBackground(int fill, int stroke, int radius) {
        GradientDrawable bg = new GradientDrawable();
        bg.setColor(fill);
        bg.setCornerRadius(dp(radius));
        bg.setStroke(dp(1), stroke);
        return bg;
    }

    private FrameLayout.LayoutParams debugLayout() {
        FrameLayout.LayoutParams params =
            new FrameLayout.LayoutParams(-1, -2, Gravity.TOP);
        params.setMargins(dp(8), dp(8), dp(8), 0);
        return params;
    }

    private int dp(float value) {
        return Math.round(value * getResources().getDisplayMetrics().density);
    }

    private void chooseLogExport() {
        if (!BuildConfig.AETHER_DEBUG_TOOLS) return;
        Intent intent = new Intent(Intent.ACTION_CREATE_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("text/plain");
        intent.putExtra(Intent.EXTRA_TITLE,
            "aether-debug-" + System.currentTimeMillis() + ".log");
        startActivityForResult(intent, REQUEST_EXPORT_LOGS);
    }

    private void exportLogs(Uri uri) {
        try {
            Process process = Runtime.getRuntime().exec(
                new String[]{"logcat", "-d", "-v", "time", "-t", "5000"});
            StringBuilder log = new StringBuilder();
            try (BufferedReader reader = new BufferedReader(
                    new InputStreamReader(process.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) log.append(line).append('\n');
            }
            process.waitFor();
            try (OutputStream out = getContentResolver().openOutputStream(uri)) {
                if (out == null) throw new IllegalStateException("Cannot create log file");
                out.write(log.toString().getBytes(java.nio.charset.StandardCharsets.UTF_8));
            }
            Toast.makeText(this, "AETHER debug log exported", Toast.LENGTH_LONG).show();
        } catch (Exception error) {
            Toast.makeText(this, "Log export failed: " + error.getMessage(),
                Toast.LENGTH_LONG).show();
        }
    }

    private void deleteRecursively(File file) {
        if (file == null || !file.exists()) return;
        File[] children = file.listFiles();
        if (children != null) for (File child : children) deleteRecursively(child);
        if (!file.delete()) android.util.Log.w("AETHER", "Could not delete " + file);
    }

    @Override protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQUEST_EXPORT_LOGS && resultCode == RESULT_OK
                && data != null && data.getData() != null) exportLogs(data.getData());
    }

    private void hideSystemUi() {
        getWindow().getDecorView().setSystemUiVisibility(
            View.SYSTEM_UI_FLAG_FULLSCREEN | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
            View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY | View.SYSTEM_UI_FLAG_LAYOUT_STABLE |
            View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
            View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN);
    }

    @Override protected void onPause() {
        handler.removeCallbacks(statsRefresh);
        if (surface != null) surface.onPause();
        super.onPause();
    }

    @Override protected void onResume() {
        super.onResume();
        if (surface != null) surface.onResume();
        hideSystemUi();
        if (BuildConfig.AETHER_DEBUG_TOOLS && statsVisible) handler.post(statsRefresh);
    }
}

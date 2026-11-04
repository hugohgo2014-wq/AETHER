package com.aether.engine;

import android.app.Activity;
import android.app.AlertDialog;
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
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.HorizontalScrollView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.util.Locale;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public final class MainActivity extends Activity {
    private static final int REQUEST_IMPORT_PROJECT = 40;
    private static final int REQUEST_EXPORT_LOGS = 41;
    private static final long MAX_IMPORT_BYTES = 160L * 1024L * 1024L;
    private GLSurfaceView surface;
    private TextView projectLabel;
    private TextView statusLabel;
    private TextView debugPanel;
    private final Handler handler = new Handler();
    private File activeProject;
    private boolean debugOpen = false;
    private final Runnable statsRefresh = new Runnable() {
        @Override public void run() {
            if (BuildConfig.AETHER_DEBUG_TOOLS && debugOpen && debugPanel != null) {
                debugPanel.setText(nativeGetStats());
                handler.postDelayed(this, 650);
            }
        }
    };

    static { System.loadLibrary("aether"); }
    private native void nativeOnSurfaceCreated(AssetManager assets, String projectRoot);
    private native void nativeOnSurfaceChanged(int width, int height);
    private native void nativeOnDrawFrame();
    private native void nativeLoadProject(String projectRoot);
    private native void nativeSetDebug(boolean enabled);
    private native void nativeAddPrimitive(String mesh);
    private native String nativeGetStats();

    @Override protected void onCreate(Bundle state) {
        super.onCreate(state);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        getWindow().setStatusBarColor(Color.rgb(7, 11, 20));
        getWindow().setNavigationBarColor(Color.rgb(7, 11, 20));
        activeProject = createStarterProject();

        FrameLayout root = new FrameLayout(this);
        surface = new GLSurfaceView(this);
        surface.setEGLContextClientVersion(3);
        surface.setPreserveEGLContextOnPause(true);
        surface.setEGLConfigChooser(8, 8, 8, 0, 16, 0);
        surface.setRenderer(new GLSurfaceView.Renderer() {
            @Override public void onSurfaceCreated(GL10 gl, EGLConfig config) {
                nativeOnSurfaceCreated(getAssets(), activeProject.getAbsolutePath());
            }
            @Override public void onSurfaceChanged(GL10 gl, int width, int height) {
                nativeOnSurfaceChanged(width, height);
            }
            @Override public void onDrawFrame(GL10 gl) { nativeOnDrawFrame(); }
        });
        surface.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        root.addView(surface, new FrameLayout.LayoutParams(-1, -1));
        root.addView(buildTopPanel(), topLayout());
        root.addView(buildBottomPanel(), bottomLayout());
        setContentView(root);
        updateProjectLabel();
        hideSystemUi();
    }

    private View buildTopPanel() {
        LinearLayout panel = new LinearLayout(this);
        panel.setOrientation(LinearLayout.VERTICAL);
        panel.setPadding(dp(12), dp(9), dp(12), dp(9));
        panel.setBackground(panelBackground(0xE610182B, 0x557A91D9, 14));
        LinearLayout heading = new LinearLayout(this);
        heading.setGravity(Gravity.CENTER_VERTICAL);
        TextView brand = text("A E T H E R", 15, Color.WHITE, true);
        brand.setTypeface(Typeface.create("sans-serif-black", Typeface.BOLD));
        heading.addView(brand, new LinearLayout.LayoutParams(-2, -2));
        TextView mode = text("  NATIVE 3D EDITOR  /  GLES 3.0", 9, 0xFF9FB2D8, false);
        mode.setTypeface(Typeface.MONOSPACE);
        heading.addView(mode, new LinearLayout.LayoutParams(-2, -2));
        panel.addView(heading);
        projectLabel = text("Loading project...", 10, 0xFFE2E9FA, true);
        projectLabel.setPadding(0, dp(4), 0, dp(6));
        panel.addView(projectLabel);

        HorizontalScrollView scroll = new HorizontalScrollView(this);
        scroll.setHorizontalScrollBarEnabled(false);
        LinearLayout tools = new LinearLayout(this);
        tools.setGravity(Gravity.CENTER_VERTICAL);
        tools.addView(actionButton("NEW PROJECT", this::showCreateProject));
        tools.addView(actionButton("PROJECTS", this::showProjects));
        tools.addView(actionButton("OPEN ZIP", this::chooseProjectZip));
        tools.addView(actionButton("+ CUBE", () -> addPrimitive("cube")));
        tools.addView(actionButton("+ SPHERE", () -> addPrimitive("sphere")));
        if (BuildConfig.AETHER_DEBUG_TOOLS) {
            tools.addView(actionButton("DEBUG", this::toggleDebug));
            tools.addView(actionButton("EXPORT LOG", this::chooseLogExport));
        }
        scroll.addView(tools);
        panel.addView(scroll);
        return panel;
    }

    private View buildBottomPanel() {
        LinearLayout panel = new LinearLayout(this);
        panel.setOrientation(LinearLayout.VERTICAL);
        panel.setPadding(dp(12), dp(8), dp(12), dp(8));
        panel.setBackground(panelBackground(0xD90C1427, 0x447A91D9, 12));
        statusLabel = text("PROJECT WORKSPACE  •  C17 CORE  •  C++ RUNTIME", 9, 0xFFD7E3FF, false);
        statusLabel.setTypeface(Typeface.MONOSPACE);
        panel.addView(statusLabel);
        if (BuildConfig.AETHER_DEBUG_TOOLS) {
            debugPanel = text("DEBUG TOOLS READY", 9, 0xFF78E0C1, false);
            debugPanel.setTypeface(Typeface.MONOSPACE);
            debugPanel.setVisibility(View.GONE);
            debugPanel.setPadding(0, dp(5), 0, 0);
            panel.addView(debugPanel);
        }
        return panel;
    }

    private Button actionButton(String label, Runnable action) {
        Button button = new Button(this);
        button.setText(label);
        button.setTextSize(10);
        button.setTextColor(Color.WHITE);
        button.setTypeface(Typeface.create("sans-serif-medium", Typeface.BOLD));
        button.setAllCaps(false);
        button.setMinHeight(dp(38));
        button.setMinimumHeight(dp(38));
        button.setPadding(dp(12), 0, dp(12), 0);
        button.setBackground(panelBackground(0xFF202D49, 0x667F9BDF, 9));
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(-2, dp(38));
        params.setMargins(0, 0, dp(7), 0);
        button.setLayoutParams(params);
        button.setOnClickListener(v -> action.run());
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

    private FrameLayout.LayoutParams topLayout() {
        FrameLayout.LayoutParams p = new FrameLayout.LayoutParams(-1, -2, Gravity.TOP);
        p.setMargins(dp(10), dp(8), dp(10), 0);
        return p;
    }

    private FrameLayout.LayoutParams bottomLayout() {
        FrameLayout.LayoutParams p = new FrameLayout.LayoutParams(-1, -2, Gravity.BOTTOM);
        p.setMargins(dp(10), 0, dp(10), dp(8));
        return p;
    }

    private int dp(float value) {
        return Math.round(value * getResources().getDisplayMetrics().density);
    }

    private File createStarterProject() {
        File root = new File(getFilesDir(), "AetherProjects/Starter");
        if (!new File(root, "project.json").isFile()) {
            root.mkdirs();
            try { copyAssetTree("game", root); }
            catch (Exception e) { Toast.makeText(this, "Starter project error: " + e.getMessage(), Toast.LENGTH_LONG).show(); }
        }
        installBundledExtension(root);
        return root;
    }

    private void copyAssetTree(String assetPath, File destination) throws Exception {
        String[] children = getAssets().list(assetPath);
        if (children != null && children.length > 0) {
            destination.mkdirs();
            for (String child : children) copyAssetTree(assetPath + "/" + child, new File(destination, child));
            return;
        }
        destination.getParentFile().mkdirs();
        try (InputStream in = getAssets().open(assetPath); OutputStream out = new FileOutputStream(destination)) {
            byte[] buffer = new byte[8192];
            int count;
            while ((count = in.read(buffer)) != -1) out.write(buffer, 0, count);
        }
    }

    private void installBundledExtension(File projectRoot) {
        File source = new File(getApplicationInfo().nativeLibraryDir, "libaether_sample_extension.so");
        File folder = new File(projectRoot, "extensions/exe");
        File destination = new File(folder, "libaether_sample_extension.so");
        if (destination.isFile() || !source.isFile()) return;
        folder.mkdirs();
        try (InputStream in = new FileInputStream(source); OutputStream out = new FileOutputStream(destination)) {
            byte[] buffer = new byte[8192];
            int count;
            while ((count = in.read(buffer)) != -1) out.write(buffer, 0, count);
            destination.setReadable(true, false);
            destination.setWritable(false, false);
        } catch (Exception e) {
            android.util.Log.e("AETHER", "Could not stage sample native extension", e);
        }
    }

    private void showCreateProject() {
        EditText name = new EditText(this);
        name.setSingleLine(true);
        name.setHint("My 3D Game");
        new AlertDialog.Builder(this)
            .setTitle("Create project")
            .setMessage("A new editable AETHER project will be created in your private workspace.")
            .setView(name)
            .setNegativeButton("CANCEL", null)
            .setPositiveButton("CREATE", (dialog, which) -> {
                String projectName = name.getText().toString().trim();
                if (projectName.isEmpty()) projectName = "Untitled Game";
                try {
                    File root = new File(getFilesDir(), "AetherProjects/" + safeFolderName(projectName));
                    if (root.exists() && new File(root, "project.json").isFile()) {
                        Toast.makeText(this, "Project already exists; opening it", Toast.LENGTH_SHORT).show();
                    } else {
                        createProjectFiles(root, projectName);
                    }
                    openProject(root);
                } catch (Exception e) {
                    Toast.makeText(this, "Create failed: " + e.getMessage(), Toast.LENGTH_LONG).show();
                }
            }).show();
    }

    private String safeFolderName(String input) {
        String value = input.replaceAll("[^A-Za-z0-9 _-]", "").trim().replace(' ', '_');
        return value.isEmpty() ? "Untitled_Game" : value;
    }

    private void createProjectFiles(File root, String name) throws Exception {
        new File(root, "scenes").mkdirs();
        new File(root, "extensions/scripts").mkdirs();
        new File(root, "extensions/exe").mkdirs();
        JSONObject project = new JSONObject();
        project.put("format", "aether.project");
        project.put("version", 1);
        project.put("name", name);
        project.put("startScene", "scenes/main.scene.json");
        JSONObject extensions = new JSONObject();
        extensions.put("scripts", new org.json.JSONArray());
        extensions.put("nativeLibraries", new org.json.JSONArray());
        project.put("extensions", extensions);
        writeText(new File(root, "project.json"), project.toString(2));
        String scene = "{\n" +
            "  \"format\": \"aether.scene\",\n" +
            "  \"version\": 1,\n" +
            "  \"environment\": {\"clearColor\": [0.025, 0.035, 0.065, 1.0], \"grid\": true},\n" +
            "  \"entities\": [\n" +
            "    {\"name\":\"Ground\",\"mesh\":\"plane\",\"transform\":{\"position\":[0,-1,0],\"rotation\":[0,0,0],\"scale\":[8,1,8]},\"material\":{\"color\":[0.11,0.15,0.22,1]}},\n" +
            "    {\"name\":\"Player Cube\",\"mesh\":\"cube\",\"transform\":{\"position\":[-1,0,0],\"rotation\":[0,25,0],\"scale\":[1,1,1]},\"material\":{\"color\":[0.30,0.58,0.98,1]}},\n" +
            "    {\"name\":\"Sphere\",\"mesh\":\"sphere\",\"transform\":{\"position\":[1,0,0],\"rotation\":[0,0,0],\"scale\":[1.2,1.2,1.2]},\"material\":{\"color\":[0.18,0.78,0.70,1]}}\n" +
            "  ]\n" +
            "}\n";
        writeText(new File(root, "scenes/main.scene.json"), scene);
        writeText(new File(root, "extensions/exe/README.md"),
            "Place Android NDK native modules (.so) here. Use extensions.nativeLibraries in project.json. " +
            "Build for the device ABI (arm64-v8a or armeabi-v7a) and implement aether_plugin_query. " +
            "This folder is for compiled native extensions, not Windows .exe files.\n");
        writeText(new File(root, "extensions/scripts/README.md"), "Project Lua scripts go here.\n");
    }

    private void writeText(File file, String value) throws Exception {
        file.getParentFile().mkdirs();
        try (FileOutputStream out = new FileOutputStream(file)) {
            out.write(value.getBytes(java.nio.charset.StandardCharsets.UTF_8));
        }
    }

    private void showProjects() {
        File parent = new File(getFilesDir(), "AetherProjects");
        File[] children = parent.listFiles(File::isDirectory);
        java.util.ArrayList<File> projects = new java.util.ArrayList<>();
        if (children != null) for (File child : children)
            if (new File(child, "project.json").isFile()) projects.add(child);
        if (projects.isEmpty()) {
            Toast.makeText(this, "No saved projects yet", Toast.LENGTH_SHORT).show();
            return;
        }
        String[] names = new String[projects.size()];
        for (int i = 0; i < names.length; i++) names[i] = projects.get(i).getName();
        new AlertDialog.Builder(this).setTitle("My AETHER projects")
            .setItems(names, (dialog, which) -> openProject(projects.get(which))).show();
    }

    private void chooseProjectZip() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("application/zip");
        startActivityForResult(intent, REQUEST_IMPORT_PROJECT);
    }

    private void importProjectZip(Uri uri) {
        File destination = new File(getFilesDir(), "AetherProjects/Imported_" + System.currentTimeMillis());
        long total = 0;
        int entries = 0;
        try (InputStream raw = getContentResolver().openInputStream(uri);
             ZipInputStream zip = new ZipInputStream(raw)) {
            if (raw == null) throw new IllegalArgumentException("Could not open ZIP");
            byte[] buffer = new byte[8192];
            ZipEntry entry;
            while ((entry = zip.getNextEntry()) != null) {
                String name = entry.getName().replace('\\', '/');
                if (name.startsWith("/") || name.contains("../") || name.equals("..")) throw new IllegalArgumentException("Unsafe ZIP path");
                File target = new File(destination, name);
                String base = destination.getCanonicalPath() + File.separator;
                if (!target.getCanonicalPath().startsWith(base)) throw new IllegalArgumentException("Unsafe ZIP entry");
                if (entry.isDirectory()) { target.mkdirs(); continue; }
                target.getParentFile().mkdirs();
                try (FileOutputStream out = new FileOutputStream(target)) {
                    int count;
                    while ((count = zip.read(buffer)) != -1) {
                        total += count;
                        if (total > MAX_IMPORT_BYTES) throw new IllegalArgumentException("Project ZIP exceeds 160 MB");
                        out.write(buffer, 0, count);
                    }
                }
                if (++entries > 20000) throw new IllegalArgumentException("Project ZIP contains too many files");
                zip.closeEntry();
            }
            File projectRoot = findProjectRoot(destination);
            if (projectRoot == null) throw new IllegalArgumentException("ZIP must contain project.json at its root or one folder below");
            openProject(projectRoot);
        } catch (Exception e) {
            deleteRecursively(destination);
            Toast.makeText(this, "Import failed: " + e.getMessage(), Toast.LENGTH_LONG).show();
        }
    }

    private File findProjectRoot(File root) {
        if (new File(root, "project.json").isFile()) return root;
        File[] children = root.listFiles(File::isDirectory);
        if (children != null) for (File child : children)
            if (new File(child, "project.json").isFile()) return child;
        return null;
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

    private void openProject(File root) {
        if (!new File(root, "project.json").isFile()) {
            Toast.makeText(this, "project.json not found", Toast.LENGTH_LONG).show();
            return;
        }
        prepareNativeModules(root);
        activeProject = root;
        if (surface != null) surface.queueEvent(() -> nativeLoadProject(activeProject.getAbsolutePath()));
        updateProjectLabel();
        Toast.makeText(this, "Opened: " + root.getName(), Toast.LENGTH_SHORT).show();
    }

    private void addPrimitive(String mesh) {
        if (surface != null) surface.queueEvent(() -> nativeAddPrimitive(mesh));
        Toast.makeText(this, mesh.toUpperCase(Locale.ROOT) + " added to scene", Toast.LENGTH_SHORT).show();
    }

    private void updateProjectLabel() {
        if (projectLabel != null && activeProject != null) {
            projectLabel.setText("PROJECT  /  " + activeProject.getName() + "     •     " + activeProject.getAbsolutePath());
        }
    }

    private void toggleDebug() {
        if (!BuildConfig.AETHER_DEBUG_TOOLS) return;
        debugOpen = !debugOpen;
        if (debugPanel != null) debugPanel.setVisibility(debugOpen ? View.VISIBLE : View.GONE);
        if (surface != null) surface.queueEvent(() -> nativeSetDebug(debugOpen));
        handler.removeCallbacks(statsRefresh);
        if (debugOpen) handler.post(statsRefresh);
        Toast.makeText(this, debugOpen ? "Debug tools enabled" : "Debug tools hidden", Toast.LENGTH_SHORT).show();
    }

    private void chooseLogExport() {
        if (!BuildConfig.AETHER_DEBUG_TOOLS) return;
        Intent intent = new Intent(Intent.ACTION_CREATE_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("text/plain");
        intent.putExtra(Intent.EXTRA_TITLE, "aether-debug-" + System.currentTimeMillis() + ".log");
        startActivityForResult(intent, REQUEST_EXPORT_LOGS);
    }

    private void exportLogs(Uri uri) {
        try {
            Process process = Runtime.getRuntime().exec(new String[]{"logcat", "-d", "-v", "time", "-s", "AETHER"});
            StringBuilder log = new StringBuilder();
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) log.append(line).append('\n');
            }
            process.waitFor();
            try (OutputStream out = getContentResolver().openOutputStream(uri)) {
                if (out == null) throw new IllegalStateException("Could not create log file");
                out.write(log.toString().getBytes(java.nio.charset.StandardCharsets.UTF_8));
            }
            Toast.makeText(this, "Debug log exported", Toast.LENGTH_LONG).show();
        } catch (Exception e) {
            Toast.makeText(this, "Log export failed: " + e.getMessage(), Toast.LENGTH_LONG).show();
        }
    }

    private void deleteRecursively(File file) {
        if (file == null || !file.exists()) return;
        File[] children = file.listFiles();
        if (children != null) for (File child : children) deleteRecursively(child);
        file.delete();
    }

    @Override protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (resultCode != RESULT_OK || data == null || data.getData() == null) return;
        if (requestCode == REQUEST_IMPORT_PROJECT) importProjectZip(data.getData());
        else if (requestCode == REQUEST_EXPORT_LOGS) exportLogs(data.getData());
    }

    private void hideSystemUi() {
        getWindow().getDecorView().setSystemUiVisibility(
            View.SYSTEM_UI_FLAG_FULLSCREEN | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
            View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY | View.SYSTEM_UI_FLAG_LAYOUT_STABLE |
            View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN);
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
        if (BuildConfig.AETHER_DEBUG_TOOLS && debugOpen) handler.post(statsRefresh);
    }
}

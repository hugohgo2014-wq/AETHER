package com.aether.engine;
import android.app.Activity;
import android.graphics.Color;
import android.graphics.Typeface;
import android.graphics.drawable.GradientDrawable;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.Gravity;
import android.view.View;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.TextView;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
public final class MainActivity extends Activity {
    static { System.loadLibrary("aether"); }
    private GLSurfaceView surface;
    private native void nativeOnSurfaceCreated(android.content.res.AssetManager assets);
    private native void nativeOnSurfaceChanged(int width,int height);
    private native void nativeOnDrawFrame();
    @Override protected void onCreate(Bundle state) {
        super.onCreate(state);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        getWindow().setStatusBarColor(Color.rgb(7,11,20));
        getWindow().setNavigationBarColor(Color.rgb(7,11,20));
        FrameLayout root=new FrameLayout(this);
        surface=new GLSurfaceView(this);
        surface.setEGLContextClientVersion(3);
        surface.setPreserveEGLContextOnPause(true);
        surface.setEGLConfigChooser(8,8,8,0,16,0);
        surface.setRenderer(new GLSurfaceView.Renderer() {
            @Override public void onSurfaceCreated(GL10 gl,EGLConfig config){nativeOnSurfaceCreated(getAssets());}
            @Override public void onSurfaceChanged(GL10 gl,int w,int h){nativeOnSurfaceChanged(w,h);}
            @Override public void onDrawFrame(GL10 gl){nativeOnDrawFrame();}
        });
        surface.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        root.addView(surface,new FrameLayout.LayoutParams(-1,-1));
        root.addView(buildBrandPanel(),brandLayout());
        root.addView(buildFooter(),footerLayout());
        setContentView(root);hideSystemUi();
    }
    private View buildBrandPanel() {
        LinearLayout panel=new LinearLayout(this);panel.setOrientation(LinearLayout.VERTICAL);
        panel.setPadding(dp(16),dp(10),dp(18),dp(10));
        GradientDrawable bg=new GradientDrawable();bg.setColor(0xB90C1428);bg.setCornerRadius(dp(14));bg.setStroke(dp(1),0x557B8CFF);panel.setBackground(bg);
        TextView title=new TextView(this);title.setText("A E T H E R");title.setTextColor(Color.WHITE);title.setTextSize(13);title.setTypeface(Typeface.create("sans-serif-medium",Typeface.BOLD));
        TextView subtitle=new TextView(this);subtitle.setText("NATIVE 3D RUNTIME  /  GLES 3.0");subtitle.setTextColor(0xFF9EAFD5);subtitle.setTextSize(9);subtitle.setTypeface(Typeface.MONOSPACE);
        panel.addView(title);panel.addView(subtitle);return panel;
    }
    private View buildFooter() {
        TextView label=new TextView(this);label.setText("PROCEDURAL WORLD  •  C17 CORE  •  C++20 SYSTEMS");
        label.setTextColor(0xFFD7E3FF);label.setTextSize(9);label.setTypeface(Typeface.MONOSPACE);label.setPadding(dp(13),dp(9),dp(13),dp(9));
        GradientDrawable bg=new GradientDrawable();bg.setColor(0xA90C1428);bg.setCornerRadius(dp(12));bg.setStroke(dp(1),0x447B8CFF);label.setBackground(bg);return label;
    }
    private FrameLayout.LayoutParams brandLayout(){FrameLayout.LayoutParams p=new FrameLayout.LayoutParams(-2,-2,Gravity.TOP|Gravity.START);p.setMargins(dp(18),dp(18),0,0);return p;}
    private FrameLayout.LayoutParams footerLayout(){FrameLayout.LayoutParams p=new FrameLayout.LayoutParams(-2,-2,Gravity.BOTTOM|Gravity.CENTER_HORIZONTAL);p.bottomMargin=dp(16);return p;}
    private int dp(float v){return Math.round(v*getResources().getDisplayMetrics().density);}
    private void hideSystemUi(){getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_FULLSCREEN|View.SYSTEM_UI_FLAG_HIDE_NAVIGATION|View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY|View.SYSTEM_UI_FLAG_LAYOUT_STABLE|View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION|View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN);}
    @Override protected void onPause(){if(surface!=null)surface.onPause();super.onPause();}
    @Override protected void onResume(){super.onResume();if(surface!=null)surface.onResume();hideSystemUi();}
}

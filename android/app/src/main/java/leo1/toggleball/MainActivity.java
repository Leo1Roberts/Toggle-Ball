package leo1.toggleball;

import android.view.View;
import com.google.androidgamesdk.GameActivity;

public class MainActivity extends GameActivity {
    static {
        System.loadLibrary("toggleball");
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        // Automatically hide the UI when the user taps back into the game
        if (hasFocus) {
            setImmersiveMode(true);
        }
    }

    // This is the method your C++ AndroidWindow.cpp calls via JNI!
    @SuppressWarnings("deprecation")
    public void setImmersiveMode(final boolean fullscreen) {
        runOnUiThread(() -> {
            View decorView = getWindow().getDecorView();

            if (fullscreen) {
                // Your original, battle-tested fullscreen flags
                decorView.setSystemUiVisibility(
                        View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                                | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                                | View.SYSTEM_UI_FLAG_FULLSCREEN
                );
            } else {
                // When showing the UI, we keep the LAYOUT flags.
                // This prevents your OpenGL canvas from physically resizing/squishing
                // when the Android navigation bar slides up.
                decorView.setSystemUiVisibility(
                        View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                );
            }
        });
    }
}
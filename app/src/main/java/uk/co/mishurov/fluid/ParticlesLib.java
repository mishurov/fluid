package uk.co.mishurov.fluid;

import android.content.res.AssetManager;


public class ParticlesLib {
    static {
        System.loadLibrary( "particles" );
    }

    public static native void init();
    public static native void surface(int width, int height);
    public static native void touch(boolean is_down, float x, float y);
    public static native void step(float elapsedTime);
    public static native void rotate(int angle);
    public static native void settings(
        String fg_color, String bg_color, int iterations, int cursor_size
    );
    public static native void createAssetManager(AssetManager assetManager);
}

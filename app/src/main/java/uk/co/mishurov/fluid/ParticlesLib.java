package uk.co.mishurov.fluid;

import android.content.res.AssetManager;

public class ParticlesLib 
{

    // This is static so it will be called at startup (without creating an instance of this class)
     static 
     {
         System.loadLibrary( "particles" );
     }

    // Define native functions that can be called from Java code
     public static native void init(int width, int height);
     public static native void touch(float x, float y);
     public static native void step(float elapsedTime);
     public static native void createAssetManager(AssetManager assetManager);
}

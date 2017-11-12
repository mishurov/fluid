package uk.co.mishurov.fluid

import android.content.res.AssetManager


object ParticlesLib {
    init {
        System.loadLibrary("particles")
    }

    external fun init()
    external fun surface(width: Int, height: Int)
    external fun touch(is_down: Boolean, x: Float, y: Float)
    external fun step(elapsedTime: Float)
    external fun rotate(angle: Int)
    external fun settings(
            fg_color: String, bg_color: String, iterations: Int, cursor_size: Int
    )

    external fun createAssetManager(assetManager: AssetManager)
}

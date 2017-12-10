package uk.co.mishurov.fluid

import android.content.Context
import android.opengl.GLSurfaceView
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10
import android.app.Dialog
import android.view.LayoutInflater
import android.view.Window


internal class FluidView(context: Context) : GLSurfaceView(context)
{
    var mInitialized = false

    init {
        setPreserveEGLContextOnPause(true)
        setEGLContextClientVersion(2)
        setRenderer(FluidRenderer(this))
    }

    private inner class FluidRenderer(
                                    internal var mParentView: FluidView)
                        : GLSurfaceView.Renderer
    {
        override fun onDrawFrame(gl: GL10)
        {
            FluidLib.step(0.0f)
        }

        override fun onSurfaceChanged(gl: GL10, width: Int, height: Int)
        {
            if (!mInitialized) {
                FluidLib.surface(width, height)
                mInitialized = true
            }
        }

        override fun onSurfaceCreated(gl: GL10, config: EGLConfig)
        {
            //FluidLib.init()
        }
    }

    companion object
    {
        private val TAG = "Fluid FluidView"
    }
}


package uk.co.mishurov.fluid

import android.content.Context
import android.opengl.GLES20
import android.opengl.GLSurfaceView
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10
import android.os.SystemClock
import android.app.Dialog
import android.view.LayoutInflater
import android.view.Window
import android.view.View
import android.util.Log

import uk.co.mishurov.fluid.R


internal class ParticlesView(context: Context) : GLSurfaceView(context) {
    var mDialog: Dialog? = null
    var mDialogLayout: RotatingLayout? = null
    var mInitialized = false

    init {
        mDialog = Dialog(context)
        mDialog?.requestWindowFeature(Window.FEATURE_NO_TITLE)
        val inflater = LayoutInflater.from(context)
        mDialogLayout = inflater.inflate(R.layout.no_extension, null) as RotatingLayout
        mDialog?.setContentView(mDialogLayout)

        setPreserveEGLContextOnPause(true)
        setEGLContextClientVersion(2)
        setRenderer(ParticlesRenderer(this))
    }

    fun showAlert() {
        post(object : Runnable {
            override fun run() {
                mDialog?.show()
            }
        })
    }

    fun rotateDialog(angle: Int) {
        //mDialogLayout.angle = angle
        mDialogLayout?.setAngle(angle)
    }

    private inner class ParticlesRenderer(internal var mParentView: ParticlesView) : GLSurfaceView.Renderer {
        private val m_nLastTime: Long = 0

        override fun onDrawFrame(gl: GL10) {
            /*
            // calculate elapsed time
            if(m_nLastTime == 0)
                m_nLastTime = SystemClock.elapsedRealtime();

            long nCurrentTime = SystemClock.elapsedRealtime();
            long nElapsedTime = nCurrentTime - m_nLastTime;
            float fElapsedTime = nElapsedTime / 1000.0f;
            m_nLastTime = nCurrentTime;
            */
            ParticlesLib.step(0.0.toFloat())
        }

        override fun onSurfaceChanged(gl: GL10, width: Int, height: Int) {
            // refresh method
            if (!mInitialized) {
                ParticlesLib.surface(width, height)
                mInitialized = true
            }
        }

        override fun onSurfaceCreated(gl: GL10, config: EGLConfig) {
            // Alert if a device doesn't support floating point extension
            /*
            String extensions = GLES20.glGetString(GLES20.GL_EXTENSIONS);
            String texture_float = "GL_OES_texture_float";
            if (!extensions.toLowerCase().contains(texture_float.toLowerCase()))
                mParentView.showAlert();
            */
            //ParticlesLib.init();
        }
    }

    companion object {
        private val TAG = "Fluid"
    }
}


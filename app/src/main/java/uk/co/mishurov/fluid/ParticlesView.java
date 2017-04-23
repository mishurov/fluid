package uk.co.mishurov.fluid;

import android.content.Context;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import android.os.SystemClock;
import android.app.Dialog;
import android.view.LayoutInflater;
import android.view.Window;
import android.view.View;
import android.util.Log;

import uk.co.mishurov.fluid.R;


class ParticlesView extends GLSurfaceView {
    private static final String TAG = "Fluid";
    Dialog mDialog;
    RotatingLayout mDialogLayout;

    public ParticlesView(Context context) {
        super(context);
        mDialog = new Dialog(context);
        mDialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
        LayoutInflater inflater = LayoutInflater.from(context);
        mDialogLayout = (RotatingLayout) inflater.inflate(R.layout.no_extension, null);
        mDialog.setContentView(mDialogLayout);

        setEGLContextClientVersion(2);
        setRenderer(new ParticlesRenderer(this));
    }

    public void showAlert() {
        post(new Runnable() {
            public void run() {
                mDialog.show();
            }
        });
    }

    public void rotateDialog(int angle) {
        mDialogLayout.setAngle(angle);
    }

    private class ParticlesRenderer implements GLSurfaceView.Renderer {
        private long m_nLastTime;
        ParticlesView mParentView;

        public ParticlesRenderer(ParticlesView view) {
            mParentView = view;
        }

        public void onDrawFrame(GL10 gl) {
            // calculate elapsed time
            if(m_nLastTime == 0)
                m_nLastTime = SystemClock.elapsedRealtime();

            long nCurrentTime = SystemClock.elapsedRealtime();
            long nElapsedTime = nCurrentTime - m_nLastTime;
            float fElapsedTime = nElapsedTime / 1000.0f;
            m_nLastTime = nCurrentTime;

            ParticlesLib.step(fElapsedTime);
        }

        public void onSurfaceChanged( GL10 gl, int width, int height ) {
            // refresh method
            ParticlesLib.init(width, height);
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            // Alert if a device doesn't support floating point extension
            /*
            String extensions = GLES20.glGetString(GLES20.GL_EXTENSIONS);
            String texture_float = "GL_OES_texture_float";
            if (!extensions.toLowerCase().contains(texture_float.toLowerCase()))
                mParentView.showAlert();
            */
        }
    }
}


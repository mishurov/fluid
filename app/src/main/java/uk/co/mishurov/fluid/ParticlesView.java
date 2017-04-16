package uk.co.mishurov.fluid;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.view.View;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import android.os.SystemClock;
import android.util.Log;


class ParticlesView extends GLSurfaceView {
    public ParticlesView( Context context ) {
        super( context );

        //setEGLContextClientVersion( 2 );
        setEGLContextClientVersion( 2 );

        // Set the renderer associated with this view
        setRenderer( new ParticlesRenderer() );
    }

    private static class ParticlesRenderer implements GLSurfaceView.Renderer {
        private long m_nLastTime;
        boolean init;

        public ParticlesRenderer() {
            init = true;
        }

        public void onDrawFrame( GL10 gl ) {
            // calculate elapsed time
            if( m_nLastTime == 0 )
                m_nLastTime = SystemClock.elapsedRealtime();

            long nCurrentTime = SystemClock.elapsedRealtime();
            long nElapsedTime = nCurrentTime - m_nLastTime;
            float fElapsedTime = nElapsedTime / 1000.0f;
            m_nLastTime = nCurrentTime;

            ParticlesLib.step( fElapsedTime );
        }

        public void onSurfaceChanged( GL10 gl, int width, int height ) {
            // refresh method
            ParticlesLib.init( width, height );
        }

        public void onSurfaceCreated( GL10 gl, EGLConfig config )
        {
        }
    }
}


package uk.co.mishurov.fluid;

import android.app.Activity;
import android.content.Intent;
import android.content.res.AssetManager;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.util.Log;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.OrientationEventListener;
import android.widget.Toast;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.preference.PreferenceManager;
import android.view.Display;
import android.view.Surface;


public class ParticlesActivity extends Activity {
    private static final String TAG = "Fluid";
    private static final float GESTURE_TRESHOLD = 3;
    public static final String KEY_PREF_FG_COLOR = "pref_fg_color";
    public static final String KEY_PREF_BG_COLOR = "pref_bg_color";
    public static final String KEY_PREF_ITERATIONS = "pref_iterations";
    public static final String KEY_PREF_CURSOR_SIZE = "pref_cursor_size";


    SharedPreferences mPrefs;
    OnSharedPreferenceChangeListener mListener;
    GestureDetector mGestureDetector;

    static AssetManager sAssetManager;
    ParticlesView mView;
    OrientationEventListener mOrientationListener;
    float x1, y1;
    float x2, y2;
    int orientation;
    int mScreenRotation;

    // On applications creation
    @Override protected void onCreate(Bundle savedInstanceState) {
        super.onCreate( savedInstanceState );

        // Listen orientation
        orientation = 0;

        Display display = getWindowManager().getDefaultDisplay();
        int rotation = display.getRotation();
        switch (rotation) {
            case Surface.ROTATION_90:
                mScreenRotation = 90;
                break;
            case Surface.ROTATION_180:
                mScreenRotation = 180;
                break;
            case Surface.ROTATION_270:
                mScreenRotation = -90;
                break;
            default:
                mScreenRotation = 0;
                break;
        }

        mOrientationListener = new OrientationEventListener(this,
            SensorManager.SENSOR_DELAY_NORMAL) {
                @Override
                public void onOrientationChanged(int orientation) {
                    int r = ParticlesActivity.this.mScreenRotation;
                    ParticlesActivity.this.orientation = orientation + r;
                    ParticlesLib.rotate(orientation + r);
                }
            };

        if (mOrientationListener.canDetectOrientation() == true) {
            Log.v(TAG, "Can detect orientation");
            mOrientationListener.enable();
        } else {
            Log.v(TAG, "Cannot detect orientation");
            mOrientationListener.disable();
        }

        mGestureDetector = new GestureDetector(this, new GestureListener());

        // Read and listen preferences
        mPrefs = PreferenceManager.getDefaultSharedPreferences(this);
        int fgColorPref = mPrefs.getInt(KEY_PREF_FG_COLOR, 0);
        int bgColorPref = mPrefs.getInt(KEY_PREF_BG_COLOR, 0);
        int iterationsPref = mPrefs.getInt(KEY_PREF_ITERATIONS, 0);
        int cursorSizePref = mPrefs.getInt(KEY_PREF_CURSOR_SIZE, 0);
        Log.v(TAG, "Got settings" + Integer.toHexString(fgColorPref));
        String fgColorStr = Integer.toHexString(fgColorPref);
        String bgColorStr = Integer.toHexString(bgColorPref);
        ParticlesLib.settings(fgColorStr, bgColorStr, iterationsPref, cursorSizePref);

        mListener = new OnSharedPreferenceChangeListener() {
            @Override
            public void onSharedPreferenceChanged(SharedPreferences sPrefs,
            String key) {

                int fgColorPref = sPrefs.getInt(KEY_PREF_FG_COLOR, 0);
                int bgColorPref = sPrefs.getInt(KEY_PREF_BG_COLOR, 0);
                int iterationsPref = sPrefs.getInt(KEY_PREF_ITERATIONS, 0);
                int cursorSizePref = sPrefs.getInt(KEY_PREF_CURSOR_SIZE, 0);
                String fgColorStr = Integer.toHexString(fgColorPref);
                String bgColorStr = Integer.toHexString(bgColorPref);
                ParticlesLib.settings(fgColorStr, bgColorStr, iterationsPref, cursorSizePref);

                if (key.equals(KEY_PREF_FG_COLOR) || key.equals(KEY_PREF_BG_COLOR)) {
                    //String gridPref = sharedPreferences.getString(KEY_PREF_GRID, "");
                    //String methodPref = sharedPreferences.getString(KEY_PREF_METHOD, "");
                    Log.v(TAG, "Setting changed");
                }
            }
        };
        mPrefs.registerOnSharedPreferenceChangeListener(mListener);

        // Pass the asset manager to the native code
        sAssetManager = getAssets();
        ParticlesLib.createAssetManager( sAssetManager );

        // Create our view for OpenGL rendering
        mView = new ParticlesView( getApplication() );
        setContentView( mView );

    }

    @Override protected void onPause() {
        super.onPause();
        mView.onPause();
    }

    @Override protected void onResume() {
        super.onResume();
        mView.onResume();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mOrientationListener.disable();
    }

    public boolean onTouchEvent(MotionEvent touchevent)
    {
        switch (touchevent.getAction())
        {
        // when user first touches the screen we get x and y coordinate
            case MotionEvent.ACTION_DOWN:
            {
                x1 = touchevent.getX();
                y1 = touchevent.getY();
                ParticlesLib.touch(true, x1, y1);
                break;
            }
            case MotionEvent.ACTION_MOVE:
            {
                x1 = touchevent.getX();
                y1 = touchevent.getY();
                ParticlesLib.touch(true, x1, y1);
                break;
            }
            case MotionEvent.ACTION_UP:
            {
                x2 = touchevent.getX();
                y2 = touchevent.getY();
                ParticlesLib.touch(false, x2, y2);

                // calculate a direction vector and orientate it
                float dx = x2 - x1;
                float dy = - (y2 - y1);
                float dist = (float) Math.sqrt(dx*dx + dy*dy);

                if (dist > GESTURE_TRESHOLD) {
                    Log.v(TAG, "Orientation " + orientation);
                    float dirx = dx / dist;
                    float diry = dy / dist;
                    float theta = (float) Math.toRadians(-orientation);
                    float cs = (float) Math.cos(theta);
                    float sn = (float) Math.sin(theta);

                    float dirrx = dirx * cs - diry * sn; 
                    float dirry = dirx * sn + diry * cs;

                    // Log.v(TAG, "dx " + dx + " dy " + dy);
                    // Log.v(TAG, "dirrx " + dirrx + " dirry " + dirry);

                    // if left to left sweep event on screen
                    if (dirrx > 0)
                    {
                        if (orientation < 45 && orientation > 315) {
                                Toast.makeText(this, "Settings", Toast.LENGTH_LONG).show();
                        }
                        //Intent i = new Intent(ParticlesActivity.this, SettingsActivity.class);
                        //startActivity(i);
                    }
                }


            }
        }
        return mGestureDetector.onTouchEvent(touchevent);
    }

    private class GestureListener extends GestureDetector.SimpleOnGestureListener {
        @Override
        public boolean onDown(MotionEvent e) {
            return true;
        }
        // event when double tap occurs
        @Override
        public boolean onDoubleTap(MotionEvent e) {
            float x = e.getX();
            float y = e.getY();

            Log.d("Double Tap", "Tapped at: (" + x + "," + y + ")");

            Intent i = new Intent(ParticlesActivity.this, SettingsActivity.class);
            startActivity(i);
            return true;
        }
    }

}

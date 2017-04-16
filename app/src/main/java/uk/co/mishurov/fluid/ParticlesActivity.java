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


public class ParticlesActivity extends Activity {
    private static final String TAG = "Fluid";
    private static final float GESTURE_TRESHOLD = 3;
    public static final String KEY_PREF_GRID = "pref_grid";
    public static final String KEY_PREF_METHOD = "pref_method";


    SharedPreferences mPrefs;
    OnSharedPreferenceChangeListener mListener;
    GestureDetector mGestureDetector;

    static AssetManager sAssetManager;
    ParticlesView mView;
    OrientationEventListener mOrientationListener;
    float x1, y1;
    float x2, y2;
    int orientation;

    // On applications creation
    @Override protected void onCreate(Bundle savedInstanceState) {
        super.onCreate( savedInstanceState );

        // Listen orientation
        orientation = 0;

        mOrientationListener = new OrientationEventListener(this,
            SensorManager.SENSOR_DELAY_NORMAL) {
                @Override
                public void onOrientationChanged(int orientation) {
                    ParticlesActivity.this.orientation = orientation;
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
        String gridPref = mPrefs.getString(KEY_PREF_GRID, "");
        String methodPref = mPrefs.getString(KEY_PREF_METHOD, "");
        Log.v(TAG, "Got settings");

        mListener = new OnSharedPreferenceChangeListener() {
            @Override
            public void onSharedPreferenceChanged(SharedPreferences sharedPreferences,
            String key) {
                if (key.equals(KEY_PREF_GRID) || key.equals(KEY_PREF_METHOD)) {
                    String gridPref = sharedPreferences.getString(KEY_PREF_GRID, "");
                    String methodPref = sharedPreferences.getString(KEY_PREF_METHOD, "");
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

                break;
            }
            case MotionEvent.ACTION_MOVE:
            {
                x1 = touchevent.getX();
                y1 = touchevent.getY();
                ParticlesLib.touch(x1, y1);
                break;
            }
            case MotionEvent.ACTION_UP:
            {
                x2 = touchevent.getX();
                y2 = touchevent.getY();

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

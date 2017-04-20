package uk.co.mishurov.fluid;

import android.app.Activity;
import android.os.Bundle;
import android.hardware.SensorManager;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.OrientationEventListener;
import android.view.Display;
import android.view.Surface;
import android.view.LayoutInflater;
import android.view.Gravity;
import android.widget.Toast;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.content.Intent;
import android.content.res.AssetManager;
import android.preference.PreferenceManager;
import android.util.Log;


public class ParticlesActivity extends Activity {
    private static final String TAG = "Fluid";
    private static final float GESTURE_TRESHOLD = 3;
    private static final String KEY_PREF_FG_COLOR = "pref_fg_color";
    private static final String KEY_PREF_BG_COLOR = "pref_bg_color";
    private static final String KEY_PREF_ITERATIONS = "pref_iterations";
    private static final String KEY_PREF_CURSOR_SIZE = "pref_cursor_size";

    SharedPreferences mPrefs;
    OnSharedPreferenceChangeListener mListener;
    GestureDetector mGestureDetector;

    OrientationEventListener mOrientationListener;
    int mOrientation = 0;
    int mScreenRotation = 0;

    static AssetManager sAssetManager;
    ParticlesView mView;

    RotatingLayout mToasterLayout;

    @Override protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Toast for settings
        LayoutInflater inflater = getLayoutInflater();
        mToasterLayout = (RotatingLayout) inflater.inflate(R.layout.toaster, null);
        Toast toast = new Toast(getApplicationContext());
        toast.setGravity(Gravity.CENTER_VERTICAL, 0, 0);
        toast.setDuration(Toast.LENGTH_LONG);
        toast.setView(mToasterLayout);

        // Display rotation to fix orientation
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
                    int sr = ParticlesActivity.this.mScreenRotation;
                    int angle = orientation + sr;
                    ParticlesActivity.this.mOrientation = angle;
                    ParticlesLib.rotate(angle);
                    ParticlesActivity.this.mView.rotateDialog(angle);
                    ParticlesActivity.this.mToasterLayout.setAngle(angle);
                }
            };

        if (mOrientationListener.canDetectOrientation() == true) {
            mOrientationListener.enable();
        } else {
            mOrientationListener.disable();
        }
        mGestureDetector = new GestureDetector(this, new GestureListener());

        // Read and listen preferences
        mPrefs = PreferenceManager.getDefaultSharedPreferences(this);
        passSettings(mPrefs);
        mListener = new OnSharedPreferenceChangeListener() {
            @Override
            public void onSharedPreferenceChanged(
                SharedPreferences prefs, String key
            ) {
                passSettings(prefs);
            }
        };
        mPrefs.registerOnSharedPreferenceChangeListener(mListener);

        // Run native
        sAssetManager = getAssets();
        ParticlesLib.createAssetManager( sAssetManager );
        mView = new ParticlesView(this);
        setContentView(mView);

        // Toast for settings
        toast.show();
    }

    void passSettings(SharedPreferences prefs) {
        int fgColorPref = prefs.getInt(KEY_PREF_FG_COLOR, 0);
        int bgColorPref = prefs.getInt(KEY_PREF_BG_COLOR, 0);
        int iterationsPref = prefs.getInt(KEY_PREF_ITERATIONS, 0);
        int cursorSizePref = prefs.getInt(KEY_PREF_CURSOR_SIZE, 0);
        String fgColorStr = Integer.toHexString(fgColorPref);
        String bgColorStr = Integer.toHexString(bgColorPref);
        ParticlesLib.settings(
            fgColorStr, bgColorStr, iterationsPref, cursorSizePref
        );
    }

    @Override
    protected void onPause() {
        super.onPause();
        mView.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mView.onResume();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mOrientationListener.disable();
    }

    public boolean onTouchEvent(MotionEvent touchevent) {
        float x = touchevent.getX();
        float y = touchevent.getY();
        switch (touchevent.getAction())
        {
            case MotionEvent.ACTION_DOWN:
            {
                ParticlesLib.touch(true, x, y);
                break;
            }
            case MotionEvent.ACTION_MOVE:
            {
                ParticlesLib.touch(true, x, y);
                break;
            }
            case MotionEvent.ACTION_UP:
            {
                ParticlesLib.touch(false, x, y);
            }
        }
        return mGestureDetector.onTouchEvent(touchevent);
    }

    private class GestureListener extends GestureDetector.SimpleOnGestureListener {
        @Override
        public boolean onDoubleTap(MotionEvent e) {
            Intent i = new Intent(ParticlesActivity.this, SettingsActivity.class);
            startActivity(i);
            return true;
        }
    }
}


package uk.co.mishurov.fluid

import android.app.Activity
import android.os.Bundle
import android.hardware.SensorManager
import android.view.GestureDetector
import android.view.MotionEvent
import android.view.OrientationEventListener
import android.view.Surface
import android.view.Gravity
import android.widget.Toast
import android.content.SharedPreferences
import android.content.SharedPreferences.OnSharedPreferenceChangeListener
import android.content.Intent
import android.preference.PreferenceManager
import android.util.DisplayMetrics

import android.util.Log


class MainActivity : Activity()
{
    private var mPrefs: SharedPreferences? = null
    private var mListener: OnSharedPreferenceChangeListener? = null
    private var mGestureDetector: GestureDetector? = null

    private var mOrientationListener: OrientationEventListener? = null
    private var mOrientation = 0
    private var mScreenRotation = 0
    private var mView: FluidView? = null

    private var glScale = 1.0f

    private var mToasterLayout: RotatingLayout? = null

    override fun onCreate(savedInstanceState: Bundle?)
    {
        super.onCreate(savedInstanceState)

        // Toast for settings
        val inflater = getLayoutInflater()
        mToasterLayout = inflater.inflate(
                                    R.layout.toaster, null) as RotatingLayout
        val toast = Toast(getApplicationContext())
        toast.setGravity(Gravity.CENTER_VERTICAL, 0, 0)
        toast.setDuration(Toast.LENGTH_LONG)
        toast.setView(mToasterLayout)

        // Display rotation to fix orientation
        val display = getWindowManager().getDefaultDisplay()
        val rotation = display.getRotation()
        when (rotation) {
            Surface.ROTATION_90 -> mScreenRotation = 90
            Surface.ROTATION_180 -> mScreenRotation = 180
            Surface.ROTATION_270 -> mScreenRotation = -90
            else -> mScreenRotation = 0
        }

        mOrientationListener = object : OrientationEventListener(this,
                SensorManager.SENSOR_DELAY_NORMAL) {
            override fun onOrientationChanged(orientation: Int) {
                val sr = this@MainActivity.mScreenRotation
                val angle = orientation + sr
                this@MainActivity.mOrientation = angle
                this@MainActivity.mToasterLayout?.setAngle(angle)
                this@MainActivity.mView?.rotate(angle.toFloat())
            }
        }

        if (mOrientationListener?.canDetectOrientation() == true) {
            mOrientationListener?.enable()
        } else {
            mOrientationListener?.disable()
        }

        mGestureDetector = GestureDetector(this, GestureListener())

        // Read and listen preferences
        mPrefs = PreferenceManager.getDefaultSharedPreferences(this)
        PreferenceManager.setDefaultValues(this, R.xml.preferences, false);

        mListener = object : OnSharedPreferenceChangeListener
        {
            override fun onSharedPreferenceChanged(prefs: SharedPreferences,
                                                   key: String)
            {
                this@MainActivity.passSettings(prefs)
            }
        }

        val displayMetrics = DisplayMetrics()
        display.getMetrics(displayMetrics)
        val width = displayMetrics.widthPixels
        val height = displayMetrics.heightPixels

        if (width > GL_SIZE || height > GL_SIZE) {
            val max = if (width > height) width else height
            glScale = GL_SIZE / max.toFloat()
        }

        val glWidth = (width.toFloat() * glScale).toInt()
        val glHeight = (height.toFloat() * glScale).toInt()

        // Run native
        mView = FluidView(this)
        setContentView(mView)
        mView!!.getHolder().setFixedSize(glWidth, glHeight);
        // Pass settings after lib initialisation
        passSettings(mPrefs!!)
        // Toast about settings
        toast.show()
    }

    private fun toRgb(color: Int) : FloatArray
    {
        val r = (color shr 16) and 0xFF
        val g = (color shr 8) and 0xFF
        val b = (color shr 0) and 0xFF
        val u = 255.0f
        return floatArrayOf(r.toFloat() / u, g.toFloat() / u, b.toFloat() / u)
    }

    private fun passSettings(prefs: SharedPreferences)
    {
        val fgColorPref = prefs.getInt(KEY_PREF_FG_COLOR, 0)
        val bgColorPref = prefs.getInt(KEY_PREF_BG_COLOR, 1)
        val iterationsPref = prefs.getInt(KEY_PREF_ITERATIONS, 8)
        val cursorSizePref = prefs.getInt(KEY_PREF_CURSOR_SIZE, 10)
        //var fgColorStr = Integer.toHexString(fgColorPref)
        //var bgColorStr = Integer.toHexString(bgColorPref)

        mView!!.cursorSize = cursorSizePref * 9.0f
        mView!!.iterations = iterationsPref
        mView!!.fgColor = toRgb(fgColorPref)
        mView!!.bgColor = toRgb(bgColorPref)
    }

    override fun onPause()
    {
        mPrefs?.unregisterOnSharedPreferenceChangeListener(mListener);
        super.onPause()
        mView?.onPause()
    }

    override fun onResume()
    {
        mPrefs?.registerOnSharedPreferenceChangeListener(mListener)
        super.onResume()
        mView?.onResume()
    }

    override fun onDestroy()
    {
        super.onDestroy()
        mOrientationListener?.disable()
    }

    override fun onTouchEvent(touchevent: MotionEvent): Boolean
    {
        val x = touchevent.getX() * glScale
        val y = touchevent.getY() * glScale

        when (touchevent.getAction()) {
            MotionEvent.ACTION_DOWN -> {
                mView?.touch(true, x, y)
            }
            MotionEvent.ACTION_MOVE -> {
                mView?.touch(true, x, y)
            }
            MotionEvent.ACTION_UP -> {
                mView?.touch(false, x, y)
            }
        }

        return mGestureDetector?.onTouchEvent(touchevent) as Boolean
    }

    private inner class GestureListener :
                                    GestureDetector.SimpleOnGestureListener()
    {
        override fun onDoubleTap(e: MotionEvent): Boolean {
            val intent = Intent(this@MainActivity, SettingsActivity::class.java)
            startActivity(intent)
            return true
        }
    }

    companion object {
        private val TAG = "Fluid MainActivity"
        private val GESTURE_TRESHOLD = 3.0f
        private val KEY_PREF_FG_COLOR = "pref_fg_color"
        private val KEY_PREF_BG_COLOR = "pref_bg_color"
        private val KEY_PREF_ITERATIONS = "pref_iterations"
        private val KEY_PREF_CURSOR_SIZE = "pref_cursor_size"
        private val GL_SIZE = 1000.0f
    }
}


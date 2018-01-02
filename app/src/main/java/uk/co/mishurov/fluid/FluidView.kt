package uk.co.mishurov.fluid

import android.opengl.GLSurfaceView
import android.content.Context


class FluidView(context: Context) : GLSurfaceView(context)
{
    var isTouch = false
    var vx = 0.0f
    var vy = 0.0f
    var gravity = floatArrayOf(0.0f, 1.0f)
    var cursorSize = 50.0f
    var iterations = 5
    var bgColor = floatArrayOf(1.0f, 1.0f, 1.0f)
    var fgColor = floatArrayOf(0.0f, 0.0f, 0.0f)

    init {
        setPreserveEGLContextOnPause(true)
        setEGLContextClientVersion(2)
        setRenderer(FluidRenderer(this))
    }

    fun touch(aIsTouch: Boolean, ax: Float, ay: Float) {
        isTouch = aIsTouch
        vx = ax
        vy = ay
    }

    fun rotate(a: Float) {
        val angle = a + 90.0f;
        val theta = angle * kotlin.math.PI.toFloat() / 180.0f
        gravity = floatArrayOf(kotlin.math.cos(theta),  kotlin.math.sin(theta))
    }

    companion object
    {
        private val TAG = "Fluid FluidView"
    }
}


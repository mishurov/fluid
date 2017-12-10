package uk.co.mishurov.fluid

import android.content.Context
import android.graphics.Canvas
import android.graphics.Matrix
import android.graphics.Rect
import android.graphics.RectF
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.View
import android.view.ViewGroup
import android.view.ViewParent
import android.view.View.MeasureSpec.UNSPECIFIED

import java.lang.Math.PI
import java.lang.Math.abs
import java.lang.Math.ceil
import java.lang.Math.cos
import java.lang.Math.sin


class RotatingLayout(context: Context, attrs: AttributeSet? = null)
                                                : ViewGroup(context, attrs)
{
    private var angle: Int = 0
    private val rotateMatrix = Matrix()
    private val viewRectRotated = Rect()

    private val tempRectF1 = RectF()
    private val tempRectF2 = RectF()

    private val viewTouchPoint = FloatArray(2)
    private val childTouchPoint = FloatArray(2)

    private var angleChanged = true

    val view: View?
        get() = if (getChildCount() > 0) {
            getChildAt(0)
        } else {
            null
        }

    init {
        val a = context.obtainStyledAttributes(attrs, R.styleable.RotateLayout)
        angle = a.getInt(R.styleable.RotateLayout_angle, 0)
        a.recycle()

        setWillNotDraw(false)
    }

    fun getAngle(): Int
    {
        return angle
    }

    fun setAngle(angle: Int)
    {
        if (this.angle != angle) {
            this.angle = angle
            angleChanged = true
            requestLayout()
            invalidate()
        }
    }

    override fun onMeasure(widthMeasureSpec: Int, heightMeasureSpec: Int)
    {
        val child = view
        if (child != null) {
            if (abs(angle % 180) == 90) {

                measureChild(child, heightMeasureSpec, widthMeasureSpec)
                setMeasuredDimension(
                    resolveSize(child.getMeasuredHeight(), widthMeasureSpec),
                    resolveSize(child.getMeasuredWidth(), heightMeasureSpec)
                )
            } else if (abs(angle % 180) == 0) {
                measureChild(child, widthMeasureSpec, heightMeasureSpec)
                setMeasuredDimension(
                    resolveSize(child.getMeasuredWidth(), widthMeasureSpec),
                    resolveSize(child.getMeasuredHeight(), heightMeasureSpec)
                )
            } else {
                val childWithMeasureSpec = MeasureSpec.makeMeasureSpec(
                                                                0, UNSPECIFIED)
                val childHeightMeasureSpec = MeasureSpec.makeMeasureSpec(
                                                                0, UNSPECIFIED)
                measureChild(
                    child, childWithMeasureSpec, childHeightMeasureSpec
                )

                val measuredWidth = ceil(child.getMeasuredWidth() *
                    abs(cos(angleC())) + child.getMeasuredHeight() *
                    abs(sin(angleC()))
                )
                val measuredHeight = ceil(child.getMeasuredWidth() *
                    abs(sin(angleC())) + child.getMeasuredHeight() *
                    abs(cos(angleC()))
                )

                setMeasuredDimension(
                    resolveSize(
                        measuredWidth.toInt(), widthMeasureSpec.toInt()
                    ),
                    resolveSize(
                        measuredHeight.toInt(), heightMeasureSpec.toInt()
                    )
                )
            }
        } else {
            super.onMeasure(widthMeasureSpec, heightMeasureSpec)
        }
    }

    override fun onLayout(changed: Boolean, l: Int, t: Int, r: Int, b: Int)
    {
        val layoutWidth = r - l
        val layoutHeight = b - t

        if (angleChanged || changed) {
            val layoutRect = tempRectF1
            layoutRect.set(
                0F, 0F, layoutWidth.toFloat(), layoutHeight.toFloat()
            )
            val layoutRectRotated = tempRectF2
            rotateMatrix.setRotate(
                angle.toFloat(), layoutRect.centerX(), layoutRect.centerY()
            )
            rotateMatrix.mapRect(layoutRectRotated, layoutRect)
            layoutRectRotated.round(viewRectRotated)
            angleChanged = false
        }

        val child = view
        if (child != null) {
            val childLeft = (layoutWidth - child.getMeasuredWidth()) / 2
            val childTop = (layoutHeight - child.getMeasuredHeight()) / 2
            val childRight = childLeft + child.getMeasuredWidth()
            val childBottom = childTop + child.getMeasuredHeight()
            child.layout(childLeft, childTop, childRight, childBottom)
        }
    }

    override fun dispatchDraw(canvas: Canvas)
    {
        canvas.save()
        canvas.rotate(-angle.toFloat(), getWidth() / 2f, getHeight() / 2f)
        super.dispatchDraw(canvas)
        canvas.restore()
    }

    override fun invalidateChildInParent(location: IntArray,
                                                    dirty: Rect): ViewParent?
    {
        invalidate()
        return super.invalidateChildInParent(location, dirty)
    }

    override fun dispatchTouchEvent(event: MotionEvent): Boolean
    {
        viewTouchPoint[0] = event.getX()
        viewTouchPoint[1] = event.getY()

        rotateMatrix.mapPoints(childTouchPoint, viewTouchPoint)

        event.setLocation(childTouchPoint[0], childTouchPoint[1])
        val result = super.dispatchTouchEvent(event)
        event.setLocation(viewTouchPoint[0], viewTouchPoint[1])

        return result
    }

    private fun angleC(): Double
    {
        val TAU = 2 * PI
        return TAU * angle / 360
    }

}

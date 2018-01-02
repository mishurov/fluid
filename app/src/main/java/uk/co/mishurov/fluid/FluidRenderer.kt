package uk.co.mishurov.fluid


import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

import android.opengl.GLES20
import android.opengl.GLSurfaceView

import uk.co.mishurov.fluid.engine.Compute
import uk.co.mishurov.fluid.engine.Geometry
import uk.co.mishurov.fluid.engine.Shader
import uk.co.mishurov.fluid.engine.Shader.Uniform
import uk.co.mishurov.fluid.engine.Texture

import android.util.Log


class FluidRenderer(internal var mParentView: FluidView)
                                                : GLSurfaceView.Renderer
{
    private var draw : Compute? = null
    private var zilch : Compute? = null
    private var impulse : Compute? = null
    private var advect : Compute? = null
    private var buoyancy : Compute? = null
    private var divergence : Compute? = null
    private var jacobi : Compute? = null
    private var gradient : Compute? = null

    private var densityPing : Texture? = null
    private var densityPong : Texture? = null
    private var velocityPing : Texture? = null
    private var velocityPong : Texture? = null
    private var pressurePing : Texture? = null
    private var pressurePong : Texture? = null
    private var temperaturePing : Texture? = null
    private var temperaturePong : Texture? = null
    private var divergencePing : Texture? = null

    private var px_x = 0.0f
    private var px_y = 0.0f
    private var x_0 = 0.0f
    private var y_0 = 0.0f
    private var step = 1.0f / 60.0f
    private var cursorSize = 0.0f

    override fun onSurfaceChanged(gl: GL10, width: Int, height: Int)
    {
        GLES20.glViewport(0, 0, width, height)
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT)

        cursorSize = mParentView.cursorSize

        px_x = 1.0f / width.toFloat()
        px_y = 1.0f / height.toFloat()
        val px = floatArrayOf(px_x, px_y)
        val px1 = floatArrayOf(1.0f, width.toFloat() / height.toFloat())

        val all = Geometry(quad(1.0f, 1.0f), shortArrayOf(0, 1, 2, 3, 4, 5))
        val inside = Geometry(
            quad(1.0f - px_x * 2.0f, 1.0f - px_y * 2.0f),
            shortArrayOf(0, 1, 2, 3, 4, 5)
        )

        densityPing = Texture(width, height)
        densityPong = Texture(width, height)
        velocityPing = Texture(width, height)
        velocityPong = Texture(width, height)
        pressurePing = Texture(width, height)
        pressurePong = Texture(width, height)
        temperaturePing = Texture(width, height)
        temperaturePong = Texture(width, height)
        divergencePing = Texture(width, height)

        val ctx = mParentView.getContext()
        val advectField = Shader(ctx, R.raw.surface, R.raw.advect_field)
        val jacobiField = Shader(ctx, R.raw.surface, R.raw.jacobi)
        val gradientField = Shader(ctx, R.raw.surface, R.raw.gradient)
        val divergenceField = Shader(ctx, R.raw.surface, R.raw.divergence)
        val buoyancyField = Shader(ctx, R.raw.surface, R.raw.buoyancy)
        val drawField = Shader(ctx, R.raw.surface, R.raw.visualize)
        val zilchField = Shader(ctx, R.raw.surface, R.raw.fill_packed_zeroes)
        val impulseField = Shader(ctx, R.raw.surface, R.raw.add_field)


        advect = Compute(
            advectField,
            inside,
            mutableMapOf<String, Uniform>(
                "px" to Uniform(null, px),
                "px1" to Uniform(null, px1),
                "dissipation" to Uniform(null, floatArrayOf(0.999f)),
                "vector_size" to Uniform(null, floatArrayOf(2.0f)),
                "dt" to Uniform(null, floatArrayOf(step)),
                "velocity" to Uniform(velocityPing!!, null),
                "source" to Uniform(velocityPing!!, null)
            ),
            velocityPong!!
        )

        buoyancy = Compute(
            buoyancyField,
            all,
            mutableMapOf<String, Uniform>(
                "px" to Uniform(null, px),
                "px1" to Uniform(null, px1),
                "dt" to Uniform(null, floatArrayOf(step)),
                "ambient_temperature" to Uniform(null, floatArrayOf(0.0f)),
                "sigma" to Uniform(null, floatArrayOf(1.5f)),
                "kappa" to Uniform(null, floatArrayOf(0.05f)),
                "gravity" to Uniform(null, floatArrayOf(0.0f, 0.0f)),
                "velocity" to Uniform(velocityPing!!, null),
                "density" to Uniform(densityPing!!, null),
                "temperature" to Uniform(temperaturePing!!, null)
            ),
            velocityPong!!
        )

        divergence = Compute(
            divergenceField,
            all,
            mutableMapOf<String, Uniform>(
                "px" to Uniform(null, px),
                "velocity" to Uniform(velocityPing!!, null)
            ),
            temperaturePing!!
        )

        jacobi = Compute(
            jacobiField,
            all,
            mutableMapOf<String, Uniform>(
                "px" to Uniform(null, px),
                "alpha" to Uniform(null, floatArrayOf(-1.0f)),
                "beta" to Uniform(null, floatArrayOf(0.25f)),
                "pressure" to Uniform(pressurePing!!, null),
                "divergence" to Uniform(divergencePing!!, null)
            ),
            pressurePong!!
        )

        gradient = Compute(
            gradientField,
            all,
            mutableMapOf<String, Uniform>(
                "px" to Uniform(null, px),
                "scale" to Uniform(null, floatArrayOf(1.0f)),
                "pressure" to Uniform(pressurePing!!, null),
                "velocity" to Uniform(velocityPing!!, null)
            ),
            velocityPong!!
        )


        impulse = Compute(
            impulseField,
            all,
            mutableMapOf<String, Uniform>(
                "px" to Uniform(null, px),
                "vector_size" to Uniform(null, floatArrayOf(1.0f)),
                "init" to Uniform(null, floatArrayOf(0.0f)),
                "force" to Uniform(null, floatArrayOf(0.0f, 0.0f)),
                "center" to Uniform(null, floatArrayOf(0.0f, 0.0f)),
                "source" to Uniform(densityPing!!, null)
            ),
            densityPong
        )

        draw = Compute(
            drawField,
            all,
            mutableMapOf<String, Uniform>(
                "sampler" to Uniform(densityPong!!, null),
                "fg_color" to Uniform(null, mParentView.fgColor),
                "bg_color" to Uniform(null, mParentView.bgColor),
                "px" to Uniform(null, px)
            ),
            null
        )

        zilch = Compute(
            zilchField,
            all,
            mutableMapOf<String, Uniform>(
                "px" to Uniform(null, px),
                "vector_size" to Uniform(null, floatArrayOf(1.0f))
            ),
            densityPing!!
        )
        zilch?.run()
        zilch?.setOutput(densityPong!!)
        zilch?.run()

        var uniforms = mutableMapOf<String, Uniform>(
            "vector_size" to Uniform(null, floatArrayOf(2.0f))
        )
        zilch?.setUniforms(uniforms)
        zilch?.setOutput(velocityPing!!)
        zilch?.run()
        zilch?.setOutput(velocityPong!!)
        zilch?.run()
    }

    override fun onDrawFrame(gl: GL10)
    {
        var uniforms = mutableMapOf<String, Uniform>(
            "vector_size" to Uniform(null, floatArrayOf(2.0f)),
            "source" to Uniform(velocityPing!!, null),
            "dissipation" to Uniform(null, floatArrayOf(0.99f)),
            "velocity" to Uniform(velocityPing!!, null)
        )

        advect?.setUniforms(uniforms)
        advect?.setOutput(velocityPong!!)
        advect?.run()
        var tmp = velocityPing
        velocityPing = velocityPong
        velocityPong = tmp

        uniforms = mutableMapOf<String, Uniform>(
            "vector_size" to Uniform(null, floatArrayOf(1.0f)),
            "source" to Uniform(temperaturePing!!, null),
            "dissipation" to Uniform(null, floatArrayOf(0.95f)),
            "velocity" to Uniform(velocityPing!!, null)
        )

        advect?.setUniforms(uniforms)
        advect?.setOutput(temperaturePong!!)
        advect?.run()

        uniforms = mutableMapOf<String, Uniform>(
            "source" to Uniform(densityPing!!, null),
            "velocity" to Uniform(velocityPing!!, null),
            "dissipation" to Uniform(null, floatArrayOf(0.999f))
        )

        advect?.setUniforms(uniforms)
        advect?.setOutput(densityPong!!)
        advect?.run()

        uniforms = mutableMapOf<String, Uniform>(
            "velocity" to Uniform(velocityPing!!, null),
            "temperature" to Uniform(temperaturePong!!, null),
            "density" to Uniform(densityPong!!, null),
            "gravity" to Uniform(null, mParentView.gravity)
        )

        buoyancy?.setUniforms(uniforms)
        buoyancy?.setOutput(velocityPong!!)
        buoyancy?.run()

        val x_1 = mParentView.vx
        val y_1 = mParentView.vy
        val xd = x_1 - x_0
        val yd = y_1 - y_0
        x_0 = x_1
        y_0 = y_1

        val force = mutableMapOf<String, Uniform>(
            "source" to Uniform(velocityPong!!, null),
            "vector_size" to Uniform(null, floatArrayOf(2.0f)),
            "force" to Uniform(null, floatArrayOf(
                                xd * px_x * cursorSize,
                                -yd * px_y * cursorSize)
            ),
            "center" to Uniform(null, floatArrayOf(x_0 * px_x, 1 - y_0 * px_y)),
            "scale" to Uniform(null, floatArrayOf(cursorSize * px_x,
                                                    cursorSize * px_y))
        )

        impulse?.setUniforms(force)
        impulse?.setOutput(velocityPing!!)
        impulse?.run()

        val sizeFactor = cursorSize * 0.003f
        val forceMin = 7.0f
        var forceAvg = kotlin.math.abs(xd) + kotlin.math.abs(yd)
        forceAvg = if (mParentView.isTouch) kotlin.math.max(forceMin, forceAvg) else 0.0f

        val forceAbs = mutableMapOf<String, Uniform>(
            "source" to Uniform(densityPong!!, null),
            "vector_size" to Uniform(null, floatArrayOf(1.0f)),
            "force" to Uniform(null, floatArrayOf(
                                forceAvg * px_x / sizeFactor,
                                forceAvg * px_y / sizeFactor)
            ),
            "scale" to Uniform(null, floatArrayOf(cursorSize * px_x,
                                                    cursorSize * px_y))
        )

        impulse?.setUniforms(forceAbs)
        impulse?.setOutput(densityPing!!)
        impulse?.run()


        uniforms = mutableMapOf<String, Uniform>(
            "source" to Uniform(temperaturePong!!, null)
        )

        impulse?.setUniforms(uniforms)
        impulse?.setOutput(temperaturePing!!)
        impulse?.run()


        uniforms = mutableMapOf<String, Uniform>(
            "velocity" to Uniform(velocityPing!!, null)
        )
        divergence?.setUniforms(uniforms)
        divergence?.setOutput(divergencePing!!)
        divergence?.run()

        for (i in 1..mParentView.iterations) {
            tmp = pressurePing
            pressurePing = pressurePong
            pressurePong = tmp

            uniforms = mutableMapOf<String, Uniform>(
                "divergence" to Uniform(divergencePing!!, null),
                "pressure" to Uniform(pressurePing!!, null)
            )
            jacobi?.setUniforms(uniforms)
            jacobi?.setOutput(pressurePong!!)
            jacobi?.run()
        }

        uniforms = mutableMapOf<String, Uniform>(
            "velocity" to Uniform(velocityPing!!, null),
            "pressure" to Uniform(pressurePong!!, null)
        )
        gradient?.setUniforms(uniforms)
        gradient?.setOutput(velocityPong!!)
        gradient?.run()

        tmp = velocityPing
        velocityPing = velocityPong
        velocityPong = tmp

        uniforms = mutableMapOf<String, Uniform>(
            "sampler" to Uniform(densityPing!!, null)
        )

        draw?.setUniforms(uniforms)
        draw?.run()

    }

    override fun onSurfaceCreated(gl: GL10, config: EGLConfig)
    {
        GLES20.glClearColor(0.0f, 0.0f, 0.0f, 1.0f)
    }

    fun quad(xscale: Float, yscale: Float): FloatArray
    {
        return floatArrayOf(
            -xscale,  yscale, 0.0f,
            -xscale, -yscale, 0.0f,
             xscale, -yscale, 0.0f,
            -xscale,  yscale, 0.0f,
             xscale, -yscale, 0.0f,
             xscale,  yscale, 0.0f
        )
    }

    companion object
    {
        private val TAG = "Fluid FluidRenderer"
    }
}

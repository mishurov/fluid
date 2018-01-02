package uk.co.mishurov.fluid.engine

import java.io.BufferedReader
import java.io.IOException
import java.io.InputStreamReader

import android.content.Context
import android.content.res.Resources
import android.opengl.GLES20

import android.util.Log


class Shader(ctx: Context, vertId : Int, fragId : Int)
{
    class Uniform(val texture : Texture?, val array : FloatArray?)

    val program: Int

    private val mRes: Resources
    private val mPkg: String

    private val mUniformNames = mutableListOf<String>()
    private val mUniformLocations = mutableMapOf<String, Int>()

    init {
        mRes = ctx.getResources()
        mPkg = ctx.getPackageName()

        val vertexShader = loadShader(
            GLES20.GL_VERTEX_SHADER, readShaderRes(vertId)
        )
        val fragmentShader = loadShader(
            GLES20.GL_FRAGMENT_SHADER, readShaderRes(fragId)
        )

        program = GLES20.glCreateProgram()
        GLES20.glAttachShader(program, vertexShader)
        GLES20.glAttachShader(program, fragmentShader)
        GLES20.glLinkProgram(program)
    }

    fun use() {
        GLES20.glUseProgram(program)
    }

    private fun prepareUniforms(uniforms: Map<String, Uniform>)
    {
        for (k in uniforms.keys) {
            mUniformNames.add(k)
            mUniformLocations[k] = GLES20.glGetUniformLocation(
                program, k
            )
        }
    }

    fun uniforms(u: Map<String, Uniform>)
    {
        if (mUniformNames.count() == 0) {
            prepareUniforms(u)
        }

        for (name in mUniformNames) {
            val location = mUniformLocations[name]!!
            val value = u[name]!!

            if (value.texture != null) {
                value.texture.uniform(location)
            } else {
                val array = value.array!!
                when (array.size) {
                    1 -> GLES20.glUniform1f(location, array[0])
                    2 -> GLES20.glUniform2f(location, array[0], array[1])
                    3 -> GLES20.glUniform3f(location, array[0], array[1], array[2])
                }
            }
        }

    }

    private fun loadShader(type: Int, shaderCode: String) : Int
    {
        val shader = GLES20.glCreateShader(type)
        GLES20.glShaderSource(shader, shaderCode)
        GLES20.glCompileShader(shader)
        return shader
    }

    private fun checkGlError(glOperation: String)
    {
        var error = GLES20.glGetError()
        if (error != GLES20.GL_NO_ERROR) {
            val output = glOperation + ": glError " + error.toString()
            Log.e(TAG, output)
            throw RuntimeException(output)
        }
    }

    private fun readShaderRes(resId: Int) : String
    {
        val inputStream = mRes.openRawResource(resId)
        try {
            val reader = BufferedReader(InputStreamReader(inputStream))
            val sb = StringBuilder()

            var line: String? = null
            while ({ line = reader.readLine(); line }() != null) {
                if (line!!.startsWith("#include")) {
                    val resName = REGEX.matchEntire(line!!)?.groups?.get(1)?.value
                    val inclId = mRes.getIdentifier(resName, "raw", mPkg)
                    sb.append(readShaderRes(inclId))
                } else {
                    sb.append(line).append("\n")
                }
            }
            reader.close()
            return sb.toString()
        } catch (e: IOException) {
            e.printStackTrace()
        }
        return ""
    }

    companion object {
        internal val TAG = "Fluid Framework Shader"
        internal val REGEX = """#include \"([^\".]+).*\"""".toRegex()
    }
}

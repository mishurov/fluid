package uk.co.mishurov.fluid.engine


import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.nio.FloatBuffer
import java.nio.ShortBuffer

import android.opengl.GLES20


class Geometry(positions: FloatArray, indices: ShortArray)
{
    private val vertexBuffer: FloatBuffer
    private val indexBuffer: ShortBuffer
    private val indexSize : Int
    private val vertexStride = COORDS_PER_VERTEX * 4

    init {
        val vb = ByteBuffer.allocateDirect(positions.size * 4)
        vb.order(ByteOrder.nativeOrder())
        vertexBuffer = vb.asFloatBuffer()
        vertexBuffer.put(positions)
        vertexBuffer.position(0)

        val ib = ByteBuffer.allocateDirect(indices.size * 2)
        ib.order(ByteOrder.nativeOrder())
        indexBuffer = ib.asShortBuffer()
        indexBuffer.put(indices)
        indexBuffer.position(0)
        indexSize = indices.size
    }

    fun draw(program: Int)
    {
        val handle = GLES20.glGetAttribLocation(program, "position")

        GLES20.glEnableVertexAttribArray(handle)

        GLES20.glVertexAttribPointer(
            handle,
            COORDS_PER_VERTEX,
            GLES20.GL_FLOAT,
            false,
            vertexStride,
            vertexBuffer
        )

        GLES20.glDrawElements(
            GLES20.GL_TRIANGLES,
            indexSize,
            GLES20.GL_UNSIGNED_SHORT,
            indexBuffer
        )

        GLES20.glDisableVertexAttribArray(handle)
    }

    companion object {
        internal val COORDS_PER_VERTEX = 3
    }
}

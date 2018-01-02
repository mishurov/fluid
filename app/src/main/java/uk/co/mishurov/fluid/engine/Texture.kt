package uk.co.mishurov.fluid.engine

import android.opengl.GLES20


class Texture(width : Int, height : Int)
{
    var bound = false
    var supported = false

    private var mTextureHandle = 0
    private var mFrameBufferHandle = 0
    private var mUnit = -1


    init {
        val buferArray = IntArray(1)
        GLES20.glGenFramebuffers(1, buferArray, 0)
        mFrameBufferHandle = buferArray[0]
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, mFrameBufferHandle)

        val textureArray = IntArray(1)
        GLES20.glGenTextures(1, textureArray, 0)
        mTextureHandle = textureArray[0]
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mTextureHandle)

        GLES20.glTexImage2D(
            GLES20.GL_TEXTURE_2D,
            0,
            GLES20.GL_RGBA,
            width,
            height,
            0,
            GLES20.GL_RGBA,
            GLES20.GL_UNSIGNED_BYTE,
            null
        )

        GLES20.glTexParameteri(
            GLES20.GL_TEXTURE_2D,
            GLES20.GL_TEXTURE_MAG_FILTER,
            GLES20.GL_LINEAR
        )

        GLES20.glTexParameteri(
            GLES20.GL_TEXTURE_2D,
            GLES20.GL_TEXTURE_MIN_FILTER,
            GLES20.GL_LINEAR
        )

        GLES20.glTexParameteri(
            GLES20.GL_TEXTURE_2D,
            GLES20.GL_TEXTURE_WRAP_S,
            GLES20.GL_CLAMP_TO_EDGE
        )

        GLES20.glTexParameteri(
            GLES20.GL_TEXTURE_2D,
            GLES20.GL_TEXTURE_WRAP_T,
            GLES20.GL_CLAMP_TO_EDGE
        )

        GLES20.glFramebufferTexture2D(
            GLES20.GL_FRAMEBUFFER,
            GLES20.GL_COLOR_ATTACHMENT0,
            GLES20.GL_TEXTURE_2D,
            mTextureHandle,
            0
        )

        supported = (
            GLES20.glCheckFramebufferStatus(GLES20.GL_FRAMEBUFFER) ==
                                                GLES20.GL_FRAMEBUFFER_COMPLETE
        )

        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0)
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, 0)
    }

    fun bind() {
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, mFrameBufferHandle)
    }

    fun unbind() {
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, 0)
    }
        
    fun bindTexture(unit: Int) {
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0 + unit)
        mUnit = unit
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mTextureHandle)
        bound = true
    }

    fun unbindTexture() {
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0 + mUnit)
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0)
        mUnit = -1
        bound = false
    }

    fun uniform(location: Int)
    {
        GLES20.glUniform1i(location, mUnit)
    }
}


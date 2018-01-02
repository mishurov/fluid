package uk.co.mishurov.fluid.engine

import uk.co.mishurov.fluid.engine.Shader.Uniform

class Compute(internal val shader: Shader,
                internal val mesh: Geometry,
                internal val uniforms: MutableMap<String, Uniform>,
                internal var output: Texture?)
{
    fun run()
    {
        if (output != null) output?.bind()

        var texUnit = 0
        for (v in uniforms.values) {
            if (v.texture != null && !v.texture.bound) {
                v.texture.bindTexture(texUnit++)
            }
        }

        shader.use()
        shader.uniforms(uniforms)

        mesh.draw(shader.program)

        for (v in uniforms.values) {
            if (v.texture != null && v.texture.bound) {
                v.texture.unbindTexture()
            }
        }

        if (output != null) output?.unbind()
    }

    fun setUniforms(u: Map<String, Uniform>)
    {
        for ((k, v) in u) {
            uniforms[k] = v
        }
    }

    fun setOutput(o: Texture?)
    {
        output = o
    }

}

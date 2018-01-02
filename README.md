# Inkling

A real time fluid simulation based on the Navier-Stokes equations and computed in OpenGL fragment shaders.

![](http://mishurov.co.uk/images/github/fluid/Screenshot1.png)

[Android version](https://play.google.com/store/apps/details?id=uk.co.mishurov.fluid)

[WebGL version](http://mishurov.co.uk/inkling/index.html)

In a nutshell, the governing Navier-Stokes equations for incompressible fluid (i.e. momentum and mass conservation equations) are discretised via finite differences, advection is solved by a semi-Lagrangian method (unconditionally stable) and a Jacobi relaxation metod is being used for the Poisson-pressure equation, the resulting pressure gradient is substracted in order to get a divergence-free velocity field for a next timestep. The maths and some code of the fragment shaders are described in Nvidia's [GPU Gems](http://developer.download.nvidia.com/books/HTML/gpugems/gpugems_ch38.html)

The framework for the fluid simulation is based on this [project](https://github.com/jwagner/fluidwebgl).

Floating point scalar and vector fields are packed (lossy) into unsigned byte textures in order to get rid of GL_OES_texture_float depenency, the extention is not supported by some GPUs in Android devices.


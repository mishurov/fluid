# Inkling

A real time fluid simulation based on the Navier-Stokes equations and computed in OpenGL shaders.

[![](https://mishurov.000webhostapp.com/github/fluid/Screenshot1.png )](https://play.google.com/store/apps/details?id=uk.co.mishurov.fluid)

[The app on Play Market](https://play.google.com/store/apps/details?id=uk.co.mishurov.fluid)

[WebGL version](http://mishurov.co.uk/inkling/index.html)

Fluid simulation using OpenGL vertex and fragment shaders.

GLView and asset managements are based on and use utils from this document:
https://software.intel.com/en-us/articles/setting-up-native-opengl-es-on-android-platforms

Engine for fluids is ported from js to cpp from this project:
https://github.com/jwagner/fluidwebgl

In a nutshell, it's a simplified numerical approach to compute Navier-Stokes equtions using Jacobi method for the Poisson part of the equations. The math part is described here: http://http.developer.nvidia.com/GPUGems/gpugems_ch38.html

The ported version isn't perfectly memory efficient because I allocate unnecessary bytes for STL maps and pairs in order to keep the architecture from the js project.

I added density and temperature fields, added buoyancy calculations, removed boundary calculations.

I wrote functions to pack floats to 4 unsigned byte texture in order to get rid of GL_OES_texture_float depenency.


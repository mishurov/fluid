# Inkling

A real time fluid simulation based on the Navier-Stokes equations and computed in OpenGL shaders.

[![](http://mishurov.co.uk/images/github/fluid/Screenshot1.png)](https://play.google.com/store/apps/details?id=uk.co.mishurov.fluid)

[The app on Play Market](https://play.google.com/store/apps/details?id=uk.co.mishurov.fluid)

[WebGL version](http://mishurov.co.uk/inkling/index.html)

Fluid simulation using OpenGL vertex and fragment shaders

The OpenGL view and asset management are based oh this
https://software.intel.com/en-us/articles/setting-up-native-opengl-es-on-android-platforms

The engine for the fluid simulation was ported from js to cpp from this project
https://github.com/jwagner/fluidwebgl

In a nutshell, it's a simple numerical approach to solve the Navier-Stokes equtions using Jacobi method for the Poisson part of the equations. The maths and fragment shaders are described here: http://developer.download.nvidia.com/books/HTML/gpugems/gpugems_ch38.html

The ported version isn't perfectly memory efficient because I allocate unnecessary bytes for STL maps and pairs in order to keep the architecture from the js project.

I've added density and temperature fields on top of the framework, added the buoyancy field, removed the boundary.

I also added functions to pack floats into 4 unsigned byte texture in order to get rid of GL_OES_texture_float depenency.


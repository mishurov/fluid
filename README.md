# Inkling

A real time fluid simulation based on the Navier-Stokes equations and computed in OpenGL fragment shaders.

![](http://mishurov.co.uk/images/github/fluid/Screenshot1.png)

Unfortunately, despite that the app works in an Android emulator, it doesn't work well on many real devices thus I've unpublished the app from the store. I believe I get a time to fix that. I know what to do but it requires a lot of refactoring including performace optimisation.

[WebGL version](http://mishurov.co.uk/inkling/index.html)

In a nutshell, it's a simple numerical approach to solve the Navier-Stokes equtions using Jacobi method for the Poisson part of the equations. The maths and fragment shaders are described here: http://developer.download.nvidia.com/books/HTML/gpugems/gpugems_ch38.html

The OpenGL view and asset management are based oh this
https://software.intel.com/en-us/articles/setting-up-native-opengl-es-on-android-platforms

The engine for the fluid simulation was ported from js to cpp from this project
https://github.com/jwagner/fluidwebgl

The ported version isn't perfectly memory efficient because I allocate unnecessary bytes for STL maps and pairs in order to keep the architecture from the js project.

I've added density and temperature fields on top of the framework, added the buoyancy field, removed the boundary.

I also added functions to pack floats into 4 unsigned byte texture in order to get rid of GL_OES_texture_float depenency.


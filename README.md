# Inkling

A real time fluid simulation based on the Navier-Stokes equations and computed in OpenGL shaders.

[The app on Play Market](https://play.google.com/store/apps/details?id=uk.co.mishurov.fluid)
[WebGL version](http://mishurov.co.uk/inkling/index.html)

Fluid simulation using OpenGL vertex and fragment shaders.

GLView and asset managements are based and use utils from this document:
https://software.intel.com/en-us/articles/setting-up-native-opengl-es-on-android-platforms

Engine for fluids is ported from js to cpp from this project:
https://github.com/jwagner/fluidwebgl

The ported version isn't perfectly memory efficient because I allocate unnecessary bytes for STL maps and pairs in order to keep the architecture from the js project.

I added density and temperature fields, added buoyancy calculations, removed boundary calculations.

The drawback is that it uses float textures, they are not supported in  Open GL ES 2.0, it uses GL_OES_texture_float extension which may not be suported on some devices. I'm working on packing float vector data to unsigned byte RGBA texture.

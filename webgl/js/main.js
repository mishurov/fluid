define(function(require, exports, module){

var jscolor = require('lib/jscolor.min');

var Loader = require('engine/loader'),
    Clock = require('engine/clock').Clock,
    ShaderManager = require('engine/shader').Manager,
    geometry = require('engine/geometry'),
    FBO = require('engine/texture').FBO,
    Mesh = require('engine/mesh').Mesh,
    glcontext = require('engine/context'),
    ComputeKernel = require('./compute').Kernel;
    debounce = require('utils').debounce,
    glm = require('engine/gl-matrix'),
    vec2 = glm.vec2,
    vec3 = glm.vec3;

var canvas = document.getElementById('webGLCanvas'),
    gl = glcontext.initialize(canvas, {
        context: {
            depth: false
        },
        debug: false,
        //log_all: true,
        extensions: {
            texture_float: true
        }
    }),
    options = {
        iterations: 16,
        mouse_force: 1,
        resolution: 1,
        cursor_size: 50,
        step: 1/60
    },
    clock = new Clock(canvas),
    loader = new Loader(),
    resources = loader.resources,
    shaders = new ShaderManager(gl, resources);

function hasFloatLuminanceFBOSupport(){
    var fbo = new FBO(gl, 32, 32, gl.FLOAT, gl.LUMINANCE);
    return fbo.supported;
}

function init(){
    gl.getExtension('OES_texture_float_linear');
    var format = hasFloatLuminanceFBOSupport() ? gl.LUMINANCE : gl.RGBA;

    var rect = canvas.getBoundingClientRect(),
        width = rect.width * options.resolution,
        height = rect.height * options.resolution;

    setup(width, height, format);

    clock.start();
}

function setup(width, height, singleComponentFboFormat){
    canvas.width = width,
    canvas.height = height;

    gl.viewport(0, 0, width, height);
    gl.lineWidth(1.0);

    var px_x = 1.0/canvas.width,
        px_y = 1.0/canvas.height,
        px = [px_x, px_y];
        px1 = [1.0, canvas.width/canvas.height],
        inside = new Mesh(gl, {
            vertex: geometry.screen_quad(1.0-px_x*2.0, 1.0-px_y*2.0),
            attributes: {
                position: {}
            }
        }),
        all = new Mesh(gl, {
            vertex: geometry.screen_quad(1.0, 1.0),
            attributes: {
                position: {}
            }
        }),
        boundary = new Mesh(gl, {
            mode: gl.LINES,
            vertex: new Float32Array([
                // bottom
                -1+px_x*0.0, -1+px_y*0.0,
                -1+px_x*0.0, -1+px_y*2.0,

                 1-px_x*0.0, -1+px_y*0.0,
                 1-px_x*0.0, -1+px_y*2.0,

                // top
                -1+px_x*0.0,  1-px_y*0.0,
                -1+px_x*0.0,  1-px_y*2.0,

                 1-px_x*0.0,  1-px_y*0.0,
                 1-px_x*0.0,  1-px_y*2.0,

                // left
                -1+px_x*0.0,  1-px_y*0.0,
                -1+px_x*2.0,  1-px_y*0.0,

                -1+px_x*0.0, -1+px_y*0.0,
                -1+px_x*2.0, -1+px_y*0.0,

                // right
                 1-px_x*0.0,  1-px_y*0.0,
                 1-px_x*2.0,  1-px_y*0.0,

                 1-px_x*0.0, -1+px_y*0.0,
                 1-px_x*2.0, -1+px_y*0.0

            ]),
            attributes: {
                position: {
                    size: 2,
                    stride: 16,
                    offset: 0
                },
                offset: {
                    size: 2,
                    stride: 16,
                    offset: 8
                }
            }
        }),

        advect_field = shaders.get('kernel', 'advect');
        advect_scalar = shaders.get('kernel', 'advect_scalar');
        jacobi = shaders.get('kernel', 'jacobi');
        gradient = shaders.get('kernel', 'gradient');
        divergence = shaders.get('kernel', 'divergence');
        buoyancy = shaders.get('kernel', 'buoyancy');
        visualize = shaders.get('kernel', 'visualize');
        add_field = shaders.get('kernel', 'add_field');
        add_scalar = shaders.get('kernel', 'add_scalar');

        fmt = gl.UNSIGNED_BYTE;

        velocity_ping = new FBO(gl, width, height, fmt);
        velocity_pong = new FBO(gl, width, height, fmt);
        density_ping = new FBO(gl, width, height, fmt);
        density_pong = new FBO(gl, width, height, fmt);
        pressure_ping = new FBO(gl, width, height, fmt);
        pressure_pong = new FBO(gl, width, height, fmt);
        temperature_ping = new FBO(gl, width, height, fmt);
        temperature_pong = new FBO(gl, width, height, fmt);
        divergence_pong = new FBO(gl, width, height, fmt);

        var advect = new ComputeKernel(gl, {
            shader: advect_field,
            mesh: inside,
            uniforms: {
                px: px,
                px1: px1,
                dissipation: 0.999,
                velocity: velocity_ping,
                source: velocity_ping,
                dt: options.step
            },
            output: velocity_pong
        }),
        advect_sc = new ComputeKernel(gl, {
            shader: advect_scalar,
            mesh: all,
            uniforms: {
                px: px,
                px1: px1,
                dissipation: 0.999,
                velocity: velocity_ping,
                source: velocity_ping,
                dt: options.step
            },
            output: velocity_pong
        }),
        apply_buoyancy = new ComputeKernel(gl, {
            shader: buoyancy,
            mesh: all,
            uniforms: {
                ambient_temperature: 0.0,
                sigma: 1.5,
                kappa: 0.05,
                gravity: vec2.create([0.0, 0.0]),
                velocity: velocity_ping,
                temperature: temperature_ping,
                density: density_ping,
                px: px,
                px1: px1,
                dt: options.step
            },
            output: velocity_pong
        }),
        cursor = new Mesh(gl, {
            vertex: geometry.screen_quad(px_x*options.cursor_size*4, px_y*options.cursor_size*4),
            attributes: {
                position: {}
            }
        }),
        apply_impulse = new ComputeKernel(gl, {
            shader: add_field,
            mesh: all,
            uniforms: {
                px: px,
                source: velocity_pong,
                force: vec2.create([0.5, 0.2]),
                center: vec2.create([0.1, 0.4]),
                scale: vec2.create([options.cursor_size*px_x, options.cursor_size*px_y])
            },
            output: temperature_ping
        }),
        apply_scalar_impulse = new ComputeKernel(gl, {
            shader: add_scalar,
            mesh: all,
            uniforms: {
                px: px,
                source: density_pong,
                force: vec2.create([0.5, 0.2]),
                center: vec2.create([0.1, 0.4]),
                scale: vec2.create([options.cursor_size*px_x, options.cursor_size*px_y])
            },
            output: temperature_ping
        }),
        apply_divergence = new ComputeKernel(gl, {
            shader: divergence,
            mesh: all,
            uniforms: {
                velocity: velocity_ping,
                px: px
            },
            output: temperature_ping
        }),
        compute_jacobi = new ComputeKernel(gl, {
            shader: jacobi,
            mesh: all,
            uniforms: {
                pressure: pressure_ping,
                divergence: divergence_pong,
                alpha: -1.0,
                beta: 0.25,
                px: px
            },
            output: pressure_pong
        }),
        subtract_gradient = new ComputeKernel(gl, {
            shader: gradient,
            mesh: all,
            uniforms: {
                scale: 1.0,
                pressure: pressure_ping,
                velocity: velocity_ping,
                px: px
            },
            output: velocity_pong
        }),
        draw = new ComputeKernel(gl, {
            shader: visualize,
            mesh: all,
            uniforms: {
                sampler: density_ping,
                fg_color: vec3.create([0.0, 0.0, 0.0]),
                bg_color: vec3.create([1.0, 1.0, 1.0]),
                px: px
            },
            output: null
        });


    var x_0 = 0,
        y_0 = 0,
        x_1 = 0,
        y_1 = 0,
        is_cursor_down = false;


    function onMouseDown(e) {
        x_1 = e.pageX;
        y_1 = e.pageY;
        is_cursor_down = true;
    }
    function onMouseMove(e) {
        x_1 = e.pageX;
        y_1 = e.pageY;
    }
    function onMouseUp(e) {
        x_1 = e.pageX;
        y_1 = e.pageY;
        is_cursor_down = false;
    }

    document.addEventListener('mousedown', onMouseDown);
    document.addEventListener('mousemove', onMouseMove);
    document.addEventListener('mouseup', onMouseUp);

    function hexToRgb(hex) {
      var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
      if (result) {
        var r = parseInt(result[1], 16)
            g = parseInt(result[2], 16),
            b = parseInt(result[3], 16);
        return [r / 256, g / 256, b / 256];
      } else {
        return null
      }
    }

    var fg_color = vec3.create([0.0, 0.0, 0.0]),
        bg_color = vec3.create([1.0, 1.0, 1.0]);

    var fg_color_input = document.getElementById('fg-color');
    fg_color_input.onchange = function(e)  {
      fg_color = vec3.create(hexToRgb(fg_color_input.value));
    };

    var bg_color_input = document.getElementById('bg-color');
    bg_color_input.onchange = function(e) {
      bg_color = vec3.create(hexToRgb(bg_color_input.value));
    };

    var iterations_val = document.getElementById('iterations-val');
    var iterations_input = document.getElementById('iterations');
    iterations_val.innerText = options.iterations;
    iterations_input.value = options.iterations;
    iterations_input.onchange = function(e) {
      options.iterations = parseInt(iterations_input.value);
      iterations_val.innerText = options.iterations;
    };

    function swapBuffers(fbo0, fbo1) {
      swap = fbo0;
      fbo0 = fbo1;
      fbo1 = swap;
    }

    clock.ontick = function(dt) {
      advect.uniforms.source = velocity_ping;
      advect.uniforms.dissipation = 0.99;
      advect.uniforms.velocity = velocity_ping;
      advect.outputFBO = velocity_pong;
      advect.run();

      swap = velocity_ping;
      velocity_ping = velocity_pong;
      velocity_pong = swap;

      advect_sc.uniforms.source = temperature_ping;
      advect_sc.uniforms.dissipation = 0.95;
      advect_sc.uniforms.velocity = velocity_ping;
      advect_sc.outputFBO = temperature_pong;
      advect_sc.run();

      advect_sc.uniforms.source = density_ping;
      advect_sc.uniforms.dissipation = 0.999;
      advect_sc.uniforms.velocity = velocity_ping;
      advect_sc.outputFBO = density_pong;
      advect_sc.run();

      apply_buoyancy.uniforms.velocity = velocity_ping;
      apply_buoyancy.uniforms.temperature = temperature_ping;
      apply_buoyancy.uniforms.density = density_pong;
      apply_buoyancy.uniforms.gravity = vec2.create([0.0, 1.0]);
      apply_buoyancy.outputFBO = velocity_pong;
      apply_buoyancy.run();

      var xd = x_1 - x_0,
          yd = y_1 - y_0;

        x_0 = x_1,
        y_0 = y_1;

      if (!is_cursor_down) {
        xd = 0;
        yd = 0;
      }

      var force = vec2.create([
        xd * px_x * options.cursor_size * options.mouse_force,
	-yd * px_y * options.cursor_size * options.mouse_force
      ]),
      center = vec2.create([
        x_0 * px_x,
	(1 - y_0 * px_y)
      ]),
      min_d = 8.0,
      force_avg = Math.abs(xd) + Math.abs(yd);

      if (is_cursor_down) {
        force_avg = Math.max(min_d, force_avg);
      } else {
        force_avg = 0;
      }

      var size_factor = options.cursor_size * 0.004,
      module_force = vec2.create([
        force_avg * px_x * options.mouse_force / size_factor,
        force_avg * px_y * options.mouse_force / size_factor,
      ]);

      apply_impulse.uniforms.center = center;
      apply_impulse.uniforms.force = force;
      apply_impulse.uniforms.source = velocity_pong;
      apply_impulse.outputFBO = velocity_ping;
      apply_impulse.run();

      apply_scalar_impulse.uniforms.center = center;
      apply_scalar_impulse.uniforms.force = module_force;
      apply_scalar_impulse.uniforms.source = density_pong;
      apply_scalar_impulse.outputFBO = density_ping;
      apply_scalar_impulse.run();

      apply_scalar_impulse.uniforms.source = temperature_pong;
      apply_scalar_impulse.outputFBO = temperature_ping;
      apply_scalar_impulse.run();

      apply_divergence.uniforms.velocity = velocity_ping;
      apply_divergence.outputFBO = divergence_pong;
      apply_divergence.run();

      for(var i = 0; i < options.iterations; i++) {
        swap = pressure_ping;
        pressure_ping = pressure_pong;
        pressure_pong = swap;
        compute_jacobi.uniforms.divergence = divergence_pong;
        compute_jacobi.uniforms.pressure = pressure_ping;
        compute_jacobi.outputFBO = pressure_pong;
        compute_jacobi.run();
      }

      subtract_gradient.uniforms.velocity = velocity_ping;
      subtract_gradient.uniforms.pressure = pressure_ping;
      subtract_gradient.outputFBO = velocity_pong;
      subtract_gradient.run();

      swap = velocity_ping;
      velocity_ping = velocity_pong;
      velocity_pong = swap;

      draw.uniforms.fg_color = fg_color;
      draw.uniforms.bg_color = bg_color;
      draw.uniforms.sampler = density_ping;
      draw.run();

    };
}

if(gl)
loader.load([
            'js/shaders/converse.glsl',
            'js/shaders/cursor.glsl',
            'js/shaders/kernel.glsl',
            'js/shaders/advect.glsl',
            'js/shaders/advect_scalar.glsl',
            'js/shaders/jacobi.glsl',
            'js/shaders/gradient.glsl',
            'js/shaders/divergence.glsl',
            'js/shaders/buoyancy.glsl',
            'js/shaders/visualize.glsl',
            'js/shaders/add_field.glsl',
            'js/shaders/add_scalar.glsl'
], init); 

});

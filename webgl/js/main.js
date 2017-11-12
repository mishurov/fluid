define(function(require, exports, module) {


var jscolor = require('lib/jscolor.min');


var Loader = require('engine/loader'),
  Clock = require('engine/clock').Clock,
  ShaderManager = require('engine/shader').Manager,
  FBO = require('engine/texture').FBO,
  Mesh = require('engine/mesh').Mesh,
  geometry = require('engine/geometry'),
  ComputeKernel = require('./compute').Kernel;
  glm = require('engine/gl-matrix'),
  glcontext = require('engine/context'),
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


function init() {
  var rect = canvas.getBoundingClientRect(),
    width = rect.width * options.resolution,
    height = rect.height * options.resolution;

    setup(width, height);
    clock.start();
}

function setup(width, height) {
  canvas.width = width,
  canvas.height = height;

  gl.viewport(0, 0, width, height);
  gl.lineWidth(1.0);

  var px_x = 1.0/canvas.width,
    px_y = 1.0/canvas.height,
    px = vec2.create([px_x, px_y]);
    px1 = vec2.create([1.0, canvas.width/canvas.height]),
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
    });

  var advect_field = shaders.get('surface', 'advect_field'),
    add_field = shaders.get('surface', 'add_field'),
    fill_packed_zeroes = shaders.get('surface', 'fill_packed_zeroes'),
    jacobi = shaders.get('surface', 'jacobi'),
    gradient = shaders.get('surface', 'gradient'),
    divergence = shaders.get('surface', 'divergence'),
    buoyancy = shaders.get('surface', 'buoyancy'),
    visualize = shaders.get('surface', 'visualize');

  var format = gl.UNSIGNED_BYTE;
  var velocity_ping = new FBO(gl, width, height, format),
    velocity_pong = new FBO(gl, width, height, format),
    density_ping = new FBO(gl, width, height, format),
    density_pong = new FBO(gl, width, height, format),
    pressure_ping = new FBO(gl, width, height, format),
    pressure_pong = new FBO(gl, width, height, format),
    temperature_ping = new FBO(gl, width, height, format),
    temperature_pong = new FBO(gl, width, height, format),
    divergence_ping = new FBO(gl, width, height, format);

  var fill_zeroes = new ComputeKernel(gl, {
      shader: fill_packed_zeroes,
      mesh: all,
      uniforms: {
        vector_size: 1.0,
      },
      output: density_ping
    }),
    advect = new ComputeKernel(gl, {
      shader: advect_field,
      mesh: inside,
      uniforms: {
        px: px,
        px1: px1,
        dissipation: 0.999,
        vector_size: 2.0,
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
    impulse = new ComputeKernel(gl, {
      shader: add_field,
      mesh: all,
      uniforms: {
        px: px,
        source: velocity_pong,
        vector_size: 2.0,
        force: vec2.create([0.0, 0.0]),
        center: vec2.create([0.0, 0.0]),
        scale: vec2.create([options.cursor_size*px_x, options.cursor_size*px_y])
      },
      output: velocity_ping
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
        divergence: divergence_ping,
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

  function clearFbo(fbo, size) {
    fill_zeroes.uniforms.vector_size = size;
    fill_zeroes.outputFBO = fbo;
    fill_zeroes.run();
  }

  clearFbo(velocity_ping, 2.0);
  clearFbo(velocity_pong, 2.0);
  clearFbo(temperature_ping, 1.0);
  clearFbo(temperature_pong, 1.0);
  clearFbo(density_ping, 1.0);
  clearFbo(density_pong, 1.0);

  // Cursor attrs
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

  // UI controls listeners
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

  var size_val = document.getElementById('size-val');
  var size_input = document.getElementById('size');
  size_val.innerText = options.cursor_size / 10;
  size_input.value = options.cursor_size / 10;
  size_input.onchange = function(e) {
    options.cursor_size = parseInt(size_input.value) * 10;
    size_val.innerText = options.cursor_size / 10;
  };

  clock.ontick = function(dt) {
    advect.uniforms.source = velocity_ping;
    advect.uniforms.vector_size = 2.0;
    advect.uniforms.dissipation = 0.99;
    advect.uniforms.velocity = velocity_ping;
    advect.outputFBO = velocity_pong;
    advect.run();

    var swap = velocity_ping;
    velocity_ping = velocity_pong;
    velocity_pong = swap;

    advect.uniforms.vector_size = 1.0;
    advect.uniforms.source = temperature_ping;
    advect.uniforms.dissipation = 0.95;
    advect.uniforms.velocity = velocity_ping;
    advect.outputFBO = temperature_pong;
    advect.run();

    advect.uniforms.source = density_ping;
    advect.uniforms.dissipation = 0.999;
    advect.uniforms.velocity = velocity_ping;
    advect.outputFBO = density_pong;
    advect.run();

    apply_buoyancy.uniforms.velocity = velocity_ping;
    apply_buoyancy.uniforms.temperature = temperature_pong;
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
      1 - y_0 * px_y
    ]),

    // Uniform force for the scalar fields
    force_min = 7.0,
    force_avg = Math.abs(xd) + Math.abs(yd);

    if (is_cursor_down) {
      force_avg = Math.max(force_min, force_avg);
    } else {
      force_avg = 0;
    }
    // The more size, the less force
    var size_factor = options.cursor_size * 0.005,
    module_force = vec2.create([
      force_avg * px_x * options.mouse_force / size_factor,
      force_avg * px_y * options.mouse_force / size_factor,
    ]);
    var scale = vec2.create(
      [options.cursor_size*px_x, options.cursor_size*px_y]
    );

    impulse.uniforms.scale = scale;
    impulse.uniforms.center = center;
    impulse.uniforms.force = force;
    impulse.uniforms.vector_size = 2.0;
    impulse.uniforms.source = velocity_pong;
    impulse.outputFBO = velocity_ping;
    impulse.run();

    impulse.uniforms.force = module_force;
    impulse.uniforms.vector_size = 1.0;
    impulse.uniforms.source = density_pong;
    impulse.outputFBO = density_ping;
    impulse.run();

    impulse.uniforms.source = temperature_pong;
    impulse.outputFBO = temperature_ping;
    impulse.run();

    apply_divergence.uniforms.velocity = velocity_ping;
    apply_divergence.outputFBO = divergence_ping;
    apply_divergence.run();

    for(var i = 0; i < options.iterations; i++) {
      swap = pressure_ping;
      pressure_ping = pressure_pong;
      pressure_pong = swap;
      compute_jacobi.uniforms.divergence = divergence_ping;
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

// app/src/main/assets/shaders

if(gl)
loader.load([
  'js/shaders/surface.glsl',
  'js/shaders/converse.glsl',
  'js/shaders/add_field.glsl',
  'js/shaders/advect_field.glsl',
  'js/shaders/fill_packed_zeroes.glsl',
  'js/shaders/jacobi.glsl',
  'js/shaders/gradient.glsl',
  'js/shaders/divergence.glsl',
  'js/shaders/buoyancy.glsl',
  'js/shaders/visualize.glsl'
], init); 

});

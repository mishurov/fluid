precision mediump float;


// vector field
const float HALF_RANGE_V = 0.5;
const float MIN_RANGE_V = -HALF_RANGE_V;
const float MAX_RANGE_V = HALF_RANGE_V;
const float EPSILON_V = 0.;

// scalar field
const float HALF_RANGE_S = 0.99;
const float MIN_RANGE_S = -HALF_RANGE_S;
const float MAX_RANGE_S = HALF_RANGE_S;
// fix maping values out of precision
const float EPSILON_S = 0.004;

const float SHIFT_LEFT_8 = 256.0;
const float SHIFT_LEFT_16 = 65536.0;
const float SHIFT_LEFT_24 = 16777216.0;

const float SHIFT_RIGHT_8 = 1.0 / SHIFT_LEFT_8;
const float SHIFT_RIGHT_16 = 1.0 / SHIFT_LEFT_16;
const float SHIFT_RIGHT_24 = 1.0 / SHIFT_LEFT_24;

float clampToRange_v(in float val) {
    return clamp(val, MIN_RANGE_V + EPSILON_V, MAX_RANGE_V - EPSILON_V);
}

float normalise_v(in float val) {
    val = clampToRange_v(val);
    return (val - MIN_RANGE_V) / (MAX_RANGE_V - MIN_RANGE_V);
}

float unnormalise_v(in float val) {
    return val * (MAX_RANGE_V - MIN_RANGE_V) + MIN_RANGE_V;
}


float clampToRange_s(in float val) {
    return clamp(val, MIN_RANGE_S + EPSILON_S, MAX_RANGE_S - EPSILON_S);
}

float normalise_s(in float val) {
    val = clampToRange_s(val);
    return (val - MIN_RANGE_S) / (MAX_RANGE_S - MIN_RANGE_S);
}

float unnormalise_s(in float val) {
    return val * (MAX_RANGE_S - MIN_RANGE_S) + MIN_RANGE_S;
}


vec2 packFloatTo2bytes(in float val) {
    val = normalise_v(val);
    const vec2 bitSh = vec2(SHIFT_LEFT_8, 1.0);
    const vec2 bitMsk = vec2(0.0, SHIFT_RIGHT_8);
    vec2 ret = fract(val * bitSh);
    ret -= ret.xx * bitMsk;
    return ret;
}

float unpackFloatTo2bytes(in vec2 val) {
    const vec2 unshift = vec2(SHIFT_RIGHT_8, 1.0);
    float ret = dot(val, unshift);
    return unnormalise_v(ret);
}

vec4 packFloatTo4bytes(in float val) {
    val = normalise_s(val);
    const vec4 bitSh = vec4(SHIFT_LEFT_24, SHIFT_LEFT_16, SHIFT_LEFT_8, 1.0);
    const vec4 bitMsk = vec4(0.0, SHIFT_RIGHT_8, SHIFT_RIGHT_8, SHIFT_RIGHT_8);
    vec4 ret = fract(val * bitSh);
    ret -= ret.xxyz * bitMsk;
    return ret;
}

float unpackFloatTo4bytes(in vec4 val) {
    const vec4 unshift = vec4(SHIFT_RIGHT_24, SHIFT_RIGHT_16, SHIFT_RIGHT_8, 1.0);
    float ret = dot(val, unshift);
    return unnormalise_s(ret);
}

vec4 pack2FloatsTo4bytes(in vec2 val) {
    return vec4(packFloatTo2bytes(val.x), packFloatTo2bytes(val.y));
}

vec2 unpack2FloatsTo4bytes(in vec4 val) {
    return vec2(unpackFloatTo2bytes(val.xy), unpackFloatTo2bytes(val.zw));
}




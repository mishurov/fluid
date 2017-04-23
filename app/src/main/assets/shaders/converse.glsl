

// clamping range
const float HALF_RANGE = 1.1;
const float MIN_RANGE = -HALF_RANGE;
const float MAX_RANGE = HALF_RANGE;
// fix maping values out of precision
const float EPSILON = 0.0000001;

const float SHIFT_LEFT_8 = 256.0;
const float SHIFT_LEFT_16 = 65536.0;
const float SHIFT_LEFT_24 = 16777216.0;

const float SHIFT_RIGHT_8 = 1.0 / SHIFT_LEFT_8;
const float SHIFT_RIGHT_16 = 1.0 / SHIFT_LEFT_16;
const float SHIFT_RIGHT_24 = 1.0 / SHIFT_LEFT_24;

float clampToRange(in float val) {
    return clamp(MIN_RANGE + EPSILON, MAX_RANGE - EPSILON, val);
}

float normalise(in float val) {
    val = clampToRange(val);
    return (val - MIN_RANGE) / (MAX_RANGE - MIN_RANGE);
}

float unnormalise(in float val) {
    return val * (MAX_RANGE - MIN_RANGE) + MIN_RANGE;
}


vec2 packFloatTo2bytes(in float val) {
    val = normalise(val);
    const vec2 bitSh = vec2(SHIFT_LEFT_8, 1.0);
    const vec2 bitMsk = vec2(0.0, SHIFT_RIGHT_8);
    vec2 ret = fract(val * bitSh);
    ret -= ret.xx * bitMsk;
    return ret;
}

float unpackFloatTo2bytes(in vec2 val) {
    const vec2 unshift = vec2(SHIFT_RIGHT_8, 1.0);
    float ret = dot(val, unshift);
    return unnormalise(ret);

}

vec4 packFloatTo4bytes(in float val) {
    val = normalise(val);
    const vec4 bitSh = vec4(SHIFT_LEFT_24, SHIFT_LEFT_16, SHIFT_LEFT_8, 1.0);
    const vec4 bitMsk = vec4(0.0, SHIFT_RIGHT_8, SHIFT_RIGHT_8, SHIFT_RIGHT_8);
    vec4 ret = fract(val * bitSh);
    ret -= ret.xxyz * bitMsk;
    return ret;
}

float unpackFloatTo4bytes(in vec4 val) {
    const vec4 unshift = vec4(SHIFT_RIGHT_24, SHIFT_RIGHT_16, SHIFT_RIGHT_8, 1.0);
    float ret = dot(val, unshift);
    return unnormalise(ret);
}

vec4 pack2FloatsTo4bytes(in vec2 val) {
    return vec4(packFloatTo2bytes(val.x), packFloatTo2bytes(val.y));
}

vec2 unpack2FloatsTo4bytes(in vec4 val) {
    return vec2(unpackFloatTo2bytes(val.xy), unpackFloatTo2bytes(val.zw));
}





// clamping range
float half_range = 1.1;
float epsilon = 0.0000001;


vec4 pack2(in vec2 val) {
    val.x = clamp(-half_range+epsilon, half_range-epsilon, val.x);
    val.y = clamp(-half_range+epsilon, half_range-epsilon, val.y);
    float min = -half_range;
    float max = half_range;

    val = (val - min) / (max - min);
    const vec2 bitSh = vec2(256.0, 1.0);
    const vec2 bitMsk = vec2(0.0, 1.0/256.0);
    vec2 res1 = fract(val.x * bitSh);
    res1 -= res1.xx * bitMsk;
    vec2 res2 = fract(val.y * bitSh);
    res2 -= res2.xx * bitMsk;
    return vec4(res1.x,res1.y,res2.x,res2.y);
}

vec2 unpack2(in vec4 val) {
    float min = -half_range;
    float max = half_range;

    const vec2 unshift = vec2(1.0/256.0, 1.0);
    vec2 ret = vec2(dot(val.xy, unshift), dot(val.zw, unshift));
    return ret * (max - min) + min;
}

vec4 pack1(in float val) {
    val = clamp(-half_range+epsilon, half_range-epsilon, val);
    float min = -half_range;
    float max = half_range;

    val = (val - min) / (max - min);
    const vec4 bitSh = vec4(256.0*256.0*256.0, 256.0*256.0, 256.0, 1.0);
    const vec4 bitMsk = vec4(0.0, 1.0/256.0, 1.0/256.0, 1.0/256.0);
    vec4 result = fract(val * bitSh);
    result -= result.xxyz * bitMsk;
    return result;
}

float unpack1(in vec4 val) {
    float min = -half_range;
    float max = half_range;
    const vec4 unshift = vec4(1.0/(256.0*256.0*256.0), 1.0/(256.0*256.0), 1.0/256.0, 1.0);
    float ret = dot(val, unshift);
    return ret * (max - min) + min;
}


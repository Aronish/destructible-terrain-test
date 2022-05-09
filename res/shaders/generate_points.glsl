#shader comp
#version 460 core

const uint WORK_GROUP_SIZE = 10;

uniform int u_points_per_axis;
uniform int u_resolution;
uniform vec3 u_position_offset;

layout(std430, binding = 0) buffer DensityDistribution
{
    float values[];
};

layout(std140, binding = 1) uniform WorldGenerationConfig
{
    int u_octaves_2d, u_octaves_3d;
    float u_frequency_2d, u_lacunarity_2d, u_persistence_2d, u_amplitude_2d, u_exponent_2d;
    float u_frequency_3d, u_lacunarity_3d, u_persistence_3d, u_amplitude_3d, u_exponent_3d;
    float u_water_level;
};

layout(local_size_x = WORK_GROUP_SIZE, local_size_y = WORK_GROUP_SIZE, local_size_z = WORK_GROUP_SIZE) in;

// Simplex 2D noise (-1, 1)
vec3 permute(vec3 x)
{
    return mod(((x * 34.0) + 1.0) * x, 289.0);
}

float simplexNoise2d(vec2 v)
{
    const vec4 C = vec4(0.211324865405187, 0.366025403784439, -0.577350269189626, 0.024390243902439);
    vec2 i = floor(v + dot(v, C.yy));
    vec2 x0 = v - i + dot(i, C.xx);
    vec2 i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;
    i = mod(i, 289.0);
    vec3 p = permute(permute(i.y + vec3(0.0, i1.y, 1.0)) + i.x + vec3(0.0, i1.x, 1.0));
    vec3 m = max(0.5 - vec3(dot(x0, x0), dot(x12.xy ,x12.xy), dot(x12.zw, x12.zw)), 0.0);
    m = m * m;
    m = m * m;
    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;
    m *= 1.79284291400159 - 0.85373472095314 * (a0 * a0 + h * h);
    vec3 g;
    g.x = a0.x * x0.x + h.x * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}

//	Simplex 3D Noise 
//	by Ian McEwan, Ashima Arts
//
vec4 permute(vec4 x)
{
    return mod(((x * 34.0) + 1.0) * x, 289.0);
}
vec4 taylorInvSqrt(vec4 r)
{
    return 1.79284291400159 - 0.85373472095314 * r;
}

float simplexNoise3d(vec3 v)
{ 
    const vec2 C = vec2(1.0 / 6.0, 1.0 / 3.0);
    const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);

    // First corner
    vec3 i = floor(v + dot(v, C.yyy));
    vec3 x0 = v - i + dot(i, C.xxx);

    // Other corners
    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min(g.xyz, l.zxy);
    vec3 i2 = max(g.xyz, l.zxy);

    //  x0 = x0 - 0. + 0.0 * C 
    vec3 x1 = x0 - i1 + 1.0 * C.xxx;
    vec3 x2 = x0 - i2 + 2.0 * C.xxx;
    vec3 x3 = x0 - 1. + 3.0 * C.xxx;

    // Permutations
    i = mod(i, 289.0);
    vec4 p = permute(permute(permute(i.z + vec4(0.0, i1.z, i2.z, 1.0)) + i.y + vec4(0.0, i1.y, i2.y, 1.0)) + i.x + vec4(0.0, i1.x, i2.x, 1.0));

    // Gradients
    // ( N*N points uniformly over a square, mapped onto an octahedron.)
    float n_ = 1.0/7.0; // N=7
    vec3 ns = n_ * D.wyz - D.xzx;
    vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,N*N)
    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0 * x_);    // mod(j,N)
    vec4 x = x_ * ns.x + ns.yyyy;
    vec4 y = y_ * ns.x + ns.yyyy;
    vec4 h = 1.0 - abs(x) - abs(y);
    vec4 b0 = vec4(x.xy, y.xy);
    vec4 b1 = vec4(x.zw, y.zw);
    vec4 s0 = floor(b0) * 2.0 + 1.0;
    vec4 s1 = floor(b1) * 2.0 + 1.0;
    vec4 sh = -step(h, vec4(0.0));
    vec4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
    vec4 a1 = b1.xzyw + s1.xzyw * sh.zzww;
    vec3 p0 = vec3(a0.xy, h.x);
    vec3 p1 = vec3(a0.zw, h.y);
    vec3 p2 = vec3(a1.xy, h.z);
    vec3 p3 = vec3(a1.zw, h.w);
    //Normalise gradients
    vec4 norm = taylorInvSqrt(vec4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;
    // Mix final noise value
    vec4 m = max(0.6 - vec4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
    m *= m;
    return 42.0 * dot(m * m, vec4(dot(p0, x0), dot(p1, x1), dot(p2, x2), dot(p3, x3)));
}

float layeredNoise(vec2 position, int octaves, float frequency, float lacunarity, float persistence)
{
    float total_noise = 0.0f;
    float total_amplitude = 0.0f;
    float amplitude = 1.0f;
    for (int i = 0; i < octaves; ++i)
    {
        total_noise += amplitude * simplexNoise2d(position * frequency);
        total_amplitude += amplitude;
        frequency *= lacunarity;
        amplitude *= persistence;
    }
    return total_noise / total_amplitude;
}

float layeredNoise(vec3 position)
{
    float total_noise = 0.0f;
    float total_amplitude = 0.0f;
    float frequency = u_frequency_3d;
    float amplitude = 1.0f;
    for (int i = 0; i < u_octaves_3d; ++i)
    {
        total_noise += amplitude * simplexNoise3d(position * frequency);
        total_amplitude += amplitude;
        frequency *= u_lacunarity_3d;
        amplitude *= u_persistence_3d;
    }
    return total_noise / total_amplitude;
}

void main()
{
    int points_from_zero = u_points_per_axis - 1; // ppa is a count, can't be used as index
    if (gl_GlobalInvocationID.x > points_from_zero || gl_GlobalInvocationID.y > points_from_zero || gl_GlobalInvocationID.z > points_from_zero) return;
    float x = (float(gl_GlobalInvocationID.x) + u_position_offset.x * float(points_from_zero)) / u_resolution,
          y = (float(gl_GlobalInvocationID.y) + u_position_offset.y * float(points_from_zero)) / u_resolution,
          z = (float(gl_GlobalInvocationID.z) + u_position_offset.z * float(points_from_zero)) / u_resolution;

    float noise2d = u_amplitude_2d * pow(layeredNoise(vec2(x, z), u_octaves_2d, u_frequency_2d, u_lacunarity_2d, u_persistence_2d) * 0.5f + 0.5f, u_exponent_2d);
    //float ridge_test = u_amplitude_3d * pow(-abs(layeredNoise(vec2(x + 58931.4f, z -358.9f), u_octaves_3d, u_frequency_3d, u_lacunarity_3d, u_persistence_3d)) + 1.0f, u_exponent_3d);
    //float noise3d = u_amplitude_3d * pow(layeredNoise(vec3(x, y, z)) * 0.5f + 0.5f, u_exponent_3d);

    float final_height = noise2d;
    float final_density = y - final_height;
    //if (final_density < u_water_level) final_density = u_water_level;
    
    values[
        gl_GlobalInvocationID.z * u_points_per_axis * u_points_per_axis +
        gl_GlobalInvocationID.y * u_points_per_axis +
        gl_GlobalInvocationID.x
    ] = final_density;
}
#shader comp
#version 460 core

const uint WORK_GROUP_SIZE = 10;

layout(local_size_x = WORK_GROUP_SIZE, local_size_y = WORK_GROUP_SIZE, local_size_z = WORK_GROUP_SIZE) in;

vec3 mod289(vec3 x)
{
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x)
{
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x)
{
    return mod289(((x * 34.0) + 10.0) * x);
}

vec4 taylorInvSqrt(vec4 r)
{
    return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v, out vec3 gradient)
{
    const vec2 C = vec2(1.0 / 6.0, 1.0 / 3.0);
    const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);

    // First corner
    vec3 i  = floor(v + dot(v, C.yyy));
    vec3 x0 = v - i + dot(i, C.xxx);

    // Other corners
    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min( g.xyz, l.zxy );
    vec3 i2 = max( g.xyz, l.zxy );

    vec3 x1 = x0 - i1 + C.xxx;
    vec3 x2 = x0 - i2 + C.yyy; // 2.0 * C.x = 1 / 3 = C.y
    vec3 x3 = x0 - D.yyy;      // -1.0 + 3.0 * C.x = -0.5 = -D.y

    // Permutations
    i = mod289(i); 
    vec4 p = permute(permute(permute(
                i.z + vec4(0.0, i1.z, i2.z, 1.0))
            + i.y + vec4(0.0, i1.y, i2.y, 1.0)) 
            + i.x + vec4(0.0, i1.x, i2.x, 1.0));

    // Gradients: 7x7 points over a square, mapped onto an octahedron.
    // The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
    float n_ = 0.142857142857; // 1.0 / 7.0
    vec3  ns = n_ * D.wyz - D.xzx;

    vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7 * 7)

    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

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
    vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

    // Mix final noise value
    vec4 m = max(0.5 - vec4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
    vec4 m2 = m * m;
    vec4 m4 = m2 * m2;
    vec4 pdotx = vec4(dot(p0, x0), dot(p1, x1), dot(p2, x2), dot(p3, x3));

    // Determine noise gradient
    vec4 temp = m2 * m * pdotx;
    gradient = -8.0 * (temp.x * x0 + temp.y * x1 + temp.z * x2 + temp.w * x3);
    gradient += m4.x * p0 + m4.y * p1 + m4.z * p2 + m4.w * p3;
    gradient *= 105.0;

    return 105.0 * dot(m4, pdotx);
}

// QEF Solver
#define SVD_NUM_SWEEPS 5

// SVD
const float Tiny_Number = 1.e-20;

void givens_coeffs_sym(float a_pp, float a_pq, float a_qq, out float c, out float s)
{
    if (a_pq == 0.0)
    {
        c = 1.0;
        s = 0.0;
        return;
    }
    float tau = (a_qq - a_pp) / (2.0 * a_pq);
    float stt = sqrt(1.0 + tau * tau);
    float tan = 1.0 / ((tau >= 0.0) ? (tau + stt) : (tau - stt));
    c = inversesqrt(1.0 + tan * tan);
    s = tan * c;
}

void svd_rotate_xy(inout float x, inout float y, in float c, in float s)
{
    float u = x; float v = y;
    x = c * u - s * v;
    y = s * u + c * v;
}

void svd_rotateq_xy(inout float x, inout float y, inout float a, in float c, in float s)
{
    float cc = c * c; float ss = s * s;
    float mx = 2.0 * c * s * a;
    float u = x; float v = y;
    x = cc * u - mx + ss * v;
    y = ss * u + mx + cc * v;
}

void svd_rotate(inout mat3 vtav, inout mat3 v, in int a, in int b)
{
    if (vtav[a][b] == 0.0) return;
    
    float c, s;
    givens_coeffs_sym(vtav[a][a], vtav[a][b], vtav[b][b], c, s);
    svd_rotateq_xy(vtav[a][a],vtav[b][b],vtav[a][b],c,s);
    svd_rotate_xy(vtav[0][3-b], vtav[1-a][2], c, s);
    vtav[a][b] = 0.0;
    
    svd_rotate_xy(v[0][a], v[0][b], c, s);
    svd_rotate_xy(v[1][a], v[1][b], c, s);
    svd_rotate_xy(v[2][a], v[2][b], c, s);
}

void svd_solve_sym(in mat3 a, out vec3 sigma, inout mat3 v)
{
    // assuming that A is symmetric: can optimize all operations for 
    // the upper right triagonal
    mat3 vtav = a;
    // assuming V is identity: you can also pass a matrix the rotations
    // should be applied to
    // U is not computed
    for (int i = 0; i < SVD_NUM_SWEEPS; ++i)
    {
        svd_rotate(vtav, v, 0, 1);
        svd_rotate(vtav, v, 0, 2);
        svd_rotate(vtav, v, 1, 2);
    }
    sigma = vec3(vtav[0][0],vtav[1][1],vtav[2][2]);    
}

float svd_invdet(float x, float tol)
{
    return (abs(x) < tol || abs(1.0 / x) < tol) ? 0.0 : (1.0 / x);
}

void svd_pseudoinverse(out mat3 o, in vec3 sigma, in mat3 v)
{
    float d0 = svd_invdet(sigma[0], Tiny_Number);
    float d1 = svd_invdet(sigma[1], Tiny_Number);
    float d2 = svd_invdet(sigma[2], Tiny_Number);
    o = mat3(
        v[0][0] * d0 * v[0][0] + v[0][1] * d1 * v[0][1] + v[0][2] * d2 * v[0][2],
        v[0][0] * d0 * v[1][0] + v[0][1] * d1 * v[1][1] + v[0][2] * d2 * v[1][2],
        v[0][0] * d0 * v[2][0] + v[0][1] * d1 * v[2][1] + v[0][2] * d2 * v[2][2],
        v[1][0] * d0 * v[0][0] + v[1][1] * d1 * v[0][1] + v[1][2] * d2 * v[0][2],
        v[1][0] * d0 * v[1][0] + v[1][1] * d1 * v[1][1] + v[1][2] * d2 * v[1][2],
        v[1][0] * d0 * v[2][0] + v[1][1] * d1 * v[2][1] + v[1][2] * d2 * v[2][2],
        v[2][0] * d0 * v[0][0] + v[2][1] * d1 * v[0][1] + v[2][2] * d2 * v[0][2],
        v[2][0] * d0 * v[1][0] + v[2][1] * d1 * v[1][1] + v[2][2] * d2 * v[1][2],
        v[2][0] * d0 * v[2][0] + v[2][1] * d1 * v[2][1] + v[2][2] * d2 * v[2][2]
    );
}

void svd_solve_ATA_ATb(in mat3 ATA, in vec3 ATb, out vec3 x)
{
    mat3 V = mat3(1.0);
    vec3 sigma;
    
    svd_solve_sym(ATA, sigma, V);
    
    // A = UEV^T; U = A / (E*V^T)
    mat3 Vinv;
    svd_pseudoinverse(Vinv, sigma, V);
    x = Vinv * ATb;
}

vec3 svd_vmul_sym(in mat3 a, in vec3 v)
{
    return vec3
    (
        dot(a[0],v),
        (a[0][1] * v.x) + (a[1][1] * v.y) + (a[1][2] * v.z),
        (a[0][2] * v.x) + (a[1][2] * v.y) + (a[2][2] * v.z)
    );
}

void svd_mul_ata_sym(out mat3 o, in mat3 a)
{
    o[0][0] = a[0][0] * a[0][0] + a[1][0] * a[1][0] + a[2][0] * a[2][0];
    o[0][1] = a[0][0] * a[0][1] + a[1][0] * a[1][1] + a[2][0] * a[2][1];
    o[0][2] = a[0][0] * a[0][2] + a[1][0] * a[1][2] + a[2][0] * a[2][2];
    o[1][1] = a[0][1] * a[0][1] + a[1][1] * a[1][1] + a[2][1] * a[2][1];
    o[1][2] = a[0][1] * a[0][2] + a[1][1] * a[1][2] + a[2][1] * a[2][2];
    o[2][2] = a[0][2] * a[0][2] + a[1][2] * a[1][2] + a[2][2] * a[2][2];
}
    
void svd_solve_Ax_b(in mat3 a, in vec3 b, out mat3 ATA, out vec3 ATb, out vec3 x)
{
    svd_mul_ata_sym(ATA, a);
    ATb = b * a; // transpose(a) * b;
    svd_solve_ATA_ATb(ATA, ATb, x);
}

// QEF
void qef_add(in vec3 n, in vec3 p, inout mat3 ATA, inout vec3 ATb, inout vec4 pointaccum)
{
    ATA[0][0] += n.x * n.x;
    ATA[0][1] += n.x * n.y;
    ATA[0][2] += n.x * n.z;
    ATA[1][1] += n.y * n.y;
    ATA[1][2] += n.y * n.z;
    ATA[2][2] += n.z * n.z;

    float b = dot(p, n);
    ATb += n * b;
    pointaccum += vec4(p, 1.0);
}

float qef_calc_error(in mat3 A, in vec3 x, in vec3 b)
{
    vec3 vtmp = b - svd_vmul_sym(A, x);
    return dot(vtmp, vtmp);
}

float qef_solve(in mat3 ATA, in vec3 ATb, in vec4 pointaccum, out vec3 x)
{
    vec3 masspoint = pointaccum.xyz / pointaccum.w;
    ATb -= svd_vmul_sym(ATA, masspoint);
    svd_solve_ATA_ATb(ATA, ATb, x);
    float result = qef_calc_error(ATA, x, ATb);
    
    x += masspoint;
        
    return result;
}

const uint cornerIndexAFromEdge[12] =
{
    0, 1, 5, 4, 2, 3, 7, 6, 0, 1, 5, 4
};

const uint cornerIndexBFromEdge[12] =
{
    1, 5, 4, 0, 3, 7, 6, 2, 2, 3, 7, 6
};

layout (std430, binding = 0) buffer DensityDistribution
{
    readonly float values[];
};

layout (std430, binding = 1) buffer Vertices
{
    vec4 vertices[];
};

layout (binding = 2) buffer IndirectDraw
{
    uint vertex_count, prim_count, base_vertex, base_instance;
};

vec3 sphereGradTemp(vec3 pos)
{
    return 2.0f * (pos - 4.0f);
}

vec3 getNormalAtEdge(vec4 corner_a, vec4 corner_b, out vec3 interpolated)
{
    if (corner_a.w < 0.0f != corner_b.w < 0.0f)
    {
        float t = -corner_a.w / (corner_b.w - corner_a.w);
        interpolated = corner_a.xyz + t * (corner_b.xyz - corner_a.xyz);
        return normalize(sphereGradTemp(interpolated));
    }
    return vec3(0.0f);
}

uint getPointIndex(uint x, uint y, uint z)
{
	return x + y * WORK_GROUP_SIZE + z * WORK_GROUP_SIZE * WORK_GROUP_SIZE;
}

uint getCellIndex(uint x, uint y, uint z)
{
	return x + y * (WORK_GROUP_SIZE - 1) + z * (WORK_GROUP_SIZE - 1) * (WORK_GROUP_SIZE - 1);
}

void main()
{
    const uint points_from_zero = WORK_GROUP_SIZE - 1;
	if (gl_GlobalInvocationID.x > points_from_zero - 1 || gl_GlobalInvocationID.y > points_from_zero - 1 || gl_GlobalInvocationID.z > points_from_zero - 1) return;
    const uint x = gl_GlobalInvocationID.x, y = gl_GlobalInvocationID.y, z = gl_GlobalInvocationID.z;

    const vec4 cube_corners[8] =
    {
        vec4(x,     y,      z,      values[getPointIndex(x,     y,      z)      ]),
        vec4(x,     y,      z + 1,  values[getPointIndex(x,     y,      z + 1)  ]),
        vec4(x,     y + 1,  z,      values[getPointIndex(x,     y + 1,  z)      ]),
        vec4(x,     y + 1,  z + 1,  values[getPointIndex(x,     y + 1,  z + 1)  ]),
        vec4(x + 1, y,      z,      values[getPointIndex(x + 1, y,      z)      ]),
        vec4(x + 1, y,      z + 1,  values[getPointIndex(x + 1, y,      z + 1)  ]),
        vec4(x + 1, y + 1,  z,      values[getPointIndex(x + 1, y + 1,  z)      ]),
        vec4(x + 1, y + 1,  z + 1,  values[getPointIndex(x + 1, y + 1,  z + 1)  ])
    };

    vec3 normals[12];
    vec3 positions[12];
    // QEF Data
    vec4 pointaccum = vec4(0.0f);
    mat3 ATA = mat3(0.0f);
    vec3 ATb = vec3(0.0f);
    uint valid_normal_count = 0;
    for (uint i = 0; i < 12; ++i)
    {
        normals[i] = getNormalAtEdge(cube_corners[cornerIndexAFromEdge[i]], cube_corners[cornerIndexBFromEdge[i]], positions[i]);
        if (length(normals[i]) > 0.0f)
        {
            ++valid_normal_count;
            qef_add(normals[i], positions[i], ATA, ATb, pointaccum);
        }
    }
#if 0 // SUCKS
    // Bias
    const float bias_strength = 0.01f;
    vec3 mass_point = vec3(0.0f);
    for (uint i = 0; i < 12; ++i) mass_point += positions[i];
    mass_point /= 12.0f;
    qef_add(vec3(bias_strength, 0.0f, 0.0f), mass_point, ATA, ATb, pointaccum);
    qef_add(vec3(0.0f, bias_strength, 0.0f), mass_point, ATA, ATb, pointaccum);
    qef_add(vec3(0.0f, 0.0f, bias_strength), mass_point, ATA, ATb, pointaccum);
#endif
    // Solve QEF
    if (valid_normal_count > 0)
    {
        vec3 com = pointaccum.xyz / pointaccum.w;
        vec3 x_q = vec3(0.0f);
        qef_solve(ATA, ATb, pointaccum, x_q);
        vertices[getCellIndex(x, y, z)] = vec4(x_q, 1.0f);
    } else vertices[getCellIndex(x, y, z)] = vec4(0.0f);
}
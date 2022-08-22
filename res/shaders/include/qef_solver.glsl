#define SVD_NUM_SWEEPS 5

// Single Value Decomposition for calculating pseudo-inverses and solving the matrix equation Ax=B for x
const float Tiny_Number = 1.e-20;

// SVD 3D
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

// SVD 2D
void svd_pseudoinverse(out mat2 o, in vec2 sigma, in mat2 v)
{
    float d0 = svd_invdet(sigma[0], Tiny_Number);
    float d1 = svd_invdet(sigma[1], Tiny_Number);
    o = mat2(
        v[0][0] * d0 * v[0][0] + v[0][1] * d1 * v[0][1],
        v[0][0] * d0 * v[1][0] + v[0][1] * d1 * v[1][1],
        v[1][0] * d0 * v[0][0] + v[1][1] * d1 * v[0][1],
        v[1][0] * d0 * v[1][0] + v[1][1] * d1 * v[1][1]
    );
}

void svd_rotate(inout mat2 vtav, inout mat2 v, in int a, in int b)
{
    if (vtav[a][b] == 0.0) return;
    
    float c, s;
    givens_coeffs_sym(vtav[a][a], vtav[a][b], vtav[b][b], c, s);
    svd_rotateq_xy(vtav[a][a],vtav[b][b],vtav[a][b],c,s);
    svd_rotate_xy(vtav[0][2-b], vtav[1-a][1], c, s);
    vtav[a][b] = 0.0;
    
    svd_rotate_xy(v[0][a], v[0][b], c, s);
    svd_rotate_xy(v[1][a], v[1][b], c, s);
}

void svd_solve_sym(in mat2 a, out vec2 sigma, inout mat2 v)
{
    // assuming that A is symmetric: can optimize all operations for 
    // the upper right triagonal
    mat2 vtav = a;
    // assuming V is identity: you can also pass a matrix the rotations
    // should be applied to
    // U is not computed
    for (int i = 0; i < SVD_NUM_SWEEPS; ++i)
    {
        svd_rotate(vtav, v, 0, 1);
    }
    sigma = vec2(vtav[0][0], vtav[1][1]);    
}

void svd_solve_ATA_ATb(in mat2 ATA, in vec2 ATb, out vec2 x)
{
    mat2 V = mat2(1.0);
    vec2 sigma;
    
    svd_solve_sym(ATA, sigma, V);
    
    // A = UEV^T; U = A / (E*V^T)
    mat2 Vinv;
    svd_pseudoinverse(Vinv, sigma, V);
    x = Vinv * ATb;
}

// QEF
struct QEF3D
{
    mat3 ATA;
    vec3 ATb;
    vec4 point_accum;
};

struct QEF2D
{
    mat2 ATA;
    vec2 ATb;
};

QEF2D qef_fix_x(QEF3D qef, float value)
{
    mat2 ATA = mat2(
        qef.ATA[1][1], qef.ATA[1][2],
        qef.ATA[2][1], qef.ATA[2][2]
    );
    vec2 ATb = vec2(qef.ATb[1] - qef.ATA[0][1] * value, qef.ATb[2] - qef.ATA[0][2] * value);
    return QEF2D(ATA, ATb);
}

void qef_add(in vec3 n, in vec3 p, inout QEF3D qef)
{
    qef.ATA[0][0] += n.x * n.x;
    qef.ATA[0][1] += n.x * n.y;
    qef.ATA[0][2] += n.x * n.z;
    qef.ATA[1][1] += n.y * n.y;
    qef.ATA[1][2] += n.y * n.z;
    qef.ATA[2][2] += n.z * n.z;

    float b = dot(p, n);
    qef.ATb += n * b;
    qef.point_accum += vec4(p, 1.0);
}

float qef_calc_error(in mat3 A, in vec3 x, in vec3 b)
{
    vec3 vtmp = b - svd_vmul_sym(A, x);
    return dot(vtmp, vtmp);
}

float qef_solve(in QEF3D qef, out vec3 x)
{
    vec3 masspoint = qef.point_accum.xyz / qef.point_accum.w;
    qef.ATb -= svd_vmul_sym(qef.ATA, masspoint);
    svd_solve_ATA_ATb(qef.ATA, qef.ATb, x);
    float result = qef_calc_error(qef.ATA, x, qef.ATb);
    
    x += masspoint;

    return result;
}

void qef_solve(in QEF2D qef, out vec2 x)
{
    svd_solve_ATA_ATb(qef.ATA, qef.ATb, x);
}
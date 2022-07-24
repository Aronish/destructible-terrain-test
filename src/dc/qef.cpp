#include <math.h>
#include <glm/glm.hpp>

#define SVD_NUM_SWEEPS 5

namespace eng
{
    float rsqrt(float x)
    {
        return 1.0f / sqrt(x);
    }

    constexpr float Tiny_Number = 1.e-20f;

    void givens_coeffs_sym(float a_pp, float a_pq, float a_qq, float & c, float & s)
    {
        if (a_pq == 0.0)
        {
            c = 1.0;
            s = 0.0;
            return;
        }
        float tau = (a_qq - a_pp) / (2.0f * a_pq);
        float stt = sqrt(1.0f + tau * tau);
        float tan = 1.0f / ((tau >= 0.0f) ? (tau + stt) : (tau - stt));
        c = rsqrt(1.0f + tan * tan);
        s = tan * c;
    }

    void svd_rotate_xy(float & x, float & y, float c, float s)
    {
        float u = x, v = y;
        x = c * u - s * v;
        y = s * u + c * v;
    }

    void svd_rotateq_xy(float & x, float & y, float & a, float c, float s)
    {
        float cc = c * c, ss = s * s;
        float mx = 2.0f * c * s * a;
        float u = x, v = y;
        x = cc * u - mx + ss * v;
        y = ss * u + mx + cc * v;
    }

    void svd_rotate(glm::mat3 & vtav, glm::mat3 & v, int a, int b)
    {
        if (vtav[a][b] == 0.0) return;

        float c, s;
        givens_coeffs_sym(vtav[a][a], vtav[a][b], vtav[b][b], c, s);
        svd_rotateq_xy(vtav[a][a], vtav[b][b], vtav[a][b], c, s);
        svd_rotate_xy(vtav[0][3 - b], vtav[1 - a][2], c, s);
        vtav[a][b] = 0.0;

        svd_rotate_xy(v[0][a], v[0][b], c, s);
        svd_rotate_xy(v[1][a], v[1][b], c, s);
        svd_rotate_xy(v[2][a], v[2][b], c, s);
    }

    void svd_solve_sym(glm::mat3 a, glm::vec3 & sigma, glm::mat3 & v)
    {
        // assuming that A is symmetric: can optimize all operations for 
        // the upper right triagonal
        glm::mat3 vtav = a;
        // assuming V is identity: you can also pass a matrix the rotations
        // should be applied to
        // U is not computed
        for (int i = 0; i < SVD_NUM_SWEEPS; ++i)
        {
            svd_rotate(vtav, v, 0, 1);
            svd_rotate(vtav, v, 0, 2);
            svd_rotate(vtav, v, 1, 2);
        }
        sigma = glm::vec3(vtav[0][0], vtav[1][1], vtav[2][2]);
    }

    float svd_invdet(float x, float tol)
    {
        return (abs(x) < tol || abs(1.0f / x) < tol) ? 0.0f : (1.0f / x);
    }

    void svd_pseudoinverse(glm::mat3 & o, glm::vec3 sigma, glm::mat3 v)
    {
        float d0 = svd_invdet(sigma[0], Tiny_Number);
        float d1 = svd_invdet(sigma[1], Tiny_Number);
        float d2 = svd_invdet(sigma[2], Tiny_Number);
        o = glm::mat3(v[0][0] * d0 * v[0][0] + v[0][1] * d1 * v[0][1] + v[0][2] * d2 * v[0][2],
                      v[0][0] * d0 * v[1][0] + v[0][1] * d1 * v[1][1] + v[0][2] * d2 * v[1][2],
                      v[0][0] * d0 * v[2][0] + v[0][1] * d1 * v[2][1] + v[0][2] * d2 * v[2][2],
                      v[1][0] * d0 * v[0][0] + v[1][1] * d1 * v[0][1] + v[1][2] * d2 * v[0][2],
                      v[1][0] * d0 * v[1][0] + v[1][1] * d1 * v[1][1] + v[1][2] * d2 * v[1][2],
                      v[1][0] * d0 * v[2][0] + v[1][1] * d1 * v[2][1] + v[1][2] * d2 * v[2][2],
                      v[2][0] * d0 * v[0][0] + v[2][1] * d1 * v[0][1] + v[2][2] * d2 * v[0][2],
                      v[2][0] * d0 * v[1][0] + v[2][1] * d1 * v[1][1] + v[2][2] * d2 * v[1][2],
                      v[2][0] * d0 * v[2][0] + v[2][1] * d1 * v[2][1] + v[2][2] * d2 * v[2][2]);
    }

    void svd_solve_ATA_ATb(glm::mat3 ATA, glm::vec3 ATb, glm::vec3 & x)
    {
        glm::mat3 V = glm::mat3(1.0);
        glm::vec3 sigma;

        svd_solve_sym(ATA, sigma, V);

        // A = UEV^T; U = A / (E*V^T)
        glm::mat3 Vinv;
        svd_pseudoinverse(Vinv, sigma, V);
        x = Vinv * ATb;
    }

    glm::vec3 svd_vmul_sym(glm::mat3 a, glm::vec3 v)
    {
        return glm::vec3(
            dot(a[0], v),
            (a[0][1] * v.x) + (a[1][1] * v.y) + (a[1][2] * v.z),
            (a[0][2] * v.x) + (a[1][2] * v.y) + (a[2][2] * v.z)
        );
    }

    void svd_mul_ata_sym(glm::mat3 & o, glm::mat3 a)
    {
        o[0][0] = a[0][0] * a[0][0] + a[1][0] * a[1][0] + a[2][0] * a[2][0];
        o[0][1] = a[0][0] * a[0][1] + a[1][0] * a[1][1] + a[2][0] * a[2][1];
        o[0][2] = a[0][0] * a[0][2] + a[1][0] * a[1][2] + a[2][0] * a[2][2];
        o[1][1] = a[0][1] * a[0][1] + a[1][1] * a[1][1] + a[2][1] * a[2][1];
        o[1][2] = a[0][1] * a[0][2] + a[1][1] * a[1][2] + a[2][1] * a[2][2];
        o[2][2] = a[0][2] * a[0][2] + a[1][2] * a[1][2] + a[2][2] * a[2][2];
    }

    void svd_solve_Ax_b(glm::mat3 a, glm::vec3 b, glm::mat3 & ATA, glm::vec3 & ATb, glm::vec3 & x)
    {
        svd_mul_ata_sym(ATA, a);
        ATb = b * a; // transpose(a) * b;
        svd_solve_ATA_ATb(ATA, ATb, x);
    }

    // QEF
    ////////////////////////////////////////////////////////////////////////////////

    void qef_add(glm::vec3 n, glm::vec3 p, glm::mat3 & ATA, glm::vec3 & ATb, glm::vec4 & pointaccum)
    {
        ATA[0][0] += n.x * n.x;
        ATA[0][1] += n.x * n.y;
        ATA[0][2] += n.x * n.z;
        ATA[1][1] += n.y * n.y;
        ATA[1][2] += n.y * n.z;
        ATA[2][2] += n.z * n.z;

        float b = dot(p, n);
        ATb += n * b;
        pointaccum += glm::vec4(p, 1.0);
    }

    float qef_calc_error(glm::mat3 A, glm::vec3 x, glm::vec3 b)
    {
        glm::vec3 vtmp = b - svd_vmul_sym(A, x);
        return dot(vtmp, vtmp);
    }

    float qef_solve(glm::mat3 ATA, glm::vec3 ATb, glm::vec4 pointaccum, glm::vec3 & x)
    {
        glm::vec3 masspoint = glm::vec3(pointaccum) / pointaccum.w;
        ATb -= svd_vmul_sym(ATA, masspoint);
        svd_solve_ATA_ATb(ATA, ATb, x);
        float result = qef_calc_error(ATA, x, ATb);

        x += masspoint;

        return result;
    }

    // Test
    ////////////////////////////////////////////////////////////////////////////////
#if 0
    int main(void)
    {
        glm::vec4 pointaccum = glm::vec4(0.0);
        glm::mat3 ATA = glm::mat3(0.0);
        glm::vec3 ATb = glm::vec3(0.0);

#define COUNT 5
        glm::vec3 normals[COUNT] =
        {
            normalize(glm::vec3(1.0,1.0,0.0)),
            normalize(glm::vec3(1.0,1.0,0.0)),
            normalize(glm::vec3(-1.0,1.0,0.0)),
            normalize(glm::vec3(-1.0,2.0,1.0)),
            //normalize(glm::vec3(-1.0,1.0,0.0)),
            normalize(glm::vec3(-1.0,1.0,0.0)),
        };
        glm::vec3 points[COUNT] =
        {
            glm::vec3(1.0,0.0,0.3),
            glm::vec3(0.9,0.1,-0.5),
            glm::vec3(-0.8,0.2,0.6),
            glm::vec3(-1.0,0.0,0.01),
            glm::vec3(-1.1,-0.1,-0.5),
        };

        for (int i = 0; i < COUNT; ++i)
        {
            qef_add(normals[i], points[i], ATA, ATb, pointaccum);
        }
        glm::vec3 com = glm::vec3(pointaccum) / pointaccum.w;

        glm::vec3 x;
        float error = qef_solve(ATA, ATb, pointaccum, x);

        printf("masspoint = (%.5f %.5f %.5f)\n", com.x, com.y, com.z);
        printf("point = (%.5f %.5f %.5f)\n", x.x, x.y, x.z);
        printf("error = %.5f\n", error);

        return 0;
    }
#endif
}
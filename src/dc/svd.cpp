#include <cmath>

#include "svd.hpp"

namespace eng::svd
{
    Mat3::Mat3()
    {
        this->clear();
    }

    Mat3::Mat3(float const m00, float const m01, float const m02, float const m10, float const m11, float const m12, float const m20, float const m21, float const m22)
    {
        this->set(m00, m01, m02, m10, m11, m12, m20, m21, m22);
    }

    Mat3::Mat3(Mat3 const & rhs)
    {
        this->set(rhs);
    }

    void Mat3::clear()
    {
        this->set(0, 0, 0, 0, 0, 0, 0, 0, 0);
    }

    void Mat3::set(float const m00, float const m01, float const m02,
                   float const m10, float const m11, float const m12,
                   float const m20, float const m21, float const m22)
    {
        this->m00 = m00;
        this->m01 = m01;
        this->m02 = m02;
        this->m10 = m10;
        this->m11 = m11;
        this->m12 = m12;
        this->m20 = m20;
        this->m21 = m21;
        this->m22 = m22;
    }

    void Mat3::set(Mat3 const & rhs)
    {
        this->set(rhs.m00, rhs.m01, rhs.m02, rhs.m10, rhs.m11, rhs.m12, rhs.m20,
                  rhs.m21, rhs.m22);
    }

    void Mat3::setSymmetric(SMat3 const & rhs)
    {
        this->setSymmetric(rhs.m00, rhs.m01, rhs.m02, rhs.m11, rhs.m12, rhs.m22);
    }

    void Mat3::setSymmetric(float const a00, float const a01, float const a02,
                            float const a11, float const a12, float const a22)
    {
        this->set(a00, a01, a02, a01, a11, a12, a02, a12, a22);
    }

    SMat3::SMat3()
    {
        this->clear();
    }

    SMat3::SMat3(float const m00, float const m01, float const m02,
                 float const m11, float const m12, float const m22)
    {
        this->setSymmetric(m00, m01, m02, m11, m12, m22);
    }

    SMat3::SMat3(SMat3 const & rhs)
    {
        this->setSymmetric(rhs);
    }

    void SMat3::clear()
    {
        this->setSymmetric(0, 0, 0, 0, 0, 0);
    }

    void SMat3::setSymmetric(SMat3 const & rhs)
    {
        this->setSymmetric(rhs.m00, rhs.m01, rhs.m02, rhs.m11, rhs.m12, rhs.m22);
    }

    void SMat3::setSymmetric(float const a00, float const a01, float const a02,
                             float const a11, float const a12, float const a22)
    {
        this->m00 = a00;
        this->m01 = a01;
        this->m02 = a02;
        this->m11 = a11;
        this->m12 = a12;
        this->m22 = a22;
    }

    Vec3::Vec3() : x(0), y(0), z(0) { }

    Vec3::Vec3(Vec3 const & rhs)
    {
        set(rhs);
    }

    Vec3::Vec3(float const x, float const y, float const z)
    {
        set(x, y, z);
    }

    void Vec3::clear()
    {
        set(0, 0, 0);
    }

    void Vec3::set(float const x, float const y, float const z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    void Vec3::set(Vec3 const & rhs)
    {
        this->set(rhs.x, rhs.y, rhs.z);
    }

    float MatrixUtils::fnorm(Mat3 const & a)
    {
        return sqrt((a.m00 * a.m00) + (a.m01 * a.m01) + (a.m02 * a.m02) + (a.m10 * a.m10) + (a.m11 * a.m11) + (a.m12 * a.m12) + (a.m20 * a.m20) + (a.m21 * a.m21) + (a.m22 * a.m22));
    }

    float MatrixUtils::fnorm(SMat3 const & a)
    {
        return sqrt((a.m00 * a.m00) + (a.m01 * a.m01) + (a.m02 * a.m02) + (a.m01 * a.m01) + (a.m11 * a.m11) + (a.m12 * a.m12) + (a.m02 * a.m02) + (a.m12 * a.m12) + (a.m22 * a.m22));
    }

    float MatrixUtils::off(Mat3 const & a)
    {
        return sqrt((a.m01 * a.m01) + (a.m02 * a.m02) + (a.m10 * a.m10) + (a.m12 * a.m12) + (a.m20 * a.m20) + (a.m21 * a.m21));
    }

    float MatrixUtils::off(SMat3 const & a)
    {
        return sqrt(2 * ((a.m01 * a.m01) + (a.m02 * a.m02) + (a.m12 * a.m12)));
    }

    void MatrixUtils::mmul(Mat3 & out, Mat3 const & a, Mat3 const & b)
    {
        out.set(a.m00 * b.m00 + a.m01 * b.m10 + a.m02 * b.m20,
                a.m00 * b.m01 + a.m01 * b.m11 + a.m02 * b.m21,
                a.m00 * b.m02 + a.m01 * b.m12 + a.m02 * b.m22,
                a.m10 * b.m00 + a.m11 * b.m10 + a.m12 * b.m20,
                a.m10 * b.m01 + a.m11 * b.m11 + a.m12 * b.m21,
                a.m10 * b.m02 + a.m11 * b.m12 + a.m12 * b.m22,
                a.m20 * b.m00 + a.m21 * b.m10 + a.m22 * b.m20,
                a.m20 * b.m01 + a.m21 * b.m11 + a.m22 * b.m21,
                a.m20 * b.m02 + a.m21 * b.m12 + a.m22 * b.m22);
    }

    void MatrixUtils::mmul_ata(SMat3 & out, Mat3 const & a)
    {
        out.setSymmetric(a.m00 * a.m00 + a.m10 * a.m10 + a.m20 * a.m20,
                         a.m00 * a.m01 + a.m10 * a.m11 + a.m20 * a.m21,
                         a.m00 * a.m02 + a.m10 * a.m12 + a.m20 * a.m22,
                         a.m01 * a.m01 + a.m11 * a.m11 + a.m21 * a.m21,
                         a.m01 * a.m02 + a.m11 * a.m12 + a.m21 * a.m22,
                         a.m02 * a.m02 + a.m12 * a.m12 + a.m22 * a.m22);
    }

    void MatrixUtils::transpose(Mat3 & out, Mat3 const & a)
    {
        out.set(a.m00, a.m10, a.m20, a.m01, a.m11, a.m21, a.m02, a.m12, a.m22);
    }

    void MatrixUtils::vmul(Vec3 & out, Mat3 const & a, Vec3 const & v)
    {
        out.x = (a.m00 * v.x) + (a.m01 * v.y) + (a.m02 * v.z);
        out.y = (a.m10 * v.x) + (a.m11 * v.y) + (a.m12 * v.z);
        out.z = (a.m20 * v.x) + (a.m21 * v.y) + (a.m22 * v.z);
    }

    void MatrixUtils::vmul_symmetric(Vec3 & out, SMat3 const & a, Vec3 const & v)
    {
        out.x = (a.m00 * v.x) + (a.m01 * v.y) + (a.m02 * v.z);
        out.y = (a.m01 * v.x) + (a.m11 * v.y) + (a.m12 * v.z);
        out.z = (a.m02 * v.x) + (a.m12 * v.y) + (a.m22 * v.z);
    }

    void VectorUtils::addScaled(Vec3 & v, float const s, Vec3 const & rhs)
    {
        v.x += s * rhs.x;
        v.y += s * rhs.y;
        v.z += s * rhs.z;
    }

    float VectorUtils::dot(Vec3 const & a, Vec3 const & b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    void VectorUtils::normalize(Vec3 & v)
    {
        float const len2 = VectorUtils::dot(v, v);

        if (fabs(len2) < 1e-12)
        {
            v.clear();
        }
        else
        {
            VectorUtils::scale(v, 1 / sqrt(len2));
        }
    }

    void VectorUtils::scale(Vec3 & v, float const s)
    {
        v.x *= s;
        v.y *= s;
        v.z *= s;
    }

    void VectorUtils::sub(Vec3 & c, Vec3 const & a, Vec3 const & b)
    {
        float const v0 = a.x - b.x;
        float const v1 = a.y - b.y;
        float const v2 = a.z - b.z;
        c.x = v0;
        c.y = v1;
        c.z = v2;
    }

    void Givens::rot01_post(Mat3 & m, float const c, float const s)
    {
        float const m00 = m.m00, m01 = m.m01, m10 = m.m10, m11 = m.m11, m20 = m.m20, m21 = m.m21;
        m.set(c * m00 - s * m01, s * m00 + c * m01, m.m02, c * m10 - s * m11, s * m10 + c * m11, m.m12, c * m20 - s * m21, s * m20 + c * m21, m.m22);
    }

    void Givens::rot02_post(Mat3 & m, float const c, float const s)
    {
        float const m00 = m.m00, m02 = m.m02, m10 = m.m10, m12 = m.m12, m20 = m.m20, m22 = m.m22;
        m.set(c * m00 - s * m02, m.m01, s * m00 + c * m02, c * m10 - s * m12, m.m11, s * m10 + c * m12, c * m20 - s * m22, m.m21, s * m20 + c * m22);
    }

    void Givens::rot12_post(Mat3 & m, float const c, float const s)
    {
        float const m01 = m.m01, m02 = m.m02, m11 = m.m11, m12 = m.m12, m21 = m.m21, m22 = m.m22;
        m.set(m.m00, c * m01 - s * m02, s * m01 + c * m02, m.m10, c * m11 - s * m12, s * m11 + c * m12, m.m20, c * m21 - s * m22, s * m21 + c * m22);
    }

    static void calcSymmetricGivensCoefficients(float const a_pp, float const a_pq, float const a_qq, float & c, float & s)
    {
        if (a_pq == 0)
        {
            c = 1;
            s = 0;
            return;
        }
        float const tau = (a_qq - a_pp) / (2 * a_pq);
        float const stt = sqrt(1.0f + tau * tau);
        float const tan = 1.0f / ((tau >= 0) ? (tau + stt) : (tau - stt));
        c = 1.0f / sqrt(1.f + tan * tan);
        s = tan * c;
    }

    void Schur2::rot01(SMat3 & m, float & c, float & s)
    {
        svd::calcSymmetricGivensCoefficients(m.m00, m.m01, m.m11, c, s);
        float const cc = c * c;
        float const ss = s * s;
        float const mix = 2 * c * s * m.m01;
        m.setSymmetric(cc * m.m00 - mix + ss * m.m11, 0, c * m.m02 - s * m.m12, ss * m.m00 + mix + cc * m.m11, s * m.m02 + c * m.m12, m.m22);
    }

    void Schur2::rot02(SMat3 & m, float & c, float & s)
    {
        svd::calcSymmetricGivensCoefficients(m.m00, m.m02, m.m22, c, s);
        float const cc = c * c;
        float const ss = s * s;
        float const mix = 2 * c * s * m.m02;
        m.setSymmetric(cc * m.m00 - mix + ss * m.m22, c * m.m01 - s * m.m12, 0, m.m11, s * m.m01 + c * m.m12, ss * m.m00 + mix + cc * m.m22);
    }

    void Schur2::rot12(SMat3 & m, float & c, float & s)
    {
        svd::calcSymmetricGivensCoefficients(m.m11, m.m12, m.m22, c, s);
        float const cc = c * c;
        float const ss = s * s;
        float const mix = 2 * c * s * m.m12;
        m.setSymmetric(m.m00, c * m.m01 - s * m.m02, s * m.m01 + c * m.m02, cc * m.m11 - mix + ss * m.m22, 0, ss * m.m11 + mix + cc * m.m22);
    }

    static void rotate01(SMat3 & vtav, Mat3 & v)
    {
        if (vtav.m01 == 0)
        {
            return;
        }

        float c, s;
        Schur2::rot01(vtav, c, s);
        Givens::rot01_post(v, c, s);
    }

    static void rotate02(SMat3 & vtav, Mat3 & v)
    {
        if (vtav.m02 == 0)
        {
            return;
        }

        float c, s;
        Schur2::rot02(vtav, c, s);
        Givens::rot02_post(v, c, s);
    }

    static void rotate12(SMat3 & vtav, Mat3 & v)
    {
        if (vtav.m12 == 0) {
            return;
        }

        float c, s;
        Schur2::rot12(vtav, c, s);
        Givens::rot12_post(v, c, s);
    }

    void Svd::getSymmetricSvd(SMat3 const & a, SMat3 & vtav, Mat3 & v, float const tol, const int max_sweeps)
    {
        vtav.setSymmetric(a);
        v.set(1, 0, 0, 0, 1, 0, 0, 0, 1);
        float const delta = tol * MatrixUtils::fnorm(vtav);

        for (int i = 0; i < max_sweeps && MatrixUtils::off(vtav) > delta; ++i)
        {
            rotate01(vtav, v);
            rotate02(vtav, v);
            rotate12(vtav, v);
        }
    }

    static float calcError(Mat3 const & A, Vec3 const & x, Vec3 const & b)
    {
        Vec3 vtmp;
        MatrixUtils::vmul(vtmp, A, x);
        VectorUtils::sub(vtmp, b, vtmp);
        return VectorUtils::dot(vtmp, vtmp);
    }

    static float calcError(SMat3 const & origA, Vec3 const & x, Vec3 const & b)
    {
        Mat3 A;
        Vec3 vtmp;
        A.setSymmetric(origA);
        MatrixUtils::vmul(vtmp, A, x);
        VectorUtils::sub(vtmp, b, vtmp);
        return VectorUtils::dot(vtmp, vtmp);
    }

    static float pinv(float const x, float const tol)
    {
        return (fabs(x) < tol || fabs(1 / x) < tol) ? 0 : (1 / x);
    }

    void Svd::pseudoinverse(Mat3 & out, SMat3 const & d, Mat3 const & v, float const tol)
    {
        float const d0 = pinv(d.m00, tol), d1 = pinv(d.m11, tol), d2 = pinv(d.m22, tol);
        out.set(v.m00 * d0 * v.m00 + v.m01 * d1 * v.m01 + v.m02 * d2 * v.m02,
                v.m00 * d0 * v.m10 + v.m01 * d1 * v.m11 + v.m02 * d2 * v.m12,
                v.m00 * d0 * v.m20 + v.m01 * d1 * v.m21 + v.m02 * d2 * v.m22,
                v.m10 * d0 * v.m00 + v.m11 * d1 * v.m01 + v.m12 * d2 * v.m02,
                v.m10 * d0 * v.m10 + v.m11 * d1 * v.m11 + v.m12 * d2 * v.m12,
                v.m10 * d0 * v.m20 + v.m11 * d1 * v.m21 + v.m12 * d2 * v.m22,
                v.m20 * d0 * v.m00 + v.m21 * d1 * v.m01 + v.m22 * d2 * v.m02,
                v.m20 * d0 * v.m10 + v.m21 * d1 * v.m11 + v.m22 * d2 * v.m12,
                v.m20 * d0 * v.m20 + v.m21 * d1 * v.m21 + v.m22 * d2 * v.m22);
    }

    float Svd::solveSymmetric(SMat3 const & A, Vec3 const & b, Vec3 & x, float const svd_tol, const int svd_sweeps, float const pinv_tol)
    {
        Mat3 mtmp, pinv, V;
        SMat3 VTAV;
        Svd::getSymmetricSvd(A, VTAV, V, svd_tol, svd_sweeps);
        Svd::pseudoinverse(pinv, VTAV, V, pinv_tol);
        MatrixUtils::vmul(x, pinv, b);
        return svd::calcError(A, x, b);
    }

    float LeastSquares::solveLeastSquares(Mat3 const & a, Vec3 const & b, Vec3 & x, float const svd_tol, const int svd_sweeps, float const pinv_tol)
    {
        Mat3 at;
        SMat3 ata;
        Vec3 atb;
        MatrixUtils::transpose(at, a);
        MatrixUtils::mmul_ata(ata, a);
        MatrixUtils::vmul(atb, at, b);
        return Svd::solveSymmetric(ata, atb, x, svd_tol, svd_sweeps, pinv_tol);
    }
}
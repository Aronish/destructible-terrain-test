#pragma once

namespace eng::svd
{
    class SMat3
    {
    public:
        float m00, m01, m02, m11, m12, m22;
    public:
        SMat3();
        SMat3(float const m00, float const m01, float const m02, float const m11, float const m12, float const m22);

        void clear();
        void setSymmetric(float const m00, float const m01, float const m02, float const m11, float const m12, float const m22);
        void setSymmetric(SMat3 const & rhs);
    private:
        SMat3(SMat3 const & rhs);
        SMat3 & operator=(SMat3 const & rhs);
    };

    class Mat3
    {
    public:
        float m00, m01, m02, m10, m11, m12, m20, m21, m22;
    public:
        Mat3();
        Mat3(float const m00, float const m01, float const m02, float const m10, float const m11, float const m12, float const m20, float const m21, float const m22);
        void clear();
        void set(float const m00, float const m01, float const m02, float const m10, float const m11, float const m12, float const m20, float const m21, float const m22);
        void set(Mat3 const & rhs);
        void setSymmetric(float const a00, float const a01, float const a02, float const a11, float const a12, float const a22);
        void setSymmetric(SMat3 const & rhs);
    private:
        Mat3(Mat3 const & rhs);
        Mat3 & operator=(Mat3 const & rhs);
    };

    class Vec3
    {
    public:
        float x, y, z;
    public:
        Vec3();
        Vec3(float const x, float const y, float const z);
        void clear();
        void set(float const x, float const y, float const z);
        void set(Vec3 const & rhs);
    private:
        Vec3(Vec3 const & rhs);
        Vec3 & operator=(Vec3 const & rhs);
    };

    class MatrixUtils
    {
    public:
        static float fnorm(Mat3 const & a);
        static float fnorm(SMat3 const & a);
        static float off(Mat3 const & a);
        static float off(SMat3 const & a);

    public:
        static void mmul(Mat3 & out, Mat3 const & a, Mat3 const & b);
        static void mmul_ata(SMat3 & out, Mat3 const & a);
        static void transpose(Mat3 & out, Mat3 const & a);
        static void vmul(Vec3 & out, Mat3 const & a, Vec3 const & v);
        static void vmul_symmetric(Vec3 & out, SMat3 const & a, Vec3 const & v);
    };

    class VectorUtils
    {
    public:
        static void addScaled(Vec3 & v, float const s, Vec3 const & rhs);
        static float dot(Vec3 const & a, Vec3 const & b);
        static void normalize(Vec3 & v);
        static void scale(Vec3 & v, float const s);
        static void sub(Vec3 & c, Vec3 const & a, Vec3 const & b);
    };

    class Givens
    {
    public:
        static void rot01_post(Mat3 & m, float const c, float const s);
        static void rot02_post(Mat3 & m, float const c, float const s);
        static void rot12_post(Mat3 & m, float const c, float const s);
    };

    class Schur2
    {
    public:
        static void rot01(SMat3 & out, float & c, float & s);
        static void rot02(SMat3 & out, float & c, float & s);
        static void rot12(SMat3 & out, float & c, float & s);
    };

    class Svd
    {
    public:
        static void getSymmetricSvd(SMat3 const & a, SMat3 & vtav, Mat3 & v, float const tol, const int max_sweeps);
        static void pseudoinverse(Mat3 & out, SMat3 const & d, Mat3 const & v, float const tol);
        static float solveSymmetric(SMat3 const & A, Vec3 const & b, Vec3 & x, float const svd_tol, const int svd_sweeps, float const pinv_tol);
    };
    class LeastSquares
    {
    public:
        static float solveLeastSquares(Mat3 const & a, Vec3 const & b, Vec3 & x, float const svd_tol, const int svd_sweeps, float const pinv_tol);
    };
};

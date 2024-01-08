#include <stdexcept>

#include "qef.hpp"

namespace eng::svd
{
    static void normalize(float & nx, float & ny, float & nz)
    {
        Vec3 tmpv(nx, ny, nz);
        VectorUtils::normalize(tmpv);
        nx = tmpv.x;
        ny = tmpv.y;
        nz = tmpv.z;
    }

    QefData::QefData()
    {
        clear();
    }

    QefData::QefData(float const ata_00, float const ata_01, float const ata_02, float const ata_11, float const ata_12, float const ata_22,
                     float const atb_x, float const atb_y, float const atb_z,
                     float const btb,
                     float const massPoint_x, float const massPoint_y, float const massPoint_z, int const numPoints
    )
    {
        set(ata_00, ata_01, ata_02, ata_11, ata_12, ata_22, atb_x, atb_y, atb_z, btb, massPoint_x, massPoint_y, massPoint_z, numPoints);
    }

    QefData::QefData(QefData const & rhs)
    {
        set(rhs);
    }

    QefData & QefData::operator=(QefData const & rhs)
    {
        this->set(rhs);
        return *this;
    }

    void QefData::add(QefData const & rhs)
    {
		ata_.row[0] = _mm_add_ps(ata_.row[0], rhs.ata_.row[0]);
		ata_.row[1] = _mm_add_ps(ata_.row[1], rhs.ata_.row[1]);
		ata_.row[2] = _mm_add_ps(ata_.row[2], rhs.ata_.row[2]);

        atb_ = _mm_add_ps(atb_, rhs.atb_);
        btb_ += rhs.btb_;
        point_accum_ = _mm_add_ps(point_accum_, rhs.point_accum_);
    }

    void QefData::clear()
    {
        set(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }

    void QefData::set(float const ata_00, float const ata_01, float const ata_02, float const ata_11, float const ata_12, float const ata_22,
                      float const atb_x, float const atb_y, float const atb_z,
                      float const btb,
                      float const massPoint_x, float const massPoint_y, float const massPoint_z, int const numPoints
    )
    {
        ata_.row[0] = _mm_set_ps(ata_00, ata_01, ata_02, 0.0f);
        ata_.row[1] = _mm_set_ps(0.0f, ata_11, ata_12, 0.0f);
        ata_.row[2] = _mm_set_ps(0.0f, 0.0f, ata_22, 0.0f);
        atb_ = _mm_set_ps(atb_x, atb_y, atb_z, 0.0f);
        btb_ = btb;
        point_accum_ = _mm_set_ps(massPoint_x, massPoint_y, massPoint_z, numPoints);
    }

    void QefData::set(QefData const & rhs)
    {
        ata_.row[0] = rhs.ata_.row[0];
        ata_.row[1] = rhs.ata_.row[1];
        ata_.row[2] = rhs.ata_.row[2];
        atb_ = rhs.atb_;
        btb_ = rhs.btb_;
        point_accum_ = rhs.point_accum_;
    }

    QefSolver::QefSolver() : data(), ata(), atb(), massPoint(), x(), hasSolution(false)
    {
    }

    void QefSolver::add(float const px, float const py, float const pz, float nx, float ny, float nz)
    {
        hasSolution = false;
        normalize(nx, ny, nz);
        qef_simd_add(_mm_set_ps(px, py, pz, 0.0f), _mm_set_ps(nx, ny, nz, 0.0f), data.ata_, data.atb_, data.point_accum_);
    }

    void QefSolver::add(const Vec3 & p, const Vec3 & n)
    {
        add(p.x, p.y, p.z, n.x, n.y, n.z);
    }

    void QefSolver::add(QefData const & rhs)
    {
        hasSolution = false;
        data.add(rhs);
    }

    QefData QefSolver::getData()
    {
        return data;
    }

    float QefSolver::getError()
    {
        if (!this->hasSolution) throw std::runtime_error("illegal state");
        return getError(x);
    }

    float QefSolver::getError(const Vec3 & pos)
    {
        if (!hasSolution)
        {
            //setAta();
            //setAtb();
            return 0.1f;
        }
		return qef_simd_calc_error(data.ata_, _mm_set_ps(pos.x, pos.y, pos.z, 0.0f), data.atb_);
    }

    void QefSolver::reset()
    {
        hasSolution = false;
        data.clear();
    }

    void QefSolver::setAta()
    {
        //ata.setSymmetric(data.ata_00_, data.ata_01_, data.ata_02_, data.ata_11_, data.ata_12_, data.ata_22_);
    }

    void QefSolver::setAtb()
    {
        //atb.set(data.atb_x_, data.atb_y_, data.atb_z_);
    }

    float QefSolver::solve(Vec3 & outx, float const svd_tol, int const svd_sweeps, float const pinv_tol)
    {
        __m128 result = _mm_set_ps1(0.0f);
        float error = qef_simd_solve(data.ata_, data.atb_, data.point_accum_, result);
        outx.x = _mm_cvtss_f32(_mm_shuffle_ps(result, result, _MM_SHUFFLE(0, 0, 0, 0)));
        outx.x = _mm_cvtss_f32(_mm_shuffle_ps(result, result, _MM_SHUFFLE(0, 0, 0, 1)));
        outx.x = _mm_cvtss_f32(_mm_shuffle_ps(result, result, _MM_SHUFFLE(0, 0, 0, 2)));
        outx.x = _mm_cvtss_f32(_mm_shuffle_ps(result, result, _MM_SHUFFLE(0, 0, 0, 3)));
        return error;
        //massPoint.set(data.massPoint_x_, data.massPoint_y_, data.massPoint_z_);
        //VectorUtils::scale(massPoint, 1.0f / data.numPoints_);
        //setAta();
        //setAtb();
        //Vec3 tmpv;
        //MatrixUtils::vmul_symmetric(tmpv, ata, massPoint);
        //VectorUtils::sub(atb, atb, tmpv);
        //x.clear();
        //float const result = Svd::solveSymmetric(ata, atb, x, svd_tol, svd_sweeps, pinv_tol);
        //VectorUtils::addScaled(x, 1, massPoint);
        //setAtb();
        //outx.set(x);
        //hasSolution = true;
        //return result;
    }
}
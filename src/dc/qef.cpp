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
        ata_00_ += rhs.ata_00_;
        ata_01_ += rhs.ata_01_;
        ata_02_ += rhs.ata_02_;
        ata_11_ += rhs.ata_11_;
        ata_12_ += rhs.ata_12_;
        ata_22_ += rhs.ata_22_;
        atb_x_ += rhs.atb_x_;
        atb_y_ += rhs.atb_y_;
        atb_z_ += rhs.atb_z_;
        btb_ += rhs.btb_;
        massPoint_x_ += rhs.massPoint_x_;
        massPoint_y_ += rhs.massPoint_y_;
        massPoint_z_ += rhs.massPoint_z_;
        numPoints_ += rhs.numPoints_;
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
        ata_00_ = ata_00;
        ata_01_ = ata_01;
        ata_02_ = ata_02;
        ata_11_ = ata_11;
        ata_12_ = ata_12;
        ata_22_ = ata_22;
        atb_x_ = atb_x;
        atb_y_ = atb_y;
        atb_z_ = atb_z;
        btb_ = btb;
        massPoint_x_ = massPoint_x;
        massPoint_y_ = massPoint_y;
        massPoint_z_ = massPoint_z;
        numPoints_ = numPoints;
    }

    void QefData::set(QefData const & rhs)
    {
        set(rhs.ata_00_, rhs.ata_01_, rhs.ata_02_, rhs.ata_11_, rhs.ata_12_, rhs.ata_22_, rhs.atb_x_, rhs.atb_y_, rhs.atb_z_, rhs.btb_, rhs.massPoint_x_, rhs.massPoint_y_, rhs.massPoint_z_, rhs.numPoints_);
    }

    QefSolver::QefSolver() : data(), ata(), atb(), massPoint(), x(), hasSolution(false)
    {
    }

    void QefSolver::add(float const px, float const py, float const pz, float nx, float ny, float nz)
    {
        hasSolution = false;
        normalize(nx, ny, nz);
        data.ata_00_ += nx * nx;
        data.ata_01_ += nx * ny;
        data.ata_02_ += nx * nz;
        data.ata_11_ += ny * ny;
        data.ata_12_ += ny * nz;
        data.ata_22_ += nz * nz;
        float const dot = nx * px + ny * py + nz * pz;
        data.atb_x_ += dot * nx;
        data.atb_y_ += dot * ny;
        data.atb_z_ += dot * nz;
        data.btb_ += dot * dot;
        data.massPoint_x_ += px;
        data.massPoint_y_ += py;
        data.massPoint_z_ += pz;
        ++data.numPoints_;
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
            setAta();
            setAtb();
        }
        Vec3 atax;
        MatrixUtils::vmul_symmetric(atax, ata, pos);
        return VectorUtils::dot(pos, atax) - 2 * VectorUtils::dot(pos, atb) + data.btb_;
    }

    void QefSolver::reset()
    {
        hasSolution = false;
        data.clear();
    }

    void QefSolver::setAta()
    {
        ata.setSymmetric(data.ata_00_, data.ata_01_, data.ata_02_, data.ata_11_, data.ata_12_, data.ata_22_);
    }

    void QefSolver::setAtb()
    {
        atb.set(data.atb_x_, data.atb_y_, data.atb_z_);
    }

    float QefSolver::solve(Vec3 & outx, float const svd_tol, int const svd_sweeps, float const pinv_tol)
    {
        if (data.numPoints_ == 0)
        {
            throw std::invalid_argument("...");
        }
        massPoint.set(data.massPoint_x_, data.massPoint_y_, data.massPoint_z_);
        VectorUtils::scale(massPoint, 1.0f / data.numPoints_);
        setAta();
        setAtb();
        Vec3 tmpv;
        MatrixUtils::vmul_symmetric(tmpv, ata, massPoint);
        VectorUtils::sub(atb, atb, tmpv);
        x.clear();
        float const result = Svd::solveSymmetric(ata, atb, x, svd_tol, svd_sweeps, pinv_tol);
        VectorUtils::addScaled(x, 1, massPoint);
        setAtb();
        outx.set(x);
        hasSolution = true;
        return result;
    }
}
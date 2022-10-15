#pragma once

#include "svd.hpp"

namespace eng::svd
{
    class QefData
    {
    public:
        float ata_00_, ata_01_, ata_02_, ata_11_, ata_12_, ata_22_;
        float atb_x_, atb_y_, atb_z_;
        float btb_;
        float massPoint_x_, massPoint_y_, massPoint_z_;
        int numPoints_;

        QefData();
        QefData(float const ata_00, float const ata_01, float const ata_02, float const ata_11, float const ata_12, float const ata_22,
                float const atb_x, float const atb_y, float const atb_z,
                float const btb,
                float const massPoint_x, float const massPoint_y, float const massPoint_z, int const numPoints
        );
        QefData(QefData const & rhs);
        QefData & operator=(QefData const & rhs);

        void add(QefData const & rhs);
        void clear();
        void set(float const ata_00, float const ata_01, float const ata_02, float const ata_11, float const ata_12, float const ata_22,
                 float const atb_x, float const atb_y, float const atb_z,
                 float const btb,
                 float const massPoint_x, float const massPoint_y, float const massPoint_z, int const numPoints
        );
        void set(QefData const & rhs);
    };

    class QefSolver
    {
    private:
        QefData data;
        SMat3 ata;
        Vec3 atb, massPoint, x;
        bool hasSolution;

    public:
        QefSolver();

        Vec3 const & getMassPoint() const { return massPoint; }

        void add(float const px, float const py, float const pz, float nx, float ny, float nz);
        void add(Vec3 const & p, Vec3 const & n);
        void add(QefData const & rhs);
        void reset();

        QefData getData();
        float getError();
        float getError(Vec3 const & pos);
        float solve(Vec3 & outx, float const svd_tol, int const svd_sweeps, float const pinv_tol);

    private:
        QefSolver(QefSolver const & rhs);
        void setAta();
        void setAtb();
    };
};
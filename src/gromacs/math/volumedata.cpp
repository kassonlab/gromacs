/*
 * This file is part of the GROMACS molecular simulation package.
 *
 * Copyright (c) 2016, by the GROMACS development team, led by
 * Mark Abraham, David van der Spoel, Berk Hess, and Erik Lindahl,
 * and including many others, as listed in the AUTHORS file in the
 * top-level source directory and at http://www.gromacs.org.
 *
 * GROMACS is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * GROMACS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GROMACS; if not, see
 * http://www.gnu.org/licenses, or write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
 *
 * If you want to redistribute modifications to GROMACS, please
 * consider that scientific software is very special. Version
 * control is crucial - bugs must be traceable. We will be happy to
 * consider code for inclusion in the official distribution, but
 * derived work must not be called official GROMACS. Details are found
 * in the README & COPYING files - if they are missing, get the
 * official version at http://www.gromacs.org.
 *
 * To help us fund GROMACS development, we humbly ask that you cite
 * the research papers on the package. Check out http://www.gromacs.org.
 */
/*! \internal \file
 * \brief
 * Implements methods from volumedata.h
 *
 * \author Christian Blau <cblau@gwdg.de>
 */
#include "gmxpre.h"

#include "volumedata.h"
#include "invertmatrix.h"

#include <algorithm>
#include <numeric>
#include <vector>

#include "gromacs/math/utilities.h"
#include "gromacs/math/vec.h"
#include "gromacs/math/vectypes.h"

namespace gmx
{

namespace volumedata
{

/********************************************************************
 * CrystalSymmetry::Impl
 */

/*! \internal \brief
 * Private implementation class for CrystalSymmetry.
 *
 */
class CrystalSymmetry::Impl
{
    public:

        Impl();
        ~Impl();

        int         space_group_; //!< space group as defined by IUCr conventions (Table 12.3.4.1 Standard space-group symbols”, pages 824-831, International Tables for Crystallography, Volume A, fifth edition)

};

CrystalSymmetry::Impl::Impl() : space_group_(1)
{
}

CrystalSymmetry::Impl::~Impl()
{
}

/********************************************************************
 * CrystalSymmetry
 */

void CrystalSymmetry::set_space_group(int space_group)
{
    impl_->space_group_ = space_group;
}
int CrystalSymmetry::space_group()
{
    return impl_->space_group_;
}

CrystalSymmetry::CrystalSymmetry() : impl_(new CrystalSymmetry::Impl())
{

};

std::string
CrystalSymmetry::print()
{
    return "---crystal symmetry---\nspace group : "+std::to_string(space_group()) + "---\n";
};

CrystalSymmetry::~CrystalSymmetry()
{

};

/********************************************************************
 * FiniteGrid::Impl
 */

/*! \internal \brief
 * Private implementation class for FiniteGrid.
 *
 */

class FiniteGrid::Impl
{
    public:
        Impl();
        ~Impl();

        IVec             extend_;        //!< The grid-size.
        matrix           cell_;          //!< r' = cell_ . (r-translate_)
        matrix           unit_cell_;     //!< cell_/extend_
        matrix           unit_cell_inv_; //!< cell_/extend_
        RVec             translate_;     //!< r' = cell_ . (r-translate_)


        /*! \brief
         * Angle between two vectors in degree.
         */
        real deg_angle(rvec a, rvec b);

        /*! \brief
         * Gram-Schmidt orthonormalisation.
         *
         * The numerical shortcommings of Gram-Schmidt
         * should not matter for three dimensions and cell matrices.
         */
        void QRDecomposition(const matrix A, matrix Q, matrix R);

        /*! \brief
         * Projects vector a onto vector b, stores this projection in c. Conveninence function for Gram-Schmidt method.
         */
        void project(const rvec a, const rvec b, rvec c);

        /*! \brief
         * set unit cell; divide cell matrix by extend in respective direction
         */
        void set_unit_cell();
};

real FiniteGrid::Impl::deg_angle(rvec a, rvec b)
{
    return gmx_angle(a, b)*180.0/M_PI;
}

void FiniteGrid::Impl::project(const rvec a, const rvec b, rvec c)
{
    svmul(iprod(a, b)/iprod(a, a), a, c);
}

void FiniteGrid::Impl::QRDecomposition(const matrix A, matrix Q, matrix R)
{
    rvec u;
    copy_rvec(A[XX], Q[XX]);

    copy_rvec(A[YY], Q[YY]);
    project(Q[YY], Q[XX], u);
    rvec_dec(Q[YY], u);

    copy_rvec(A[ZZ], Q[ZZ]);
    project(Q[ZZ], Q[XX], u);
    rvec_dec(Q[ZZ], u);
    project(Q[ZZ], Q[YY], u);
    rvec_dec(Q[ZZ], u);

    unitv(Q[XX], Q[XX]);
    unitv(Q[YY], Q[YY]);
    unitv(Q[ZZ], Q[ZZ]);

    tmmul(A, Q, R);

}

void FiniteGrid::Impl::set_unit_cell()
{
    svmul(1./extend_[XX], cell_[XX], unit_cell_[XX]);
    svmul(1./extend_[YY], cell_[YY], unit_cell_[YY]);
    svmul(1./extend_[ZZ], cell_[ZZ], unit_cell_[ZZ]);
    invertMatrix(unit_cell_, unit_cell_inv_);
}

FiniteGrid::Impl::Impl()
{
    // to avoid MSBuild Error C2536, use this intialization instead of braced list
    for (size_t i = XX; i <= ZZ; i++)
    {
        for (size_t j = XX; j <= ZZ; j++)
        {
            cell_[i][j] = 0;
        }
    }
    cell_[XX][XX] = 1;
    cell_[YY][YY] = 1;
    cell_[ZZ][ZZ] = 1;
};

FiniteGrid::Impl::~Impl()
{
};

/********************************************************************
 * FiniteGrid
 */
FiniteGrid::FiniteGrid() : impl_(new FiniteGrid::Impl())
{
};


FiniteGrid::~FiniteGrid()
{
};

void FiniteGrid::set_extend(IVec extend)
{
    impl_->extend_ = extend;
}
IVec FiniteGrid::extend()
{
    return impl_->extend_;
}

real FiniteGrid::grid_cell_volume()
{
    return det(impl_->cell_)/(real) num_gridpoints();
}

int FiniteGrid::ndx3d_to_ndx1d(IVec i_xyz)
{
    return i_xyz[XX] + impl_->extend_[XX] * i_xyz[YY]  + impl_->extend_[XX] * impl_->extend_[YY] * i_xyz[ZZ];
}

IVec FiniteGrid::ndx1d_to_ndx3d(int i)
{
    IVec result;
    result[XX] = (i % impl_->extend_[XX]) % impl_->extend_[YY];
    result[YY] = (i / impl_->extend_[XX]) % impl_->extend_[YY];
    result[ZZ] = (i / impl_->extend_[XX]) / impl_->extend_[YY];
    return result;
}

void FiniteGrid::set_translation(RVec translate)
{
    impl_->translate_ = translate;
}

RVec FiniteGrid::translation()
{
    return impl_->translate_;
}

size_t FiniteGrid::num_gridpoints()
{
    return impl_->extend_[XX]*impl_->extend_[YY]*impl_->extend_[ZZ];
}

RVec FiniteGrid::cell_lengths()
{
    return {
               norm(impl_->cell_[XX]), norm(impl_->cell_[YY]), norm(impl_->cell_[ZZ])
    };
}

RVec FiniteGrid::cell_angles()
{
    return {
               impl_->deg_angle(impl_->cell_[XX], impl_->cell_[ZZ]),
               impl_->deg_angle(impl_->cell_[XX], impl_->cell_[YY]),
               impl_->deg_angle(impl_->cell_[YY], impl_->cell_[ZZ])
    };
}


void FiniteGrid::set_cell(RVec length, RVec angle)
{


    real cos_beta  = cos(M_PI*angle[YY]/180.);
    real cos_gamma = cos(M_PI*angle[ZZ]/180.);
    real sin_gamma = sin(M_PI*angle[ZZ]/180.);

    impl_->cell_[XX][XX] = length[XX];
    impl_->cell_[XX][YY] = 0;
    impl_->cell_[XX][ZZ] = 0;

    impl_->cell_[YY][XX] = length[YY] * cos_gamma;
    impl_->cell_[YY][YY] = length[YY] * sin_gamma;
    impl_->cell_[YY][ZZ] = 0;

    impl_->cell_[ZZ][XX] = cos_beta;
    impl_->cell_[ZZ][YY] = ( cos(M_PI*angle[XX]/180.) - cos_beta * cos_gamma ) / sin_gamma;
    impl_->cell_[ZZ][ZZ] = sqrt(1-impl_->cell_[ZZ][XX]*impl_->cell_[ZZ][XX]-impl_->cell_[ZZ][YY]*impl_->cell_[ZZ][YY]);

    impl_->cell_[ZZ][XX] *= length[ZZ];
    impl_->cell_[ZZ][YY] *= length[ZZ];
    impl_->cell_[ZZ][ZZ] *= length[ZZ];

    if ((impl_->extend_[XX] > 0) && (impl_->extend_[YY] > 0) && (impl_->extend_[ZZ] > 0) )
    {
        impl_->set_unit_cell();
    }

};

bool FiniteGrid::rectangular()
{
    RVec angles = cell_angles();
    for (int i = XX; i <= ZZ; ++i)
    {
        if ((angles[i] < 89.999) || (angles[i] > 90.001))
        {
            return false;
        }
    }
    return true;
};

RVec FiniteGrid::unit_cell_XX() const
{
    return impl_->unit_cell_[XX];
}

RVec FiniteGrid::unit_cell_YY() const
{
    return impl_->unit_cell_[YY];
}

RVec FiniteGrid::unit_cell_ZZ() const
{
    return impl_->unit_cell_[ZZ];
}

bool FiniteGrid::spacing_is_same_xyz()
{
    return (abs(norm2(impl_->unit_cell_[XX]) - norm2(impl_->unit_cell_[YY])) < 1e-5) && (abs(norm2(impl_->unit_cell_[XX])-norm2(impl_->unit_cell_[ZZ])) < 1e-5);
};

IVec FiniteGrid::coordinate_to_gridindex_ceil_ivec(const rvec x)
{
    RVec result;
    rvec x_shifted;
    rvec_sub(x, impl_->translate_, x_shifted);
    mvmul(impl_->unit_cell_inv_, x_shifted, result);
    return {
               (int)ceil(result[XX]), (int)ceil(result[YY]), (int)ceil(result[ZZ])
    };
}

IVec FiniteGrid::coordinate_to_gridindex_floor_ivec(const rvec x)
{
    RVec result;
    rvec x_shifted;
    rvec_sub(x, impl_->translate_, x_shifted);
    mvmul(impl_->unit_cell_inv_, x_shifted, result);
    return {
               (int)floor(result[XX]), (int)floor(result[YY]), (int)floor(result[ZZ])
    };
}


real
FiniteGrid::avg_spacing()
{
    return (impl_->unit_cell_[XX][XX] + impl_->unit_cell_[YY][YY]+impl_->unit_cell_[ZZ][ZZ])/3;
}

RVec FiniteGrid::gridpoint_coordinate(IVec i) const
{
    RVec result;
    mvmul(impl_->unit_cell_, RVec(i[XX], i[YY], i[ZZ]), result);
    rvec_inc(result, impl_->translate_);
    return result;
};

RVec FiniteGrid::gridpoint_coordinate(int i)
{
    return gridpoint_coordinate(ndx1d_to_ndx3d(i));
}

void FiniteGrid::rotation(matrix Q)
{
    matrix R;
    impl_->QRDecomposition(impl_->cell_, Q, R);
}

void FiniteGrid::copy_grid(FiniteGrid &grid)
{
    copy_mat(grid.impl_->cell_, this->impl_->cell_);
    copy_ivec(grid.impl_->extend_, this->impl_->extend_);
    copy_rvec(grid.impl_->translate_, this->impl_->translate_);
    this->impl_->set_unit_cell();
}
std::string
FiniteGrid::print()
{
    std::string result("\n  ------- finite grid -------\n");
    result += "    extend       : " + std::to_string(extend()[0]) + " " + std::to_string(extend()[1]) + " " + std::to_string(extend()[2]) + "\n";
    result += "    ngridpoints  : " + std::to_string(num_gridpoints()) + "\n";
    result += "    translation  : " + std::to_string(translation()[0]) + " " + std::to_string(translation()[1]) + " " + std::to_string(translation()[2]) + "\n";
    result += "    cell_lengths : " + std::to_string(cell_lengths()[0]) + " " + std::to_string(cell_lengths()[1]) + " " + std::to_string(cell_lengths()[2]) + "\n";
    result += "    cell_angles  : " + std::to_string(cell_angles()[0]) + " " + std::to_string(cell_angles()[1]) + " " + std::to_string(cell_angles()[2]) + "\n";
    result += "    V_cell       : " + std::to_string(grid_cell_volume()) + "\n";
    return result+"  ----- end finite grid -----\n\n";
}


/********************************************************************
 * GridReal::Impl
 */

/*! \internal \brief
 * Private implementation class for GridReal.
 *
 */
class GridReal::Impl
{
    public:
        Impl();
        ~Impl();

        std::vector<real> data_; //!< The data on the grid, represented as one-dimensional vector
};

GridReal::Impl::Impl()
{

};

GridReal::Impl::~Impl()
{

};


/********************************************************************
 * GridReal
 */
real GridReal::min()
{
    return *std::min_element(std::begin(impl_->data_), std::end(impl_->data_));
};

real GridReal::max()
{
    return *std::max_element(std::begin(impl_->data_), std::end(impl_->data_));
};

real GridReal::mean()
{
    return sum()/ static_cast<float>(impl_->data_.size());
};

real GridReal::var()
{
    real data_mean        = mean();
    real square_deviation = std::accumulate(std::begin(impl_->data_), std::end(impl_->data_), 0.,
                                            [ = ](const real &a, real b){return a + (b - data_mean)*(b - data_mean); } );
    return square_deviation / static_cast<float>(impl_->data_.size());
}

real GridReal::rms()
{
    return sqrt(var());
};

size_t GridReal::data_size()
{
    return impl_->data_.size();
};

std::pair<std::vector<real>::iterator, std::vector<real>::iterator> GridReal::z_section(int z)
{
    std::vector<real>::iterator z_begin = impl_->data_.begin()+extend()[XX]*extend()[YY]*z;
    std::vector<real>::iterator z_end   = impl_->data_.begin()+extend()[XX]*extend()[YY]*(z+1);
    return std::pair<std::vector<real>::iterator, std::vector<real>::iterator> (z_begin, z_end);
}
std::pair<std::vector<real>::iterator, std::vector<real>::iterator> GridReal::zy_column(int z, int y)
{
    std::vector<real>::iterator column_begin = impl_->data_.begin()+extend()[XX]*extend()[YY]*z + extend()[XX]*y;
    std::vector<real>::iterator column_end   = impl_->data_.begin()+extend()[XX]*extend()[YY]*z + extend()[XX]*(y+1);
    return std::pair<std::vector<real>::iterator, std::vector<real>::iterator> (column_begin, column_end);
};

std::vector<real>::iterator GridReal::zy_column_begin(int z, int y) const
{
    return impl_->data_.begin()+gmx::volumedata::FiniteGrid::impl_->extend_[XX]*gmx::volumedata::FiniteGrid::impl_->extend_[YY]*z + gmx::volumedata::FiniteGrid::impl_->extend_[XX]*y;
}

void GridReal::resize()
{
    impl_->data_.resize(num_gridpoints());
}
real GridReal::normalize()
{
    real scale =  this->grid_cell_volume() / std::accumulate(impl_->data_.begin(), impl_->data_.end(), 0.);
    std::for_each(impl_->data_.begin(), impl_->data_.end(), [ = ] (real &v) {v *= scale; });
    return 1./scale;
}

real &GridReal::at(IVec index)
{
    return impl_->data_.at(ndx3d_to_ndx1d(index));
};

std::vector<real> &GridReal::data()
{
    return impl_->data_;
}

void GridReal::add_offset(real value)
{
    std::for_each(impl_->data_.begin(), impl_->data_.end(), [ = ](real &datum){ datum += value; });
}

GridReal::GridReal() : impl_(new GridReal::Impl())
{
}

GridReal::~GridReal()
{
}

real
GridReal::sum()
{
    return std::accumulate(std::begin(impl_->data_), std::end(impl_->data_), 0.);
}

RVec
GridReal::center_of_mass()
{
    rvec weighted_grid_coordinate;
    RVec com = {0, 0, 0};
    for (size_t i = 0; i < num_gridpoints(); i++)
    {
        svmul(impl_->data_[i], gridpoint_coordinate(i), weighted_grid_coordinate);
        rvec_inc(com, weighted_grid_coordinate);
    }
    svmul(1./(sum()), com, com);
    return com;
}

std::string
GridReal::print()
{
    std::string result;
    result += "------- real number grid -------\n";
    result += FiniteGrid::print();
    result += "  min  :" + std::to_string(min()) + "\n";
    result += "  max  :" + std::to_string(max()) + "\n";
    result += "  mean :" + std::to_string(mean()) + "\n";
    result += "  var  :" + std::to_string(var()) + "\n";
    result += "  rms  :" + std::to_string(rms()) + "\n";
    result += "\n----- end real number grid -----\n\n";
    return result;
}

void GridReal::copy_grid(FiniteGrid &grid)
{
    FiniteGrid::copy_grid(grid);
    resize();
}

void GridReal::zero()
{
    resize();
    std::fill(impl_->data_.begin(), impl_->data_.end(), 0);
};

} //namespace grid_data

} //namespace gmx

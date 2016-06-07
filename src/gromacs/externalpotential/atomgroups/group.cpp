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

#include "group.h"

#include <algorithm>
#include <memory>

#include "gromacs/domdec/ga2la.h"
#include "gromacs/externalpotential/forceplotter.h"
#include "gromacs/math/vec.h"
#include "gromacs/mdlib/groupcoord.h"
#include "gromacs/mdlib/sim_util.h"
#include "gromacs/mdtypes/commrec.h"
#include "gromacs/topology/topology.h"
#include "gromacs/utility/smalloc.h"
#include "gromacs/mdlib/gmx_omp_nthreads.h"
#include "gromacs/utility/gmxomp.h"


namespace gmx
{
/*******************************************************************************
 * Group::Iterator
 */
Group::GroupIterator::GroupIterator(Group * group, int i)
    : group_(group), i_(i) {}

Group::GroupIterator::GroupIterator(Group * group)
    : group_(group), i_(0) {}

Group::GroupIterator::GroupIterator(const GroupIterator * iterator)
    : group_(iterator->group_), i_(iterator->i_) {}

Group::GroupIterator &Group::GroupIterator::operator++()
{
    ++i_;
    return *this;
}

Group::GroupIterator Group::GroupIterator::operator++(int)
{
    GroupIterator tmp(this);
    operator++();
    return tmp;
}

bool Group::GroupIterator::operator==(const GroupIterator &rhs)
{
    return (i_ == rhs.i_) && (group_ == rhs.group_);
}

bool Group::GroupIterator::operator!=(const GroupIterator &rhs)
{
    return (i_ != rhs.i_) || (group_ != rhs.group_);
}

GroupAtom &Group::GroupIterator::operator*()
{
    int i_local = group_->ind_loc_[i_];
    group_->atom_.x          = group_->x_[i_local];
    group_->atom_.force      = as_vec_array(group_->f_loc_.data())[i_];
    group_->atom_.properties = group_->properties_[i_];
    group_->atom_.i_local    = i_local;
    group_->atom_.i_global   = group_->coll_ind_[i_];
    return group_->atom_;
}

/*******************************************************************************
 * Group
 */

Group::GroupIterator Group::begin()
{
    return GroupIterator(this);
};

Group::GroupIterator Group::end()
{
    return GroupIterator(this, num_atoms_loc_);
};

GroupAtom &Group::operator[](int i)
{
    GroupIterator result(this, i);
    return *result;
};

Group::Group(const Group &group)
{
    this->ind_           = group.ind_;
    this->ind_loc_       = group.ind_loc_;
    this->coll_ind_      = group.coll_ind_;
    this->properties_    = group.properties_;
    this->atom_          = group.atom_;
    this->bParallel_     = group.bParallel_;
    this->f_loc_         = group.f_loc_;
    this->num_atoms_     = group.num_atoms_;
    this->num_atoms_loc_ = group.num_atoms_loc_;
    this->x_             = group.x_;
}

Group::Group( int nat, int *ind, bool bParallel) :
    num_atoms_(nat), ind_(ind), bParallel_(bParallel)
{
    if (!bParallel_)
    {
        num_atoms_loc_ = num_atoms_;
        ind_loc_.assign(ind, ind+num_atoms_);
        coll_ind_.resize(num_atoms_loc_);
        std::iota(coll_ind_.begin(), coll_ind_.end(), 0);
        f_loc_.resize(num_atoms_, {0, 0, 0});
        properties_.resize(num_atoms_, nullptr);
    }
};

Group::~Group()
{
}


void Group::set_indices(gmx_ga2la_t *ga2la)
{
    /* Loop over all the atom indices of the group to check
     * which ones are on the local node */
    int  i_local;
    int  nalloc_loc = 0;
    num_atoms_loc_ = 0;
    ind_loc_.clear();
    coll_ind_.clear();

    for (int i_collective = 0; i_collective < num_atoms_; i_collective++)
    {
        if (ga2la_get_home(ga2la, ind_[i_collective], &i_local))
        {
            /* The atom with this index is a home atom ? */
            if (num_atoms_loc_ >= nalloc_loc)  /* Check whether memory suffices */
            {
                nalloc_loc = over_alloc_dd(num_atoms_loc_+1);
                /* We never need more memory than the number of atoms in the group */
                nalloc_loc = std::min(nalloc_loc, num_atoms_);

                ind_loc_.reserve(nalloc_loc);
                coll_ind_.reserve(nalloc_loc);
            }
            /* Save the atoms index in the local atom numbers array */
            ind_loc_.push_back(i_local);

            /* Keep track of where this local atom belongs in the collective index array.
             * This is needed when reducing the local arrays to a collective/global array
             * in communicate_group_positions */
            coll_ind_.push_back(ind_[i_collective]);

            /* add one to the local atom count */
            num_atoms_loc_++;
        }
    }

    f_loc_.resize(num_atoms_loc_, {0, 0, 0});
    properties_.resize(num_atoms_loc_, nullptr);

};


void Group::add_forces(rvec f[], real w)
{

    for (int l = 0; l < num_atoms_loc_; l++)
    {
        for (int i = XX; i <= ZZ; i++)
        {
            f_loc_[l][i] *= w;
        }
        /* Get the right index of the local force, since typically not all local
         * atoms are subject to density fitting forces and add to local force */
        rvec_inc(f[ind_loc_[l]], f_loc_[l]);
    }
};

const std::vector<int> &Group::collective_index()
{
    return coll_ind_;
}

void Group::set_x(const rvec x[])
{
    /*
     * For more general cases , x_ is not const but a modified version of x.
     * However, in the plane case not copying atom coordinates saves plenty of time.
     */
    x_ = const_cast<rvec *>(x);
};

int Group::num_atoms_global() const
{
    return num_atoms_;
}

int Group::num_atoms_loc()
{
    return num_atoms_loc_;
};

void Group::parallel_loop(std::function<void(GroupAtom &, const int &)> loop_kernel_function)
{
#pragma omp parallel for num_threads(std::max(1, gmx_omp_nthreads_get(emntDefault))) schedule(static)
    for (int i = 0; i < num_atoms_loc_; i++)
    {
        loop_kernel_function((*this)[i], std::max(0, gmx_omp_get_thread_num()));
    }
}

RVec
Group::local_coordinate_sum()
{
    return std::accumulate(
            this->begin(), this->end(), RVec {0, 0, 0},
            [] (RVec &coordinate_sum, GroupAtom local_atom) {
                rvec_inc(coordinate_sum, local_atom.x);
                return coordinate_sum;
            });
}

} // namespace gmx
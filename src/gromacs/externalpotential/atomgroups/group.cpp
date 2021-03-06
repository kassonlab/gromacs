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
GroupIterator::GroupIterator(Group &group, int i)
{
    atom_.i_local    = group.ind_loc_.begin() + i;
    atom_.force      = group.f_loc_.begin() + i;
    atom_.properties = group.properties_.begin() + i;
    // if we never dereference group->end() or above , we are fines
    atom_.i_global   = group.ind_+i;
    x_               = group.x_;
}

GroupIterator &GroupIterator::operator++()
{
    ++atom_.i_local;
    ++atom_.force;
    ++atom_.properties;
    ++atom_.i_global;
    return *this;
}

bool GroupIterator::operator==(const GroupIterator &rhs)
{
    return (atom_.i_local == rhs.atom_.i_local);
}

bool GroupIterator::operator!=(const GroupIterator &rhs)
{
    return (atom_.i_local != rhs.atom_.i_local);
}

GroupAtom &GroupIterator::operator*()
{
    atom_.x  = x_[*atom_.i_local];
    return atom_;
}

/*******************************************************************************
 * Group
 */

GroupIterator Group::begin(int thread_index, int num_threads)
{
    return GroupIterator(*this, thread_index * (num_atoms_loc_ / num_threads ));
};

GroupIterator Group::end(int thread_index, int num_threads)
{
    if (thread_index == num_threads-1)
    {
        return GroupIterator(*this, num_atoms_loc_);
    }
    else
    {
        return GroupIterator(*this, (thread_index+1) * (num_atoms_loc_ / num_threads));
    }
};


GroupIterator Group::begin()
{
    return GroupIterator(*this);
};

GroupIterator Group::end()
{
    return GroupIterator(*this, num_atoms_loc_);
};

GroupAtom &Group::operator[](int i)
{
    GroupIterator result(*this, i);
    return *result;
};

std::shared_ptr<GroupAtom> Group::atom(int i)
{
    auto result = std::make_shared<GroupAtom>();
    result->i_local      = ind_loc_.begin()+i;
    result->i_collective = coll_ind_.begin()+i;
    result->i_global     = ind_+i;
    result->x            = x_[ind_loc_[i]];
    result->force        = f_loc_.begin()+i;
    result->properties   = properties_.begin()+i;
    return result;
}

Group::Group(const Group &group)
{
    this->ind_           = group.ind_;
    this->ind_loc_       = group.ind_loc_;
    this->coll_ind_      = group.coll_ind_;
    this->properties_    = group.properties_;
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

real Group::max_element(rvec arr[])
{
    real max_norm = 0;
    for (int l = 0; l < num_atoms_loc_; l++)
    {
        if (norm2(arr[ind_loc_[l]]) > max_norm)
        {
            max_norm = norm2(arr[ind_loc_[l]]);
        }
        ;
    }
    return sqrt(max_norm);
}

real Group::max_set_f()
{
    RVec f_max = *std::max_element(f_loc_.begin(), f_loc_.end(),
                                   [](const RVec &a, const RVec &b){
                                       return norm2(a) < norm2(b);
                                   }
                                   );
    return norm(f_max);
}


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
    const int number_of_threads = std::max(1, gmx_omp_nthreads_get(emntDefault));
    #pragma omp parallel num_threads(number_of_threads)
    {
        int           this_thread_index     = gmx_omp_get_thread_num();
        GroupIterator first_atom_for_thread = begin(this_thread_index, number_of_threads);
        GroupIterator end_of_thread_atoms   = end(this_thread_index, number_of_threads);
        for (auto atom = first_atom_for_thread; atom != end_of_thread_atoms; ++atom)
        {
            loop_kernel_function(*atom, this_thread_index);
        }
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

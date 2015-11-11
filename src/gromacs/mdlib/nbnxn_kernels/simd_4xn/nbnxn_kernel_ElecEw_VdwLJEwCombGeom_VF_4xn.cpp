/*
 * This file is part of the GROMACS molecular simulation package.
 *
 * Copyright (c) 2012,2013,2014,2015, by the GROMACS development team, led by
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
/*
 * Note: this file was generated by the Verlet kernel generator for
 * kernel type 4xn.
 */

/* Some target architectures compile kernels for only some NBNxN
 * kernel flavours, but the code is generated before the target
 * architecture is known. So compilation is conditional upon
 * GMX_NBNXN_SIMD_4XN, so that this file reduces to a stub
 * function definition when the kernel will never be called.
 */
#include "gmxpre.h"

#include "gromacs/legacyheaders/types/interaction_const.h"
#include "gromacs/mdlib/nbnxn_simd.h"

#define GMX_SIMD_J_UNROLL_SIZE 1
#include "gromacs/mdlib/nbnxn_kernels/simd_4xn/nbnxn_kernel_simd_4xn.h"

#define CALC_COUL_EWALD
#define LJ_CUT
#define LJ_EWALD_GEOM
/* Use full LJ combination matrix + geometric rule for the grid correction */
#define CALC_ENERGIES

#ifdef GMX_NBNXN_SIMD_4XN
#include "gromacs/mdlib/nbnxn_kernels/simd_4xn/nbnxn_kernel_simd_4xn_common.h"
#endif /* GMX_NBNXN_SIMD_4XN */

#ifdef CALC_ENERGIES
void
nbnxn_kernel_ElecEw_VdwLJEwCombGeom_VF_4xn(const nbnxn_pairlist_t    gmx_unused *nbl,
                                           const nbnxn_atomdata_t    gmx_unused *nbat,
                                           const interaction_const_t gmx_unused *ic,
                                           rvec                      gmx_unused *shift_vec,
                                           real                      gmx_unused *f,
                                           real                      gmx_unused *fshift,
                                           real                      gmx_unused *Vvdw,
                                           real                      gmx_unused *Vc)
#else /* CALC_ENERGIES */
void
nbnxn_kernel_ElecEw_VdwLJEwCombGeom_VF_4xn(const nbnxn_pairlist_t    gmx_unused *nbl,
                                           const nbnxn_atomdata_t    gmx_unused *nbat,
                                           const interaction_const_t gmx_unused *ic,
                                           rvec                      gmx_unused *shift_vec,
                                           real                      gmx_unused *f,
                                           real                      gmx_unused *fshift)
#endif /* CALC_ENERGIES */
#ifdef GMX_NBNXN_SIMD_4XN
#include "gromacs/mdlib/nbnxn_kernels/simd_4xn/nbnxn_kernel_simd_4xn_outer.h"
#else /* GMX_NBNXN_SIMD_4XN */
{
/* No need to call gmx_incons() here, because the only function
 * that calls this one is also compiled conditionally. When
 * GMX_NBNXN_SIMD_4XN is not defined, it will call no kernel functions and
 * instead call gmx_incons().
 */
}
#endif /* GMX_NBNXN_SIMD_4XN */

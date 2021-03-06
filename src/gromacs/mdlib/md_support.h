/*
 * This file is part of the GROMACS molecular simulation package.
 *
 * Copyright (c) 1991-2000, University of Groningen, The Netherlands.
 * Copyright (c) 2001-2004, The GROMACS development team.
 * Copyright (c) 2013,2014,2015,2016, by the GROMACS development team, led by
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
#ifndef GMX_MDLIB_MD_SUPPORT_H
#define GMX_MDLIB_MD_SUPPORT_H

#include "gromacs/mdlib/vcm.h"
#include "gromacs/timing/wallcycle.h"

struct gmx_constr;
struct gmx_ekindata_t;
struct gmx_enerdata_t;
struct gmx_global_stat;
struct gmx_multisim_t;
struct gmx_signalling_t;
struct t_extmass;
struct t_forcerec;
struct t_grpopts;
struct t_lambda;
struct t_nrnb;
struct t_state;
struct t_trxframe;

namespace gmx
{
class MDLogger;
}

/* Define a number of flags to better control the information
 * passed to compute_globals in md.c and global_stat.
 */

/* we are computing the kinetic energy from average velocities */
#define CGLO_EKINAVEVEL     (1<<2)
/* we are removing the center of mass momenta */
#define CGLO_STOPCM         (1<<3)
/* bGStat is defined in do_md */
#define CGLO_GSTAT          (1<<4)
/* Sum the energy terms in global computation */
#define CGLO_ENERGY         (1<<6)
/* Sum the kinetic energy terms in global computation */
#define CGLO_TEMPERATURE    (1<<7)
/* Sum the kinetic energy terms in global computation */
#define CGLO_PRESSURE       (1<<8)
/* Sum the constraint term in global computation */
#define CGLO_CONSTRAINT     (1<<9)
/* Reading ekin from the trajectory */
#define CGLO_READEKIN       (1<<10)
/* we need to reset the ekin rescaling factor here */
#define CGLO_SCALEEKIN      (1<<11)
/* After a new DD partitioning, we need to set a flag to schedule
 * global reduction of the total number of bonded interactions that
 * will be computed, to check none are missing. */
#define CGLO_CHECK_NUMBER_OF_BONDED_INTERACTIONS (1<<12)


/* return the number of steps between global communcations */
int check_nstglobalcomm(const gmx::MDLogger &mdlog, int nstglobalcomm, t_inputrec *ir);

/* check which of the multisim simulations has the shortest number of
   steps and return that number of nsteps */
gmx_int64_t get_multisim_nsteps(const t_commrec *cr,
                                gmx_int64_t      nsteps);

void rerun_parallel_comm(t_commrec *cr, t_trxframe *fr,
                         gmx_bool *bNotLastFrame);

/* get the conserved energy associated with the ensemble type*/
real compute_conserved_from_auxiliary(t_inputrec *ir, t_state *state,
                                      t_extmass *MassQ);

/* set the lambda values at each step of mdrun when they change */
void set_current_lambdas(gmx_int64_t step, t_lambda *fepvals, gmx_bool bRerunMD,
                         t_trxframe *rerun_fr, t_state *state_global, t_state *state, double lam0[]);

int multisim_min(const gmx_multisim_t *ms, int nmin, int n);
/* Set an appropriate value for n across the whole multi-simulation */

int multisim_nstsimsync(const t_commrec *cr,
                        const t_inputrec *ir, int repl_ex_nst);
/* Determine the interval for inter-simulation communication */

void copy_coupling_state(t_state *statea, t_state *stateb,
                         gmx_ekindata_t *ekinda, gmx_ekindata_t *ekindb, t_grpopts* opts);
/* Copy stuff from state A to state B */

void compute_globals(FILE *fplog, gmx_global_stat *gstat, t_commrec *cr, t_inputrec *ir,
                     t_forcerec *fr, gmx_ekindata_t *ekind,
                     t_state *state, t_mdatoms *mdatoms,
                     t_nrnb *nrnb, t_vcm *vcm, gmx_wallcycle_t wcycle,
                     gmx_enerdata_t *enerd, tensor force_vir, tensor shake_vir, tensor total_vir,
                     tensor pres, rvec mu_tot, gmx_constr *constr,
                     gmx_signalling_t *gs, gmx_bool bInterSimGS,
                     matrix box, int *totalNumberOfBondedInteractions,
                     gmx_bool *bSumEkinhOld, int flags);
/* Compute global variables during integration */

#endif

/*
 * This file is part of the GROMACS molecular simulation package.
 *
 * Copyright (c) 2012,2013,2014.2015, by the GROMACS development team, led by
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
 * Note: this file was generated by the GROMACS c kernel generator.
 */
#include "gmxpre.h"

#include "config.h"

#include <math.h>

#include "../nb_kernel.h"
#include "gromacs/math/vec.h"
#include "gromacs/gmxlib/nrnb.h"

/*
 * Gromacs nonbonded kernel:   nb_kernel_ElecRF_VdwBham_GeomW3W3_VF_c
 * Electrostatics interaction: ReactionField
 * VdW interaction:            Buckingham
 * Geometry:                   Water3-Water3
 * Calculate force/pot:        PotentialAndForce
 */
void
nb_kernel_ElecRF_VdwBham_GeomW3W3_VF_c
                    (t_nblist                    * gmx_restrict       nlist,
                     rvec                        * gmx_restrict          xx,
                     rvec                        * gmx_restrict          ff,
                     t_forcerec                  * gmx_restrict          fr,
                     t_mdatoms                   * gmx_restrict     mdatoms,
                     nb_kernel_data_t gmx_unused * gmx_restrict kernel_data,
                     t_nrnb                      * gmx_restrict        nrnb)
{
    int              i_shift_offset,i_coord_offset,j_coord_offset;
    int              j_index_start,j_index_end;
    int              nri,inr,ggid,iidx,jidx,jnr,outeriter,inneriter;
    real             shX,shY,shZ,tx,ty,tz,fscal,rcutoff,rcutoff2;
    int              *iinr,*jindex,*jjnr,*shiftidx,*gid;
    real             *shiftvec,*fshift,*x,*f;
    int              vdwioffset0;
    real             ix0,iy0,iz0,fix0,fiy0,fiz0,iq0,isai0;
    int              vdwioffset1;
    real             ix1,iy1,iz1,fix1,fiy1,fiz1,iq1,isai1;
    int              vdwioffset2;
    real             ix2,iy2,iz2,fix2,fiy2,fiz2,iq2,isai2;
    int              vdwjidx0;
    real             jx0,jy0,jz0,fjx0,fjy0,fjz0,jq0,isaj0;
    int              vdwjidx1;
    real             jx1,jy1,jz1,fjx1,fjy1,fjz1,jq1,isaj1;
    int              vdwjidx2;
    real             jx2,jy2,jz2,fjx2,fjy2,fjz2,jq2,isaj2;
    real             dx00,dy00,dz00,rsq00,rinv00,rinvsq00,r00,qq00,c6_00,c12_00,cexp1_00,cexp2_00;
    real             dx01,dy01,dz01,rsq01,rinv01,rinvsq01,r01,qq01,c6_01,c12_01,cexp1_01,cexp2_01;
    real             dx02,dy02,dz02,rsq02,rinv02,rinvsq02,r02,qq02,c6_02,c12_02,cexp1_02,cexp2_02;
    real             dx10,dy10,dz10,rsq10,rinv10,rinvsq10,r10,qq10,c6_10,c12_10,cexp1_10,cexp2_10;
    real             dx11,dy11,dz11,rsq11,rinv11,rinvsq11,r11,qq11,c6_11,c12_11,cexp1_11,cexp2_11;
    real             dx12,dy12,dz12,rsq12,rinv12,rinvsq12,r12,qq12,c6_12,c12_12,cexp1_12,cexp2_12;
    real             dx20,dy20,dz20,rsq20,rinv20,rinvsq20,r20,qq20,c6_20,c12_20,cexp1_20,cexp2_20;
    real             dx21,dy21,dz21,rsq21,rinv21,rinvsq21,r21,qq21,c6_21,c12_21,cexp1_21,cexp2_21;
    real             dx22,dy22,dz22,rsq22,rinv22,rinvsq22,r22,qq22,c6_22,c12_22,cexp1_22,cexp2_22;
    real             velec,felec,velecsum,facel,crf,krf,krf2;
    real             *charge;
    int              nvdwtype;
    real             rinvsix,rvdw,vvdw,vvdw6,vvdw12,fvdw,fvdw6,fvdw12,vvdwsum,br,vvdwexp,sh_vdw_invrcut6;
    int              *vdwtype;
    real             *vdwparam;

    x                = xx[0];
    f                = ff[0];

    nri              = nlist->nri;
    iinr             = nlist->iinr;
    jindex           = nlist->jindex;
    jjnr             = nlist->jjnr;
    shiftidx         = nlist->shift;
    gid              = nlist->gid;
    shiftvec         = fr->shift_vec[0];
    fshift           = fr->fshift[0];
    facel            = fr->epsfac;
    charge           = mdatoms->chargeA;
    krf              = fr->ic->k_rf;
    krf2             = krf*2.0;
    crf              = fr->ic->c_rf;
    nvdwtype         = fr->ntype;
    vdwparam         = fr->nbfp;
    vdwtype          = mdatoms->typeA;

    /* Setup water-specific parameters */
    inr              = nlist->iinr[0];
    iq0              = facel*charge[inr+0];
    iq1              = facel*charge[inr+1];
    iq2              = facel*charge[inr+2];
    vdwioffset0      = 3*nvdwtype*vdwtype[inr+0];

    jq0              = charge[inr+0];
    jq1              = charge[inr+1];
    jq2              = charge[inr+2];
    vdwjidx0         = 3*vdwtype[inr+0];
    qq00             = iq0*jq0;
    c6_00            = vdwparam[vdwioffset0+vdwjidx0];
    cexp1_00         = vdwparam[vdwioffset0+vdwjidx0+1];
    cexp2_00         = vdwparam[vdwioffset0+vdwjidx0+2];
    qq01             = iq0*jq1;
    qq02             = iq0*jq2;
    qq10             = iq1*jq0;
    qq11             = iq1*jq1;
    qq12             = iq1*jq2;
    qq20             = iq2*jq0;
    qq21             = iq2*jq1;
    qq22             = iq2*jq2;

    outeriter        = 0;
    inneriter        = 0;

    /* Start outer loop over neighborlists */
    for(iidx=0; iidx<nri; iidx++)
    {
        /* Load shift vector for this list */
        i_shift_offset   = DIM*shiftidx[iidx];
        shX              = shiftvec[i_shift_offset+XX];
        shY              = shiftvec[i_shift_offset+YY];
        shZ              = shiftvec[i_shift_offset+ZZ];

        /* Load limits for loop over neighbors */
        j_index_start    = jindex[iidx];
        j_index_end      = jindex[iidx+1];

        /* Get outer coordinate index */
        inr              = iinr[iidx];
        i_coord_offset   = DIM*inr;

        /* Load i particle coords and add shift vector */
        ix0              = shX + x[i_coord_offset+DIM*0+XX];
        iy0              = shY + x[i_coord_offset+DIM*0+YY];
        iz0              = shZ + x[i_coord_offset+DIM*0+ZZ];
        ix1              = shX + x[i_coord_offset+DIM*1+XX];
        iy1              = shY + x[i_coord_offset+DIM*1+YY];
        iz1              = shZ + x[i_coord_offset+DIM*1+ZZ];
        ix2              = shX + x[i_coord_offset+DIM*2+XX];
        iy2              = shY + x[i_coord_offset+DIM*2+YY];
        iz2              = shZ + x[i_coord_offset+DIM*2+ZZ];

        fix0             = 0.0;
        fiy0             = 0.0;
        fiz0             = 0.0;
        fix1             = 0.0;
        fiy1             = 0.0;
        fiz1             = 0.0;
        fix2             = 0.0;
        fiy2             = 0.0;
        fiz2             = 0.0;

        /* Reset potential sums */
        velecsum         = 0.0;
        vvdwsum          = 0.0;

        /* Start inner kernel loop */
        for(jidx=j_index_start; jidx<j_index_end; jidx++)
        {
            /* Get j neighbor index, and coordinate index */
            jnr              = jjnr[jidx];
            j_coord_offset   = DIM*jnr;

            /* load j atom coordinates */
            jx0              = x[j_coord_offset+DIM*0+XX];
            jy0              = x[j_coord_offset+DIM*0+YY];
            jz0              = x[j_coord_offset+DIM*0+ZZ];
            jx1              = x[j_coord_offset+DIM*1+XX];
            jy1              = x[j_coord_offset+DIM*1+YY];
            jz1              = x[j_coord_offset+DIM*1+ZZ];
            jx2              = x[j_coord_offset+DIM*2+XX];
            jy2              = x[j_coord_offset+DIM*2+YY];
            jz2              = x[j_coord_offset+DIM*2+ZZ];

            /* Calculate displacement vector */
            dx00             = ix0 - jx0;
            dy00             = iy0 - jy0;
            dz00             = iz0 - jz0;
            dx01             = ix0 - jx1;
            dy01             = iy0 - jy1;
            dz01             = iz0 - jz1;
            dx02             = ix0 - jx2;
            dy02             = iy0 - jy2;
            dz02             = iz0 - jz2;
            dx10             = ix1 - jx0;
            dy10             = iy1 - jy0;
            dz10             = iz1 - jz0;
            dx11             = ix1 - jx1;
            dy11             = iy1 - jy1;
            dz11             = iz1 - jz1;
            dx12             = ix1 - jx2;
            dy12             = iy1 - jy2;
            dz12             = iz1 - jz2;
            dx20             = ix2 - jx0;
            dy20             = iy2 - jy0;
            dz20             = iz2 - jz0;
            dx21             = ix2 - jx1;
            dy21             = iy2 - jy1;
            dz21             = iz2 - jz1;
            dx22             = ix2 - jx2;
            dy22             = iy2 - jy2;
            dz22             = iz2 - jz2;

            /* Calculate squared distance and things based on it */
            rsq00            = dx00*dx00+dy00*dy00+dz00*dz00;
            rsq01            = dx01*dx01+dy01*dy01+dz01*dz01;
            rsq02            = dx02*dx02+dy02*dy02+dz02*dz02;
            rsq10            = dx10*dx10+dy10*dy10+dz10*dz10;
            rsq11            = dx11*dx11+dy11*dy11+dz11*dz11;
            rsq12            = dx12*dx12+dy12*dy12+dz12*dz12;
            rsq20            = dx20*dx20+dy20*dy20+dz20*dz20;
            rsq21            = dx21*dx21+dy21*dy21+dz21*dz21;
            rsq22            = dx22*dx22+dy22*dy22+dz22*dz22;

            rinv00           = gmx_invsqrt(rsq00);
            rinv01           = gmx_invsqrt(rsq01);
            rinv02           = gmx_invsqrt(rsq02);
            rinv10           = gmx_invsqrt(rsq10);
            rinv11           = gmx_invsqrt(rsq11);
            rinv12           = gmx_invsqrt(rsq12);
            rinv20           = gmx_invsqrt(rsq20);
            rinv21           = gmx_invsqrt(rsq21);
            rinv22           = gmx_invsqrt(rsq22);

            rinvsq00         = rinv00*rinv00;
            rinvsq01         = rinv01*rinv01;
            rinvsq02         = rinv02*rinv02;
            rinvsq10         = rinv10*rinv10;
            rinvsq11         = rinv11*rinv11;
            rinvsq12         = rinv12*rinv12;
            rinvsq20         = rinv20*rinv20;
            rinvsq21         = rinv21*rinv21;
            rinvsq22         = rinv22*rinv22;

            /**************************
             * CALCULATE INTERACTIONS *
             **************************/

            r00              = rsq00*rinv00;

            /* REACTION-FIELD ELECTROSTATICS */
            velec            = qq00*(rinv00+krf*rsq00-crf);
            felec            = qq00*(rinv00*rinvsq00-krf2);

            /* BUCKINGHAM DISPERSION/REPULSION */
            rinvsix          = rinvsq00*rinvsq00*rinvsq00;
            vvdw6            = c6_00*rinvsix;
            br               = cexp2_00*r00;
            vvdwexp          = cexp1_00*exp(-br);
            vvdw             = vvdwexp - vvdw6*(1.0/6.0);
            fvdw             = (br*vvdwexp-vvdw6)*rinvsq00;

            /* Update potential sums from outer loop */
            velecsum        += velec;
            vvdwsum         += vvdw;

            fscal            = felec+fvdw;

            /* Calculate temporary vectorial force */
            tx               = fscal*dx00;
            ty               = fscal*dy00;
            tz               = fscal*dz00;

            /* Update vectorial force */
            fix0            += tx;
            fiy0            += ty;
            fiz0            += tz;
            f[j_coord_offset+DIM*0+XX] -= tx;
            f[j_coord_offset+DIM*0+YY] -= ty;
            f[j_coord_offset+DIM*0+ZZ] -= tz;

            /**************************
             * CALCULATE INTERACTIONS *
             **************************/

            /* REACTION-FIELD ELECTROSTATICS */
            velec            = qq01*(rinv01+krf*rsq01-crf);
            felec            = qq01*(rinv01*rinvsq01-krf2);

            /* Update potential sums from outer loop */
            velecsum        += velec;

            fscal            = felec;

            /* Calculate temporary vectorial force */
            tx               = fscal*dx01;
            ty               = fscal*dy01;
            tz               = fscal*dz01;

            /* Update vectorial force */
            fix0            += tx;
            fiy0            += ty;
            fiz0            += tz;
            f[j_coord_offset+DIM*1+XX] -= tx;
            f[j_coord_offset+DIM*1+YY] -= ty;
            f[j_coord_offset+DIM*1+ZZ] -= tz;

            /**************************
             * CALCULATE INTERACTIONS *
             **************************/

            /* REACTION-FIELD ELECTROSTATICS */
            velec            = qq02*(rinv02+krf*rsq02-crf);
            felec            = qq02*(rinv02*rinvsq02-krf2);

            /* Update potential sums from outer loop */
            velecsum        += velec;

            fscal            = felec;

            /* Calculate temporary vectorial force */
            tx               = fscal*dx02;
            ty               = fscal*dy02;
            tz               = fscal*dz02;

            /* Update vectorial force */
            fix0            += tx;
            fiy0            += ty;
            fiz0            += tz;
            f[j_coord_offset+DIM*2+XX] -= tx;
            f[j_coord_offset+DIM*2+YY] -= ty;
            f[j_coord_offset+DIM*2+ZZ] -= tz;

            /**************************
             * CALCULATE INTERACTIONS *
             **************************/

            /* REACTION-FIELD ELECTROSTATICS */
            velec            = qq10*(rinv10+krf*rsq10-crf);
            felec            = qq10*(rinv10*rinvsq10-krf2);

            /* Update potential sums from outer loop */
            velecsum        += velec;

            fscal            = felec;

            /* Calculate temporary vectorial force */
            tx               = fscal*dx10;
            ty               = fscal*dy10;
            tz               = fscal*dz10;

            /* Update vectorial force */
            fix1            += tx;
            fiy1            += ty;
            fiz1            += tz;
            f[j_coord_offset+DIM*0+XX] -= tx;
            f[j_coord_offset+DIM*0+YY] -= ty;
            f[j_coord_offset+DIM*0+ZZ] -= tz;

            /**************************
             * CALCULATE INTERACTIONS *
             **************************/

            /* REACTION-FIELD ELECTROSTATICS */
            velec            = qq11*(rinv11+krf*rsq11-crf);
            felec            = qq11*(rinv11*rinvsq11-krf2);

            /* Update potential sums from outer loop */
            velecsum        += velec;

            fscal            = felec;

            /* Calculate temporary vectorial force */
            tx               = fscal*dx11;
            ty               = fscal*dy11;
            tz               = fscal*dz11;

            /* Update vectorial force */
            fix1            += tx;
            fiy1            += ty;
            fiz1            += tz;
            f[j_coord_offset+DIM*1+XX] -= tx;
            f[j_coord_offset+DIM*1+YY] -= ty;
            f[j_coord_offset+DIM*1+ZZ] -= tz;

            /**************************
             * CALCULATE INTERACTIONS *
             **************************/

            /* REACTION-FIELD ELECTROSTATICS */
            velec            = qq12*(rinv12+krf*rsq12-crf);
            felec            = qq12*(rinv12*rinvsq12-krf2);

            /* Update potential sums from outer loop */
            velecsum        += velec;

            fscal            = felec;

            /* Calculate temporary vectorial force */
            tx               = fscal*dx12;
            ty               = fscal*dy12;
            tz               = fscal*dz12;

            /* Update vectorial force */
            fix1            += tx;
            fiy1            += ty;
            fiz1            += tz;
            f[j_coord_offset+DIM*2+XX] -= tx;
            f[j_coord_offset+DIM*2+YY] -= ty;
            f[j_coord_offset+DIM*2+ZZ] -= tz;

            /**************************
             * CALCULATE INTERACTIONS *
             **************************/

            /* REACTION-FIELD ELECTROSTATICS */
            velec            = qq20*(rinv20+krf*rsq20-crf);
            felec            = qq20*(rinv20*rinvsq20-krf2);

            /* Update potential sums from outer loop */
            velecsum        += velec;

            fscal            = felec;

            /* Calculate temporary vectorial force */
            tx               = fscal*dx20;
            ty               = fscal*dy20;
            tz               = fscal*dz20;

            /* Update vectorial force */
            fix2            += tx;
            fiy2            += ty;
            fiz2            += tz;
            f[j_coord_offset+DIM*0+XX] -= tx;
            f[j_coord_offset+DIM*0+YY] -= ty;
            f[j_coord_offset+DIM*0+ZZ] -= tz;

            /**************************
             * CALCULATE INTERACTIONS *
             **************************/

            /* REACTION-FIELD ELECTROSTATICS */
            velec            = qq21*(rinv21+krf*rsq21-crf);
            felec            = qq21*(rinv21*rinvsq21-krf2);

            /* Update potential sums from outer loop */
            velecsum        += velec;

            fscal            = felec;

            /* Calculate temporary vectorial force */
            tx               = fscal*dx21;
            ty               = fscal*dy21;
            tz               = fscal*dz21;

            /* Update vectorial force */
            fix2            += tx;
            fiy2            += ty;
            fiz2            += tz;
            f[j_coord_offset+DIM*1+XX] -= tx;
            f[j_coord_offset+DIM*1+YY] -= ty;
            f[j_coord_offset+DIM*1+ZZ] -= tz;

            /**************************
             * CALCULATE INTERACTIONS *
             **************************/

            /* REACTION-FIELD ELECTROSTATICS */
            velec            = qq22*(rinv22+krf*rsq22-crf);
            felec            = qq22*(rinv22*rinvsq22-krf2);

            /* Update potential sums from outer loop */
            velecsum        += velec;

            fscal            = felec;

            /* Calculate temporary vectorial force */
            tx               = fscal*dx22;
            ty               = fscal*dy22;
            tz               = fscal*dz22;

            /* Update vectorial force */
            fix2            += tx;
            fiy2            += ty;
            fiz2            += tz;
            f[j_coord_offset+DIM*2+XX] -= tx;
            f[j_coord_offset+DIM*2+YY] -= ty;
            f[j_coord_offset+DIM*2+ZZ] -= tz;

            /* Inner loop uses 318 flops */
        }
        /* End of innermost loop */

        tx = ty = tz = 0;
        f[i_coord_offset+DIM*0+XX] += fix0;
        f[i_coord_offset+DIM*0+YY] += fiy0;
        f[i_coord_offset+DIM*0+ZZ] += fiz0;
        tx                         += fix0;
        ty                         += fiy0;
        tz                         += fiz0;
        f[i_coord_offset+DIM*1+XX] += fix1;
        f[i_coord_offset+DIM*1+YY] += fiy1;
        f[i_coord_offset+DIM*1+ZZ] += fiz1;
        tx                         += fix1;
        ty                         += fiy1;
        tz                         += fiz1;
        f[i_coord_offset+DIM*2+XX] += fix2;
        f[i_coord_offset+DIM*2+YY] += fiy2;
        f[i_coord_offset+DIM*2+ZZ] += fiz2;
        tx                         += fix2;
        ty                         += fiy2;
        tz                         += fiz2;
        fshift[i_shift_offset+XX]  += tx;
        fshift[i_shift_offset+YY]  += ty;
        fshift[i_shift_offset+ZZ]  += tz;

        ggid                        = gid[iidx];
        /* Update potential energies */
        kernel_data->energygrp_elec[ggid] += velecsum;
        kernel_data->energygrp_vdw[ggid] += vvdwsum;

        /* Increment number of inner iterations */
        inneriter                  += j_index_end - j_index_start;

        /* Outer loop uses 32 flops */
    }

    /* Increment number of outer iterations */
    outeriter        += nri;

    /* Update outer/inner flops */

    inc_nrnb(nrnb,eNR_NBKERNEL_ELEC_VDW_W3W3_VF,outeriter*32 + inneriter*318);
}
/*
 * Gromacs nonbonded kernel:   nb_kernel_ElecRF_VdwBham_GeomW3W3_F_c
 * Electrostatics interaction: ReactionField
 * VdW interaction:            Buckingham
 * Geometry:                   Water3-Water3
 * Calculate force/pot:        Force
 */
void
nb_kernel_ElecRF_VdwBham_GeomW3W3_F_c
                    (t_nblist                    * gmx_restrict       nlist,
                     rvec                        * gmx_restrict          xx,
                     rvec                        * gmx_restrict          ff,
                     t_forcerec                  * gmx_restrict          fr,
                     t_mdatoms                   * gmx_restrict     mdatoms,
                     nb_kernel_data_t gmx_unused * gmx_restrict kernel_data,
                     t_nrnb                      * gmx_restrict        nrnb)
{
    int              i_shift_offset,i_coord_offset,j_coord_offset;
    int              j_index_start,j_index_end;
    int              nri,inr,ggid,iidx,jidx,jnr,outeriter,inneriter;
    real             shX,shY,shZ,tx,ty,tz,fscal,rcutoff,rcutoff2;
    int              *iinr,*jindex,*jjnr,*shiftidx,*gid;
    real             *shiftvec,*fshift,*x,*f;
    int              vdwioffset0;
    real             ix0,iy0,iz0,fix0,fiy0,fiz0,iq0,isai0;
    int              vdwioffset1;
    real             ix1,iy1,iz1,fix1,fiy1,fiz1,iq1,isai1;
    int              vdwioffset2;
    real             ix2,iy2,iz2,fix2,fiy2,fiz2,iq2,isai2;
    int              vdwjidx0;
    real             jx0,jy0,jz0,fjx0,fjy0,fjz0,jq0,isaj0;
    int              vdwjidx1;
    real             jx1,jy1,jz1,fjx1,fjy1,fjz1,jq1,isaj1;
    int              vdwjidx2;
    real             jx2,jy2,jz2,fjx2,fjy2,fjz2,jq2,isaj2;
    real             dx00,dy00,dz00,rsq00,rinv00,rinvsq00,r00,qq00,c6_00,c12_00,cexp1_00,cexp2_00;
    real             dx01,dy01,dz01,rsq01,rinv01,rinvsq01,r01,qq01,c6_01,c12_01,cexp1_01,cexp2_01;
    real             dx02,dy02,dz02,rsq02,rinv02,rinvsq02,r02,qq02,c6_02,c12_02,cexp1_02,cexp2_02;
    real             dx10,dy10,dz10,rsq10,rinv10,rinvsq10,r10,qq10,c6_10,c12_10,cexp1_10,cexp2_10;
    real             dx11,dy11,dz11,rsq11,rinv11,rinvsq11,r11,qq11,c6_11,c12_11,cexp1_11,cexp2_11;
    real             dx12,dy12,dz12,rsq12,rinv12,rinvsq12,r12,qq12,c6_12,c12_12,cexp1_12,cexp2_12;
    real             dx20,dy20,dz20,rsq20,rinv20,rinvsq20,r20,qq20,c6_20,c12_20,cexp1_20,cexp2_20;
    real             dx21,dy21,dz21,rsq21,rinv21,rinvsq21,r21,qq21,c6_21,c12_21,cexp1_21,cexp2_21;
    real             dx22,dy22,dz22,rsq22,rinv22,rinvsq22,r22,qq22,c6_22,c12_22,cexp1_22,cexp2_22;
    real             velec,felec,velecsum,facel,crf,krf,krf2;
    real             *charge;
    int              nvdwtype;
    real             rinvsix,rvdw,vvdw,vvdw6,vvdw12,fvdw,fvdw6,fvdw12,vvdwsum,br,vvdwexp,sh_vdw_invrcut6;
    int              *vdwtype;
    real             *vdwparam;

    x                = xx[0];
    f                = ff[0];

    nri              = nlist->nri;
    iinr             = nlist->iinr;
    jindex           = nlist->jindex;
    jjnr             = nlist->jjnr;
    shiftidx         = nlist->shift;
    gid              = nlist->gid;
    shiftvec         = fr->shift_vec[0];
    fshift           = fr->fshift[0];
    facel            = fr->epsfac;
    charge           = mdatoms->chargeA;
    krf              = fr->ic->k_rf;
    krf2             = krf*2.0;
    crf              = fr->ic->c_rf;
    nvdwtype         = fr->ntype;
    vdwparam         = fr->nbfp;
    vdwtype          = mdatoms->typeA;

    /* Setup water-specific parameters */
    inr              = nlist->iinr[0];
    iq0              = facel*charge[inr+0];
    iq1              = facel*charge[inr+1];
    iq2              = facel*charge[inr+2];
    vdwioffset0      = 3*nvdwtype*vdwtype[inr+0];

    jq0              = charge[inr+0];
    jq1              = charge[inr+1];
    jq2              = charge[inr+2];
    vdwjidx0         = 3*vdwtype[inr+0];
    qq00             = iq0*jq0;
    c6_00            = vdwparam[vdwioffset0+vdwjidx0];
    cexp1_00         = vdwparam[vdwioffset0+vdwjidx0+1];
    cexp2_00         = vdwparam[vdwioffset0+vdwjidx0+2];
    qq01             = iq0*jq1;
    qq02             = iq0*jq2;
    qq10             = iq1*jq0;
    qq11             = iq1*jq1;
    qq12             = iq1*jq2;
    qq20             = iq2*jq0;
    qq21             = iq2*jq1;
    qq22             = iq2*jq2;

    outeriter        = 0;
    inneriter        = 0;

    /* Start outer loop over neighborlists */
    for(iidx=0; iidx<nri; iidx++)
    {
        /* Load shift vector for this list */
        i_shift_offset   = DIM*shiftidx[iidx];
        shX              = shiftvec[i_shift_offset+XX];
        shY              = shiftvec[i_shift_offset+YY];
        shZ              = shiftvec[i_shift_offset+ZZ];

        /* Load limits for loop over neighbors */
        j_index_start    = jindex[iidx];
        j_index_end      = jindex[iidx+1];

        /* Get outer coordinate index */
        inr              = iinr[iidx];
        i_coord_offset   = DIM*inr;

        /* Load i particle coords and add shift vector */
        ix0              = shX + x[i_coord_offset+DIM*0+XX];
        iy0              = shY + x[i_coord_offset+DIM*0+YY];
        iz0              = shZ + x[i_coord_offset+DIM*0+ZZ];
        ix1              = shX + x[i_coord_offset+DIM*1+XX];
        iy1              = shY + x[i_coord_offset+DIM*1+YY];
        iz1              = shZ + x[i_coord_offset+DIM*1+ZZ];
        ix2              = shX + x[i_coord_offset+DIM*2+XX];
        iy2              = shY + x[i_coord_offset+DIM*2+YY];
        iz2              = shZ + x[i_coord_offset+DIM*2+ZZ];

        fix0             = 0.0;
        fiy0             = 0.0;
        fiz0             = 0.0;
        fix1             = 0.0;
        fiy1             = 0.0;
        fiz1             = 0.0;
        fix2             = 0.0;
        fiy2             = 0.0;
        fiz2             = 0.0;

        /* Start inner kernel loop */
        for(jidx=j_index_start; jidx<j_index_end; jidx++)
        {
            /* Get j neighbor index, and coordinate index */
            jnr              = jjnr[jidx];
            j_coord_offset   = DIM*jnr;

            /* load j atom coordinates */
            jx0              = x[j_coord_offset+DIM*0+XX];
            jy0              = x[j_coord_offset+DIM*0+YY];
            jz0              = x[j_coord_offset+DIM*0+ZZ];
            jx1              = x[j_coord_offset+DIM*1+XX];
            jy1              = x[j_coord_offset+DIM*1+YY];
            jz1              = x[j_coord_offset+DIM*1+ZZ];
            jx2              = x[j_coord_offset+DIM*2+XX];
            jy2              = x[j_coord_offset+DIM*2+YY];
            jz2              = x[j_coord_offset+DIM*2+ZZ];

            /* Calculate displacement vector */
            dx00             = ix0 - jx0;
            dy00             = iy0 - jy0;
            dz00             = iz0 - jz0;
            dx01             = ix0 - jx1;
            dy01             = iy0 - jy1;
            dz01             = iz0 - jz1;
            dx02             = ix0 - jx2;
            dy02             = iy0 - jy2;
            dz02             = iz0 - jz2;
            dx10             = ix1 - jx0;
            dy10             = iy1 - jy0;
            dz10             = iz1 - jz0;
            dx11             = ix1 - jx1;
            dy11             = iy1 - jy1;
            dz11             = iz1 - jz1;
            dx12             = ix1 - jx2;
            dy12             = iy1 - jy2;
            dz12             = iz1 - jz2;
            dx20             = ix2 - jx0;
            dy20             = iy2 - jy0;
            dz20             = iz2 - jz0;
            dx21             = ix2 - jx1;
            dy21             = iy2 - jy1;
            dz21             = iz2 - jz1;
            dx22             = ix2 - jx2;
            dy22             = iy2 - jy2;
            dz22             = iz2 - jz2;

            /* Calculate squared distance and things based on it */
            rsq00            = dx00*dx00+dy00*dy00+dz00*dz00;
            rsq01            = dx01*dx01+dy01*dy01+dz01*dz01;
            rsq02            = dx02*dx02+dy02*dy02+dz02*dz02;
            rsq10            = dx10*dx10+dy10*dy10+dz10*dz10;
            rsq11            = dx11*dx11+dy11*dy11+dz11*dz11;
            rsq12            = dx12*dx12+dy12*dy12+dz12*dz12;
            rsq20            = dx20*dx20+dy20*dy20+dz20*dz20;
            rsq21            = dx21*dx21+dy21*dy21+dz21*dz21;
            rsq22            = dx22*dx22+dy22*dy22+dz22*dz22;

            rinv00           = gmx_invsqrt(rsq00);
            rinv01           = gmx_invsqrt(rsq01);
            rinv02           = gmx_invsqrt(rsq02);
            rinv10           = gmx_invsqrt(rsq10);
            rinv11           = gmx_invsqrt(rsq11);
            rinv12           = gmx_invsqrt(rsq12);
            rinv20           = gmx_invsqrt(rsq20);
            rinv21           = gmx_invsqrt(rsq21);
            rinv22           = gmx_invsqrt(rsq22);

            rinvsq00         = rinv00*rinv00;
            rinvsq01         = rinv01*rinv01;
            rinvsq02         = rinv02*rinv02;
            rinvsq10         = rinv10*rinv10;
            rinvsq11         = rinv11*rinv11;
            rinvsq12         = rinv12*rinv12;
            rinvsq20         = rinv20*rinv20;
            rinvsq21         = rinv21*rinv21;
            rinvsq22         = rinv22*rinv22;

            /**************************
             * CALCULATE INTERACTIONS *
             **************************/

            r00              = rsq00*rinv00;

            /* REACTION-FIELD ELECTROSTATICS */
            felec            = qq00*(rinv00*rinvsq00-krf2);

            /* BUCKINGHAM DISPERSION/REPULSION */
            rinvsix          = rinvsq00*rinvsq00*rinvsq00;
            vvdw6            = c6_00*rinvsix;
            br               = cexp2_00*r00;
            vvdwexp          = cexp1_00*exp(-br);
            fvdw             = (br*vvdwexp-vvdw6)*rinvsq00;

            fscal            = felec+fvdw;

            /* Calculate temporary vectorial force */
            tx               = fscal*dx00;
            ty               = fscal*dy00;
            tz               = fscal*dz00;

            /* Update vectorial force */
            fix0            += tx;
            fiy0            += ty;
            fiz0            += tz;
            f[j_coord_offset+DIM*0+XX] -= tx;
            f[j_coord_offset+DIM*0+YY] -= ty;
            f[j_coord_offset+DIM*0+ZZ] -= tz;

            /**************************
             * CALCULATE INTERACTIONS *
             **************************/

            /* REACTION-FIELD ELECTROSTATICS */
            felec            = qq01*(rinv01*rinvsq01-krf2);

            fscal            = felec;

            /* Calculate temporary vectorial force */
            tx               = fscal*dx01;
            ty               = fscal*dy01;
            tz               = fscal*dz01;

            /* Update vectorial force */
            fix0            += tx;
            fiy0            += ty;
            fiz0            += tz;
            f[j_coord_offset+DIM*1+XX] -= tx;
            f[j_coord_offset+DIM*1+YY] -= ty;
            f[j_coord_offset+DIM*1+ZZ] -= tz;

            /**************************
             * CALCULATE INTERACTIONS *
             **************************/

            /* REACTION-FIELD ELECTROSTATICS */
            felec            = qq02*(rinv02*rinvsq02-krf2);

            fscal            = felec;

            /* Calculate temporary vectorial force */
            tx               = fscal*dx02;
            ty               = fscal*dy02;
            tz               = fscal*dz02;

            /* Update vectorial force */
            fix0            += tx;
            fiy0            += ty;
            fiz0            += tz;
            f[j_coord_offset+DIM*2+XX] -= tx;
            f[j_coord_offset+DIM*2+YY] -= ty;
            f[j_coord_offset+DIM*2+ZZ] -= tz;

            /**************************
             * CALCULATE INTERACTIONS *
             **************************/

            /* REACTION-FIELD ELECTROSTATICS */
            felec            = qq10*(rinv10*rinvsq10-krf2);

            fscal            = felec;

            /* Calculate temporary vectorial force */
            tx               = fscal*dx10;
            ty               = fscal*dy10;
            tz               = fscal*dz10;

            /* Update vectorial force */
            fix1            += tx;
            fiy1            += ty;
            fiz1            += tz;
            f[j_coord_offset+DIM*0+XX] -= tx;
            f[j_coord_offset+DIM*0+YY] -= ty;
            f[j_coord_offset+DIM*0+ZZ] -= tz;

            /**************************
             * CALCULATE INTERACTIONS *
             **************************/

            /* REACTION-FIELD ELECTROSTATICS */
            felec            = qq11*(rinv11*rinvsq11-krf2);

            fscal            = felec;

            /* Calculate temporary vectorial force */
            tx               = fscal*dx11;
            ty               = fscal*dy11;
            tz               = fscal*dz11;

            /* Update vectorial force */
            fix1            += tx;
            fiy1            += ty;
            fiz1            += tz;
            f[j_coord_offset+DIM*1+XX] -= tx;
            f[j_coord_offset+DIM*1+YY] -= ty;
            f[j_coord_offset+DIM*1+ZZ] -= tz;

            /**************************
             * CALCULATE INTERACTIONS *
             **************************/

            /* REACTION-FIELD ELECTROSTATICS */
            felec            = qq12*(rinv12*rinvsq12-krf2);

            fscal            = felec;

            /* Calculate temporary vectorial force */
            tx               = fscal*dx12;
            ty               = fscal*dy12;
            tz               = fscal*dz12;

            /* Update vectorial force */
            fix1            += tx;
            fiy1            += ty;
            fiz1            += tz;
            f[j_coord_offset+DIM*2+XX] -= tx;
            f[j_coord_offset+DIM*2+YY] -= ty;
            f[j_coord_offset+DIM*2+ZZ] -= tz;

            /**************************
             * CALCULATE INTERACTIONS *
             **************************/

            /* REACTION-FIELD ELECTROSTATICS */
            felec            = qq20*(rinv20*rinvsq20-krf2);

            fscal            = felec;

            /* Calculate temporary vectorial force */
            tx               = fscal*dx20;
            ty               = fscal*dy20;
            tz               = fscal*dz20;

            /* Update vectorial force */
            fix2            += tx;
            fiy2            += ty;
            fiz2            += tz;
            f[j_coord_offset+DIM*0+XX] -= tx;
            f[j_coord_offset+DIM*0+YY] -= ty;
            f[j_coord_offset+DIM*0+ZZ] -= tz;

            /**************************
             * CALCULATE INTERACTIONS *
             **************************/

            /* REACTION-FIELD ELECTROSTATICS */
            felec            = qq21*(rinv21*rinvsq21-krf2);

            fscal            = felec;

            /* Calculate temporary vectorial force */
            tx               = fscal*dx21;
            ty               = fscal*dy21;
            tz               = fscal*dz21;

            /* Update vectorial force */
            fix2            += tx;
            fiy2            += ty;
            fiz2            += tz;
            f[j_coord_offset+DIM*1+XX] -= tx;
            f[j_coord_offset+DIM*1+YY] -= ty;
            f[j_coord_offset+DIM*1+ZZ] -= tz;

            /**************************
             * CALCULATE INTERACTIONS *
             **************************/

            /* REACTION-FIELD ELECTROSTATICS */
            felec            = qq22*(rinv22*rinvsq22-krf2);

            fscal            = felec;

            /* Calculate temporary vectorial force */
            tx               = fscal*dx22;
            ty               = fscal*dy22;
            tz               = fscal*dz22;

            /* Update vectorial force */
            fix2            += tx;
            fiy2            += ty;
            fiz2            += tz;
            f[j_coord_offset+DIM*2+XX] -= tx;
            f[j_coord_offset+DIM*2+YY] -= ty;
            f[j_coord_offset+DIM*2+ZZ] -= tz;

            /* Inner loop uses 270 flops */
        }
        /* End of innermost loop */

        tx = ty = tz = 0;
        f[i_coord_offset+DIM*0+XX] += fix0;
        f[i_coord_offset+DIM*0+YY] += fiy0;
        f[i_coord_offset+DIM*0+ZZ] += fiz0;
        tx                         += fix0;
        ty                         += fiy0;
        tz                         += fiz0;
        f[i_coord_offset+DIM*1+XX] += fix1;
        f[i_coord_offset+DIM*1+YY] += fiy1;
        f[i_coord_offset+DIM*1+ZZ] += fiz1;
        tx                         += fix1;
        ty                         += fiy1;
        tz                         += fiz1;
        f[i_coord_offset+DIM*2+XX] += fix2;
        f[i_coord_offset+DIM*2+YY] += fiy2;
        f[i_coord_offset+DIM*2+ZZ] += fiz2;
        tx                         += fix2;
        ty                         += fiy2;
        tz                         += fiz2;
        fshift[i_shift_offset+XX]  += tx;
        fshift[i_shift_offset+YY]  += ty;
        fshift[i_shift_offset+ZZ]  += tz;

        /* Increment number of inner iterations */
        inneriter                  += j_index_end - j_index_start;

        /* Outer loop uses 30 flops */
    }

    /* Increment number of outer iterations */
    outeriter        += nri;

    /* Update outer/inner flops */

    inc_nrnb(nrnb,eNR_NBKERNEL_ELEC_VDW_W3W3_F,outeriter*30 + inneriter*270);
}

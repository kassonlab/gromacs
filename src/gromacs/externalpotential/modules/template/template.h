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
#ifndef _PULLING_H
#define _PULLING_H

#include <memory>
#include <string>

#include "gromacs/externalpotential/externalpotential.h"
#include "gromacs/externalpotential/modules.h"
#include "gromacs/mdtypes/commrec.h"
namespace gmx
{

class Template : public ExternalPotential
{
    public:

        static std::unique_ptr<ExternalPotential> create( struct ext_pot_ir *ep_ir, t_commrec * cr, t_inputrec * ir, const gmx_mtop_t* mtop, const rvec x[], matrix box, FILE *input_file, FILE *output_file, FILE *fplog, bool bVerbose, const gmx_output_env_t *oenv, unsigned long Flags, int number_groups);
        void do_potential(t_commrec *cr, t_inputrec *ir, matrix box, const rvec x[], real t, gmx_int64_t step, gmx_wallcycle_t wcycle, bool bNS);

    private:

        Template(struct ext_pot_ir *ep_ir, t_commrec * cr, t_inputrec * ir, const gmx_mtop_t* mtop, const rvec x[], matrix box, FILE * input_file_p, FILE *output_file_p, FILE *fplog, bool bVerbose, const gmx_output_env_t *oenv, unsigned long Flags, int number_groups);
        real k_;

};

class TemplateInfo
{
    public:
        static std::string name;
        static std::string shortDescription;
        static const int   numberIndexGroups;
        static externalpotential::ModuleCreator create;
};

} // namespace gmx

#endif

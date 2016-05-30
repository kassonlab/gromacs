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

#ifndef GMX_EXTERNALPOTENTIAL_MODULES_H
#define GMX_EXTERNALPOTENTIAL_MODULES_H
#include <memory>
#include "gromacs/math/vectypes.h"
#include "externalpotential.h"
#include <string>
#include <map>
#include <vector>
namespace gmx
{
namespace externalpotential
{

typedef std::unique_ptr<ExternalPotential> (* ModuleCreator) ();

struct Modules
{
    typedef std::string ModuleName;
    struct ModuleProperties
    {
        std::string   shortDescription;
        int           numberIndexGroups;
        int           numberWholeMoleculeGroups;
        ModuleCreator create;
    };
    std::map<ModuleName, ModuleProperties> module;
};

/*! \brief
 * Registers all trajectory analysis command-line modules.
 *
 * \param[in] manager  Command-line module manager to receive the modules.
 * \throws    std::bad_alloc if out of memory.
 *
 * Registers all trajectory analysis modules declared in the library such that
 * they can be run through \p manager.
 *
 * \ingroup module_trajectoryanalysis
 */
void registerExternalPotentialModules(Modules *modules);


} // namespace exteranlpotential
} // namespace gmx

 #endif

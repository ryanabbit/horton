// Horton is a development platform for electronic structure methods.
// Copyright (C) 2011-2013 Toon Verstraelen <Toon.Verstraelen@UGent.be>
//
// This file is part of Horton.
//
// Horton is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// Horton is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>
//
//--


#ifndef HORTON_NUCPOT_H
#define HORTON_NUCPOT_H

/** @brief
        Compute the electrostatic potential due to the nuclei on a grid.

    @param numbers
        The pointer to an array with atomic numbers. This array contains natom
        elements.

    @param coordinates
        The pointer to an array with atomic coordinates. This array contains
        3*natom elements of an (atom,3) array in row-major ordering.

    @param natom
        The number of atoms.

    @param points
        The pointer to the array with coordinates of the grid points. This array
        contains 3*npoint elements of an (npoint,3) array in row-major ordering.

    @param output
        The pointer to the output array. This array constins npoint elements.

    @param npoint
        The number of grid points.

 */
void compute_grid_nucpot(long* numbers, double* coordinates, long natom,
                         double* points, double* output, long npoint);

#endif

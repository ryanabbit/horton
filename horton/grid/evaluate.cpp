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

//#define DEBUG

#ifdef DEBUG
#include <cstdio>
#endif

#include <cmath>
#include <stdexcept>
#include "evaluate.h"


void eval_spline_cube(CubicSpline* spline, double* center, double* output,
                      UniformGrid* ugrid) {

    // Find the ranges for the triple loop
    double rcut = spline->get_last_x();
    long begin[3], end[3];
    ugrid->set_ranges_rcut(center, rcut, begin, end);

    Block3Iterator b3i = Block3Iterator(begin, end, ugrid->shape);

    // Run triple loop over blocks (serial)
    for (long iblock=b3i.get_nblock()-1; iblock>=0; iblock--) {
        long b[3];
        b3i.set_block(iblock, b);

        long cube_begin[3];
        long cube_end[3];
        b3i.set_cube_ranges(b, cube_begin, cube_end);

        // Run triple loop within one block (parallel)
        Cube3Iterator c3i = Cube3Iterator(cube_begin, cube_end);
        #pragma omp parallel for
        for (long ipoint=c3i.get_npoint()-1; ipoint>=0; ipoint--) {
            long j[3];
            long jwrap[3];
            c3i.set_point(ipoint, jwrap);
            b3i.translate(b, jwrap, j);

            double d = ugrid->dist_grid_point(center, j);

            // Evaluate spline if needed
            if ((d < rcut) || spline->get_extrapolation()->has_tail()) {
                double s;
                spline->eval(&d, &s, 1);
                *(ugrid->get_pointer(output, jwrap)) += s;
            }

        }
    }
}

void eval_spline_grid(CubicSpline* spline, double* center, double* output,
                      double* points, Cell* cell, long npoint) {
    double rcut = spline->get_last_x();

    while (npoint > 0) {
        // Find the ranges for the triple loop
        double delta[3];
        delta[0] = points[0] - center[0];
        delta[1] = points[1] - center[1];
        delta[2] = points[2] - center[2];
        long ranges_begin[3], ranges_end[3];
        cell->set_ranges_rcut(delta, rcut, ranges_begin, ranges_end);

        for (int i=cell->get_nvec(); i < 3; i++) {
            ranges_begin[i] = 0;
            ranges_end[i] = 1;
        }

        // Run the triple loop
        for (long i0 = ranges_begin[0]; i0 < ranges_end[0]; i0++) {
            for (long i1 = ranges_begin[1]; i1 < ranges_end[1]; i1++) {
                for (long i2 = ranges_begin[2]; i2 < ranges_end[2]; i2++) {
                    // Compute the distance between the point and the image of the center
                    double frac[3], cart[3];
                    frac[0] = i0;
                    frac[1] = i1;
                    frac[2] = i2;
                    cell->to_cart(frac, cart);
                    double x = cart[0] + delta[0];
                    double y = cart[1] + delta[1];
                    double z = cart[2] + delta[2];
                    double d = sqrt(x*x+y*y+z*z);

                    // Evaluate spline if needed
                    if ((d < rcut) || spline->get_extrapolation()->has_tail()) {
                        double s;
                        spline->eval(&d, &s, 1);
#ifdef DEBUG
                        printf("i=[%li,%li,%li] d=%f s=%f ||", i0, i1, i2, d, s);
#endif
                        *output += s;
                    }

                }
            }
        }

        // move on
        points += 3;
        output++;
        npoint--;
    }

#ifdef DEBUG
    printf("\n");
#endif

}

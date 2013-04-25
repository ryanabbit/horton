// Horton is a Density Functional Theory program.
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

#include <stdexcept>
#include <cmath>
#include <cstdlib>
#include "uniform.h"

UniformIntGrid::UniformIntGrid(double* _origin, Cell* _grid_cell, long* _shape, long* _pbc, Cell* _cell)
    : grid_cell(_grid_cell), cell(_cell) {

    if (_grid_cell->get_nvec() != 3) {
        throw std::domain_error("The grid_cell of a UniformIntGrid must be 3D.");
    }

    for (int i=0; i<3; i++) {
        origin[i] = _origin[i];
        shape[i] = _shape[i];
        pbc[i] = _pbc[i];
    }
}

void UniformIntGrid::copy_origin(double* output) {
    output[0] = origin[0];
    output[1] = origin[1];
    output[2] = origin[2];
}

void UniformIntGrid::copy_shape(long* output) {
    output[0] = shape[0];
    output[1] = shape[1];
    output[2] = shape[2];
}

void UniformIntGrid::copy_pbc(long* output) {
    output[0] = pbc[0];
    output[1] = pbc[1];
    output[2] = pbc[2];
}



void UniformIntGrid::set_ranges_rcut(double* center, double rcut, long* ranges_begin, long* ranges_end) {
    double delta[3];
    delta[0] = origin[0] - center[0];
    delta[1] = origin[1] - center[1];
    delta[2] = origin[2] - center[2];
    grid_cell->set_ranges_rcut(delta, rcut, ranges_begin, ranges_end);

    // Truncate ranges in case of non-periodic boundary conditions
    for (int i=2; i>=0; i--) {
        if (!pbc[i]) {
            if (ranges_begin[i] < 0) {
                ranges_begin[i] = 0;
            }
            if (ranges_end[i] > shape[i]) {
                ranges_end[i] = shape[i];
            }
        }
    }

#ifdef DEBUG
    printf("ranges [%li,%li,%li] [%li,%li,%li]\n", ranges_begin[0], ranges_begin[1], ranges_begin[2], ranges_end[0], ranges_end[1], ranges_end[2]);
#endif
}

double UniformIntGrid::dist_grid_point(double* center, long* i) {
    double delta[3];
    delta[0] = origin[0] - center[0];
    delta[1] = origin[1] - center[1];
    delta[2] = origin[2] - center[2];
    grid_cell->add_rvec(delta, i);
    return sqrt(delta[0]*delta[0]+delta[1]*delta[1]+delta[2]*delta[2]);
}

void UniformIntGrid::delta_grid_point(double* center, long* i) {
    // center is used here as a input; the final value is the relative vector;
    center[0] = origin[0] - center[0];
    center[1] = origin[1] - center[1];
    center[2] = origin[2] - center[2];
    grid_cell->add_rvec(center, i);
}

double* UniformIntGrid::get_pointer(double* array, long* i) {
    return array + (i[0]*shape[1] + i[1])*shape[2] + i[2];
}


UniformIntGridWindow::UniformIntGridWindow(UniformIntGrid* ui_grid, long* _begin, long* _end)
    : ui_grid(ui_grid) {

    for (int i=0; i<3; i++) {
        begin[i] = _begin[i];
        end[i] = _end[i];
        shape[i] = end[i] - begin[i];
    }
}

void UniformIntGridWindow::copy_begin(long* output) {
    output[0] = begin[0];
    output[1] = begin[1];
    output[2] = begin[2];
}

void UniformIntGridWindow::copy_end(long* output) {
    output[0] = end[0];
    output[1] = end[1];
    output[2] = end[2];
}

void UniformIntGridWindow::extend(double* cell, double* local) {
    Range3Iterator r3i = Range3Iterator(begin, end, ui_grid->get_shape());
    long j[3], jwrap[3];
    for (long ipoint=r3i.get_npoint()-1; ipoint >= 0; ipoint--) {
        r3i.set_point(ipoint, j, jwrap);
#ifdef DEBUG
        printf("ipoint=%li  j={%li, %li, %li}  jwrap={%li, %li, %li}\n", ipoint, j[0], j[1], j[2], jwrap[0], jwrap[1], jwrap[2]);
#endif
        double* source = ui_grid->get_pointer(cell, jwrap);
        local[ipoint] = *source;
    }
}

void UniformIntGridWindow::wrap(double* local, double* cell) {
    // Run triple loop over blocks
    Block3Iterator b3i = Block3Iterator(begin, end, ui_grid->get_shape());
    for (long iblock=b3i.get_nblock()-1; iblock>=0; iblock--) {
        long b[3];
        b3i.set_block(iblock, b);

        long cube_begin[3];
        long cube_end[3];
        b3i.set_cube_ranges(b, cube_begin, cube_end);

        // Run triple loop within one block
        Cube3Iterator c3i = Cube3Iterator(cube_begin, cube_end);
        for (long ipoint=c3i.get_npoint()-1; ipoint>=0; ipoint--) {
            long j[3];
            long jwrap[3];
            c3i.set_point(ipoint, jwrap);
            b3i.translate(b, jwrap, j);
            *(ui_grid->get_pointer(cell, jwrap)) += *get_pointer(local, j);

        }
    }
}

double* UniformIntGridWindow::get_pointer(double* array, long* i) {
    return array + ((i[0]-begin[0])*shape[1] + (i[1]-begin[1]))*shape[2] + (i[2]-begin[2]);
}


long index_wrap(long i, long high) {
    // Get around compiler weirdness
    long result = i%high;
    if (result<0) result += high;
    return result;
}


Range3Iterator::Range3Iterator(const long* ranges_begin, const long* ranges_end, const long* shape) :
    ranges_begin(ranges_begin), ranges_end(ranges_end), shape(shape) {

    loop_shape[0] = ranges_end[0];
    loop_shape[1] = ranges_end[1];
    loop_shape[2] = ranges_end[2];
    if (ranges_begin!=NULL) {
        loop_shape[0] -= ranges_begin[0];
        loop_shape[1] -= ranges_begin[1];
        loop_shape[2] -= ranges_begin[2];
    }
    npoint = loop_shape[0] * loop_shape[1] * loop_shape[2];
};

void Range3Iterator::set_point(long ipoint, long* i, long* iwrap) {
    i[2] = ipoint%loop_shape[2];
    ipoint /= loop_shape[2];
    i[1] = ipoint%loop_shape[1];
    i[0] = ipoint/loop_shape[1];
    if (ranges_begin!=NULL) {
        i[0] += ranges_begin[0];
        i[1] += ranges_begin[1];
        i[2] += ranges_begin[2];
    }
    if (iwrap!=NULL) {
        iwrap[0] = index_wrap(i[0], shape[0]);
        iwrap[1] = index_wrap(i[1], shape[1]);
        iwrap[2] = index_wrap(i[2], shape[2]);
    }
}


Block3Iterator::Block3Iterator(const long* begin, const long* end, const long* shape)
    : begin(begin), end(end), shape(shape) {

    for (int i=0; i<3; i++) {
        // Clumsy code to avoid troubles with different standards for integer
        // division of negative numbers.
        if (begin[i] >= 0) {
            block_begin[i] = begin[i] / shape[i];
        } else {
            block_begin[i] = -1 - labs(begin[i]+1) / shape[i];
        }
        if (end[i] > 0) {
            block_end[i] = (end[i]-1) / shape[i] + 1;
        } else {
            block_end[i] = -labs(end[i]) / shape[i];
        }
        block_shape[i] = block_end[i] - block_begin[i];
    }
#ifdef DEBUG
    printf("begin=[%li %li %li]\n", begin[0], begin[1], begin[2]);
    printf("end=[%li %li %li]\n", end[0], end[1], end[2]);
    printf("shape=[%li %li %li]\n", shape[0], shape[1], shape[2]);
    printf("block_begin=[%li %li %li]\n", block_begin[0], block_begin[1], block_begin[2]);
    printf("block_end=[%li %li %li]\n", block_end[0], block_end[1], block_end[2]);
    printf("block_shape=[%li %li %li]\n", block_shape[0], block_shape[1], block_shape[2]);
#endif
    nblock = block_shape[0]*block_shape[1]*block_shape[2];
}

void Block3Iterator::copy_block_begin(long* output) {
    for (int i=0; i<3; i++) output[i] = block_begin[i];
}

void Block3Iterator::copy_block_end(long* output) {
    for (int i=0; i<3; i++) output[i] = block_end[i];
}

void Block3Iterator::set_block(long iblock, long* b) {
    b[2] = block_begin[2] + iblock % block_shape[2];
    iblock /= block_shape[2];
    b[1] = block_begin[1] + iblock % block_shape[1];
    b[0] = block_begin[0] + iblock / block_shape[1];
}

void Block3Iterator::set_cube_ranges(long* b, long* cube_begin, long* cube_end) {
    for (int i=0; i<3; i++) {
        long offset = b[i]*shape[i];
        cube_begin[i] = offset;
        if (cube_begin[i] < begin[i]) cube_begin[i] = begin[i];
        cube_begin[i] -= offset;

        cube_end[i] = offset + shape[i];
        if (cube_end[i] > end[i]) cube_end[i] = end[i];
        cube_end[i] -= offset;
    }
}

void Block3Iterator::translate(long* b, long* jwrap, long* j) {
    j[0] = jwrap[0] + b[0]*shape[0];
    j[1] = jwrap[1] + b[1]*shape[1];
    j[2] = jwrap[2] + b[2]*shape[2];
}


Cube3Iterator::Cube3Iterator(const long* begin, const long* end)
    : begin(begin), end(end) {
    shape[0] = end[0];
    shape[1] = end[1];
    shape[2] = end[2];
    if (begin!=NULL) {
        shape[0] -= begin[0];
        shape[1] -= begin[1];
        shape[2] -= begin[2];
    }
    npoint = shape[0]*shape[1]*shape[2];
}

void Cube3Iterator::set_point(long ipoint, long* j) {
    j[2] = ipoint % shape[2];
    ipoint /= shape[2];
    j[1] = ipoint % shape[1];
    j[0] = ipoint / shape[1];
    if (begin!=NULL) {
        j[0] += begin[0];
        j[1] += begin[1];
        j[2] += begin[2];
    }
}

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


#ifndef HORTON_GBASIS_INTS_H
#define HORTON_GBASIS_INTS_H

#include "calc.h"
#include "iter_pow.h"
#include "libint2.h"


class GB2Integral : public GBCalculator {
    protected:
        long shell_type0, shell_type1;
        const double *r0, *r1;
        IterPow2 i2p;
    public:
        GB2Integral(long max_shell_type);
        void reset(long shell_type0, long shell_type1, const double* r0, const double* r1);
        virtual void add(double coeff, double alpha0, double alpha1, const double* scales0, const double* scales1) = 0;
        void cart_to_pure();
        const long get_shell_type0() const {return shell_type0;};
        const long get_shell_type1() const {return shell_type1;};
    };


class GB2OverlapIntegral: public GB2Integral {
    public:
        GB2OverlapIntegral(long max_shell_type) : GB2Integral(max_shell_type) {};
        virtual void add(double coeff, double alpha0, double alpha1, const double* scales0, const double* scales1);
    };


class GB2KineticIntegral: public GB2Integral {
    public:
        GB2KineticIntegral(long max_shell_type) : GB2Integral(max_shell_type) {};
        virtual void add(double coeff, double alpha0, double alpha1, const double* scales0, const double* scales1);
    };


class GB2NuclearAttractionIntegral: public GB2Integral {
    private:
        double* charges;
        double* centers;
        long ncharge;

        double* work_g0;
        double* work_g1;
        double* work_g2;
        double* work_boys;
    public:
        GB2NuclearAttractionIntegral(long max_shell_type, double* charges, double* centers, long ncharge);
        ~GB2NuclearAttractionIntegral();
        virtual void add(double coeff, double alpha0, double alpha1, const double* scales0, const double* scales1);
    };


class GB4Integral : public GBCalculator {
    protected:
        long shell_type0, shell_type1, shell_type2, shell_type3;
        const double *r0, *r1, *r2, *r3;
    public:
        GB4Integral(long max_shell_type);
        virtual void reset(long shell_type0, long shell_type1, long shell_type2, long shell_type3, const double* r0, const double* r1, const double* r2, const double* r3);
        virtual void add(double coeff, double alpha0, double alpha1, double alpha2, double alpha3, const double* scales0, const double* scales1, const double* scales2, const double* scales3) = 0;
        void cart_to_pure();

        const long get_shell_type0() const {return shell_type0;};
        const long get_shell_type1() const {return shell_type1;};
        const long get_shell_type2() const {return shell_type2;};
        const long get_shell_type3() const {return shell_type3;};
    };


typedef struct {
    unsigned int am;
    const double* r;
    double alpha;
} libint_arg_t;

class GB4ElectronReuplsionIntegralLibInt : public GB4Integral {
    private:
        Libint_eri_t erieval;
        libint_arg_t libint_args[4];
        long order[4];
        double ab[3], cd[3];
        double ab2, cd2;
    public:
        GB4ElectronReuplsionIntegralLibInt(long max_shell_type);
        ~GB4ElectronReuplsionIntegralLibInt();
        virtual void reset(long shell_type0, long shell_type1, long shell_type2, long shell_type3, const double* r0, const double* r1, const double* r2, const double* r3);
        virtual void add(double coeff, double alpha0, double alpha1, double alpha2, double alpha3, const double* scales0, const double* scales1, const double* scales2, const double* scales3);
    };


#endif

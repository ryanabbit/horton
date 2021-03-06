Atoms-in-Molecules (AIM) analysis
#################################


.. contents::


Introduction
============

Horton supports several real-space atoms-in-molecules (AIM) schemes. A
wavefunction or an electronic density on a grid can be loaded from several
file formats. With this input, several AIM schemes can be used to derive AIM
observables, e.g. atomic charge, atomic multipole expansion, etc.

Horton supports two different approaches to perform the partitioning: WPart
(based on wavefunction files) and CPart (based on cube files). Both
implementations build on the same algorithms but use different types of
grids for the numerical integrals. An overview of both implementations is
given in the following table:

======================== =========================== =========================== ============
Feature                  WPart                       CPart                       References
======================== =========================== =========================== ============
Command-line script      ``horton-wpart.py``         ``horton-cpart.py``
**Boundary conditions**
0D                       X                           X
1D                                                   X
2D                                                   X
3D                                                   X
**Input file formats**
Gaussian03/09 fchk       X                                                       http://www.gaussian.com/
Molekel (Orca)           X                                                       http://cec.mpg.de/forum/
Molden input (Orca)      X                                                       http://cec.mpg.de/forum/
Gaussian/GAMESS wfn      X                                                       http://www.gaussian.com/ http://www.msg.ameslab.gov/gamess/
Gaussian03/09 cube                                   X                           http://www.gaussian.com/
Vasp CHGCAR                                          X                           https://www.vasp.at/
**AIM schemes**
Becke                    X                                                       [becke1988_multicenter]_
Hirshfeld                X                           X                           [hirshfeld1977]_
Iterative Hirshfeld      X                           X                           [bultinck2007]_
Iterative Stockholder    X                                                       [lillestolen2008]_
Extended Hirshfeld       X                           X                           [verstraelen2013]_
**AIM properties**
Charges                  X                           X
Spin charges             X
Cartesian multipoles     X                           X
Pure/harmonic multipoles X                           X
Radial moments           X                           X
Dispersion coefficients  X                           X                           [tkatchenko2009]_
**Extra options**
Symmetry analysis                                    X
======================== =========================== =========================== ============

Note that Gaussian cube files can be generated with many other codes like CPMD, ADF,
Siesta, Crystal, etc. The output of all these program should be compatible with
Horton. Horton can partition all-electron and pseudo-potential wavefunctions.
However, for all-electron partitioning with CPart, very fine grids are required.

For all the Hirshfeld variants, one must first set up a database of proatomic
densities, preferably at the same level of theory used for the molecular
computation. The script ``horton-atomdb.py`` facilitate the setup of such a
database.

All output generated by Horton is written to `HDF5
<http://www.hdfgroup.org/HDF5/>`_ files (with extension ``.h5``). These are
binary (compact) platform-independent files that can be post-processed easily
with Python scripts. The script ``horton-hdf2csv.py`` can be used to convert
(part of) an HDF5 file into the "comma-separated value" (CSV) format, which is
supported by most spreadsheet software.

The usage of the four scripts (``horton-atomdb.py``, ``horton-wpart.py``,
``horton-cpart.py`` and ``horton-hdf2csv.py``) will be discussed in the
following sections. All scripts have a ``--help`` option that prints out a
complete list of all options. The penultimate section shows how one can use the
partitioning code through the Python interface. The last section answers some
frequently asked questions about partitioning with Horton.


``horton-atomdb.py`` -- Set up a database of proatoms
=====================================================

The usage of the script ``horton-atomdb.py`` consists of three steps:

1) **Generate input files** for isolated atom computations for one of the following
   codes: Gaussian03/09, Orca or CP2K. The following example generates
   Gaussian09 inputs for hydrogen, carbon, nitrogen and oxygen::

    horton-atomdb.py input g09 1,6-8 template.com

   The file ``template.com`` is used to generate the input files and will
   be discussed in detail below. A series of directories is created with input
   files for the atomic computations: ``001__h_001_q+00``, ``001__h_002_q-01``,
   ``001__h_003_q-02``, ... Optional arguments can be used to control the range
   of cations and anions, the spin multiplicities considered, etc. Also a script
   ``run_g09.sh`` is present that takes care of the next step.

   Run ``horton-atomdb.py input --help`` to obtain a complete list of all
   options.

2) **Run the atomic computations.** Just execute the ``run_PROGRAM.sh`` script.
   In this case::

    ./run_g09.sh

3) **Convert the output** of the external programs into a database of
   spherically averaged pro-atom densities (``atoms.h5``). Just run::

    ./horton-atomdb.py convert

   This script also generates figures of the radial densities and Fukui
   functions if ``matplotlib`` is installed. In this step, one may use
   :ref:`ref_grid_option`, although the default setting should be fine for
   nearly all cases.

One may remove some directories with atomic computations before or after
executing the ``run_PROGRAM.sh`` script. The corresponding atoms will not be
included in the database. Similarly, one may rerun ``horton-atomdb.py input``
to generate more input files. In that case ``run_PROGRAM.sh`` will only consider
the atomic computations that are not completed yet.


Template files
--------------

A template file is simply an input file for an atomic computation, where the
critical parameters (element, charge, ...) are replaced keys that are recognized
by the input generator of ``horton-atomdb.py``. These keys are:

* ``${element}``: The element symbol of the atom.
* ``${number}``: The element number of the atom.
* ``${charge}``: The charge of the atom (or kation, or anion).
* ``${mult}``: The spin multiplicity of the atom

For the more advanced cases, one may include (parts of) other files with generic
keys, e.g. for basis sets that are different for every element:

* ``${file:filename}``: This is replaced by the contents of
  ``filename.NNN_PPP_MM``, where ``NNN`` is the element number, ``PPP`` is the
  atomic population and ``MM`` is the spin multiplicity. These numbers are
  left-padded with zeros to fix the the length. If a field in the filename is
  zero, it is considered as a wild card. For example, one may use
  ``${file:basis}`` in the template file and store a basis set specification for
  oxygen in the file ``basis.008_000_00``.

* ``${line:filename}``. This comparable to the previous, except that all
  replacements are stored in one file. Each (non-empty) line in that file starts
  with ``NNN_PPP_MM``, which is then followed by the string that will be filled
  into the field.

None of the keys is mandatory, although ``${element}`` (or ``${number}``,
``${charge}`` and ``${mult}`` must be present to obtain sensible results.


Simple template file for Gaussian 03/09
---------------------------------------

Thia is a basic template file for atomic computations at the HF/3-21G level::

    %chk=atom.chk
    #p HF/3-21G scf(xqc)

    A random title line

    ${charge} ${mult}
    ${element} 0.0 0.0 0.0

Do not forget to include an empty line at the end. Otherwise, Gaussian will
complain. The first line ``%chk=atom.chk`` is required to read in the atomic
wavefunction in the ``convert`` step of ``horton-atomdb.py``.


Advanced template file for Gaussian 03/09
-----------------------------------------

When custom basis sets are specified with the ``Gen`` keyword in Gaussian, one
has to use keys that include other files. For a database with H, C and O, one
could use the following template:

* ``template.com``::

    %chk=atom.chk
    #p PBE1PBE/Gen scf(xqc)

    A random title line

    ${charge} ${mult}
    ${element} 0.0 0.0 0.0

    ${file:basis}

* ``basis.001_000_00``::

    H 0
    6-31G(d,p)
    ****

* ``basis.006_000_00``::

    C 0
    6-31G(d,p)
    ****

* ``basis.008_000_00``::

    O     0
    S   6   1.00
       8588.5000000              0.00189515
       1297.2300000              0.0143859
        299.2960000              0.0707320
         87.3771000              0.2400010
         25.6789000              0.5947970
          3.7400400              0.2808020
    SP   3   1.00
         42.1175000              0.1138890              0.0365114
          9.6283700              0.9208110              0.2371530
          2.8533200             -0.00327447             0.8197020
    SP   1   1.00
          0.9056610              1.0000000              1.0000000
    SP   1   1.00
          0.2556110              1.0000000              1.0000000
    SP   1   1.00
          0.0845000              1.0000000              1.0000000
    D   1   1.00
          5.1600000              1.0000000
    D   1   1.00
          1.2920000              1.0000000
    D   1   1.00
          0.3225000              1.0000000
    F   1   1.00
          1.4000000              1.0000000
    ****


Simple template file for ORCA
-----------------------------

The following template file use the built-in ``cc_pVQZ`` basis set of ORCA::

    !HF TightSCF

    %basis
      Basis cc_pVQZ
    end

    *xyz ${charge} ${mult}
    ${element} 0.0 0.0 0.0
    *


Template file for CP2K
----------------------

One must use ``CP2K`` version 2.4-r12857 (or newer). The computation of proatoms
with ``CP2K`` is more involved because one has to specify the occupation
of each subshell. The ``ATOM`` program of ``CP2K`` does not simply follow the Aufbau
rule to assign to orbital occupations. For now, only the computation of atomic
densities with contracted basis sets and pseudopotentials is supported, mainly
because this is case where ``CP2K`` offers additional functionality.

The following example can be used to generator a proatomic database with the
elements, O, Na, Al and Si with the GTH pseudopotential and the MolOpt basis
set.

* ``template.inp``::

    &GLOBAL
      PROJECT ATOM
      PROGRAM_NAME ATOM
    &END GLOBAL
    &ATOM
      ATOMIC_NUMBER ${number}
      ELECTRON_CONFIGURATION (${mult}) CORE ${line:valence.inc}
      CORE none

      MAX_ANGULAR_MOMENTUM 1
      &METHOD
         METHOD_TYPE UKS
         &XC
           &XC_FUNCTIONAL PBE
           &END XC_FUNCTIONAL
         &END XC
      &END METHOD
      &POTENTIAL
          PSEUDO_TYPE GTH
          POTENTIAL_FILE_NAME ../../PBE_PSEUDOPOTENTIALS
          POTENTIAL_NAME ${line:ppot.inc}
      &END POTENTIAL
      &PP_BASIS
          BASIS_SET_FILE_NAME ../../BASIS_MOLOPT
          BASIS_TYPE CONTRACTED_GTO
          BASIS_SET DZVP-MOLOPT-SR-GTH
      &END PP_BASIS
      &PRINT
        &BASIS_SET ON
        &END
        &ORBITALS ON
        &END
        &POTENTIAL ON
        &END
      &END
    &END ATOM

* ``ppot.inc``::

    008_000_00 GTH-PBE-q6
    011_000_00 GTH-PBE-q9
    013_000_00 GTH-PBE-q3
    014_000_00 GTH-PBE-q4

* ``valence.inc``::

    008_005_02 1s2 2p1
    008_006_03 1s2 2p2
    008_007_04 1s2 2p3
    008_008_03 1s2 2p4
    008_009_02 1s2 2p5
    008_010_01 1s2 2p6

    011_005_02 1s2 2p1
    011_006_03 1s2 2p2
    011_007_04 1s2 2p3
    011_008_03 1s2 2p4
    011_009_02 1s2 2p5
    011_010_01 1s2 2p6
    011_011_02 1s2 2p6 2s1
    011_012_01 1s2 2p6 2s2
    011_013_02 1s2 2p6 2s2 3p1

    013_011_02 1s1
    013_012_01 1s2
    013_013_02 1s2 2p1
    013_014_03 1s2 2p2

    014_011_02 1s1
    014_012_01 1s2
    014_013_02 1s2 2p1
    014_014_03 1s2 2p2


``horton-wpart.py`` -- AIM analysis based on a wavefunction file
================================================================

The basic usage of ``horton-wpart.py`` is as follows::

    horton-wpart.py wfn output.h5[:group] {b,h,hi,he} [atoms.h5]

This script has four important arguments:

1. The ``wfn`` argument can be a Gaussian03 or 09 formatted checkpoint file, a
   Molekel file or a Molden input file.

2. The output file in which the partitioning results are written. The output
   is written in HDF5 format. Usually, the output file has the extension
   ``.h5``. The output file is optionally followed by a colon and a group
   suffix. This suffix can be used to specify a group in the HDF5 output file
   where the output is stored.

3. The third argument refers to the partitioning scheme:

    * ``b``: Becke partitioning. [becke1988_multicenter]_
    * ``h``: Hirshfeld partitioning. [hirshfeld1977]_
    * ``hi``: Iterative Hirshfeld partitioning. [bultinck2007]_
    * ``is``: Iterative Stockholder partitioning. [lillestolen2008]_
    * ``he``: Extended Hirshfeld partitioning. [verstraelen2013]_

4. The ``atoms.h5`` argument is only needed for the Hirshfeld variants, not for
   ``b`` and ``is``. This script computes atomic weight functions and then
   derives all AIM observables that are implemented for that scheme. These
   results are stored in a HDF5 file with the same name as the ``wfn`` file but
   with a suffix ``_wpart.h5``. In this HDF5 file, the results are written in
   the group ``wpart/${SCHEME}`` where ``${SCHEME}`` is any of ``b``, ``h``,
   ``hi``, ``is``, ``he``.

Run ``horton-wpart.py --help`` to get a complete list of all command-line
options. The integration grid can be tuned with :ref:`ref_grid_option`.

.. note::

    When a post Hartree-Fock level is used in Gaussian 03/09 (MP2, MP3, CC or
    CI), one must add the keyword ``Density=current`` to the commands in the
    Gaussian input file. This is needed to have the corresponding density matrix
    in the formatted checkpoint file. When such a post HF density matrix is
    present, Horton will load that density matrix instead of the SCF density
    matrix. Also note that for some levels of theory, no 1RDM is constructed,
    including MP4, MP5, ZINDO and QCISD(T).


``horton-cpart.py`` -- AIM analysis based on a cube file
========================================================

The basic usage of ``horton-cpart.py`` is as follows::

    horton-cpart.py cube output.h5:group {h,hi,he} atoms.h5

The script takes three arguments:

1. The ``cube`` argument can be a Gaussian cube file (possibly generated with
   another program, e.g. CP2K). Some information must be added to the cube file
   about the effective core charges that were used. This information is needed
   to compute the correct net charges. (This is not only relevant for the final
   output, but also for constructing the proper pro-atoms. When these numbers
   are not set correctly, you'll get garbage output.)

   For example, given an original cube file with the following header::

           -Quickstep-
            ELECTRON DENSITY
              18    0.000000    0.000000    0.000000
              54    0.183933    0.000000    0.000000
              75    0.000000    0.187713    0.000000
              81    0.000000    0.000000    0.190349
               8    0.000000    0.000000    3.677294    0.000000
               8    0.000000    4.966200   10.401166    0.000000
               8    0.000000    0.000000   10.401166    0.000000
               8    0.000000    4.966200    3.677294    0.000000
               8    0.000000    2.483100    0.000000    2.243351
               8    0.000000    7.449300    0.000000   13.174925
               8    0.000000    2.483100    4.554391    4.206115
               8    0.000000    2.483100    9.524087    4.206115
               8    0.000000    7.449300    9.524087   11.212180
               8    0.000000    7.449300    4.554391   11.212180
               8    0.000000    4.966200    7.039230    7.709138
               8    0.000000    0.000000    7.039230    7.709138
              14    0.000000    2.483100    2.971972    1.606588
              14    0.000000    2.483100   11.106506    1.606588
              14    0.000000    7.449300   11.106506   13.811687
              14    0.000000    7.449300    2.971972   13.811687
              14    0.000000    2.483100    7.039230    5.959157
              14    0.000000    7.449300    7.039230    9.459119

   Now consider the case that 6 effective electrons were used for oxygen and 4
   effective electrons for silicon. Then, the second column in the atom lines
   has to be set to the effective core charge. (This number is not used in the
   cube format.) In this case, one gets::

           -Quickstep-
            ELECTRON DENSITY
              18    0.000000    0.000000    0.000000
              54    0.183933    0.000000    0.000000
              75    0.000000    0.187713    0.000000
              81    0.000000    0.000000    0.190349
               8    6.0         0.000000    3.677294    0.000000
               8    6.0         4.966200   10.401166    0.000000
               8    6.0         0.000000   10.401166    0.000000
               8    6.0         4.966200    3.677294    0.000000
               8    6.0         2.483100    0.000000    2.243351
               8    6.0         7.449300    0.000000   13.174925
               8    6.0         2.483100    4.554391    4.206115
               8    6.0         2.483100    9.524087    4.206115
               8    6.0         7.449300    9.524087   11.212180
               8    6.0         7.449300    4.554391   11.212180
               8    6.0         4.966200    7.039230    7.709138
               8    6.0         0.000000    7.039230    7.709138
              14    4.0         2.483100    2.971972    1.606588
              14    4.0         2.483100   11.106506    1.606588
              14    4.0         7.449300   11.106506   13.811687
              14    4.0         7.449300    2.971972   13.811687
              14    4.0         2.483100    7.039230    5.959157
              14    4.0         7.449300    7.039230    9.459119

2. The output file in which the partitioning results are written. The output
   is written in HDF5 format. Usually, the output file has the extension
   ``.h5``. The output file is optionally followed by a colon and a group
   suffix. This suffix can be used to specify a group in the HDF5 output file
   where the output is stored.

3. The third argument refers to the partitioning scheme:

        * ``h``: Hirshfeld partitioning. [hirshfeld1977]_
        * ``hi``: Iterative Hirshfeld partitioning. [bultinck2007]_
        * ``he``: Extended Hirshfeld partitioning. [verstraelen2013]_

4. The fourth argument is the atom database generated with ``horton-atomdb.py``

The ``horton-cpart.py`` script
computes atomic weight functions and then derives all AIM observables that are
implemented for that scheme. These results are stored in a HDF5 file with
the same name as the ``cube`` file but with a ``_cpart.h5`` suffix. In this HDF5
file, the results are written in the group ``cpart/${SCHEME}_r${STRIDE}`` where
``${SCHEME}`` is any of ``h``, ``hi``, ``he`` and ``${STRIDE}`` is an optional
argument of the script ``horton-cpart.py`` that is discussed below.

The ``horton-cpart.py`` script is somewhat experimental. Always make sure that the
numbers have converged with an increasing number of grid points. One may need to
following options to control the efficiency of the program

* ``--compact COMPACT``. Automatically determine cutoff radii for the pro-atoms,
  where ``COMPACT`` is the maximum number of electrons lost in the tail after
  the cutoff radius. 0.001 is typically a reasonable value for ``COMPACT``. The
  pro-atoms are renormalized after setting the cutoff radii. One cutoff radius
  is defined per element. This implies that the tail of the most diffuse anion
  determines the cutoff radius when the ``--compact`` option is used.

* ``--greedy``. This enables a more memory-hungry version of the Iterative and
  Extended Hirshfeld algorithms that runs considerably faster. This becomes
  unfeasible for systems with huge unit cells.

* ``--stride STRIDE``. The ``STRIDE`` parameter controls the subsampling of the
  cube file prior to the partitioning. It is ``1`` by default.

Run ``horton-cpart.py --help`` to get a complete list of all command-line
options.


Python interface to the partitioning code
=========================================

The ``horton-wpart.py`` and ``horton-cpart.py`` scripts have a rather intuitive
Python interface that allows one to run a more customized analysis. The script
``data/examples/004_wpart/run.py`` is a simple example that runs a Becke partitioning
where only the charges are computed and written to a simple text file.

.. literalinclude:: ../data/examples/004_wpart/run.py

The unit tests in the source code contain many small examples that can be used
as a starting point for similar scripts. These unit tests can be found in
``horton/part/test/test_wpart.py`` and ``horton/part/test/test_cpart.py``.


Frequently asked questions
==========================

**Which atoms-in-molecules (AIM or partitioning) scheme should I use?**

    There is no single partitioning scheme that is most suitable for any
    application. Nevertheless, some people claim that only one AIM scheme should
    be used above all others, especially in the QTAIM and electron density
    communities. Try to stay away from such fanboyism as it has little to do
    with good science.

    In practice, the choice depends on the purposes one has in mind. One should
    try to select a scheme that shows some desirable behavior.
    Typically, one would like to have a compromise between some of the following
    features:

    * Numerical stability.
    * Robustness with respect to conformational changes.
    * Uniqueness of the result.
    * Computational efficiency.
    * Mathematical elegance.
    * Simplicity.
    * Linearity in the density (matrix).
    * Chemical transferability.
    * Applicable to a broad range of systems.
    * Applicable to periodic/isolated systems.
    * Applicable to a broad range of electronic structure methods/implementations.
    * Accuracy of electrostatic interactions with a limited multipole expansion
      of the atoms.
    * Good (empirical) correlations between some AIM property with some experimental
      observable.
    * ...

    It goes beyond the scope of this FAQ to describe how each partitioning scheme
    (implemented in Horton or not) performs for these criteria. Some of these
    features are also very hard to assess and subject of intense debate in the
    literature.

    Regarding our own work, the following papers are directly related to this
    question:

    * [verstraelen2011a]_ "Assessment of Atomic Charge Models for {Gas-Phase} Computations on Polypeptides"
    * [verstraelen2012a]_ "The conformational sensitivity of iterative stockholder partitioning schemes"
    * [verstraelen2013]_ "Hirshfeld-E Partitioning: AIM Charges with an Improved Trade-off between Robustness and Accurate Electrostatics"

    The following are also related to this question, but the list is far from
    complete:

    * [bultinck2007]_ "Critical analysis and extension of the Hirshfeld atoms in molecules"
    * [bultinck2007b]_ "Uniqueness and basis set dependence of iterative Hirshfeld charges"

    If you have more suggestions, please drop me a note: Toon.Verstraelen@UGent.be.


**What is the recommended level of theory for the computation of the database of proatoms?**

    This question is relevant the following methods implemented in Horton:
    Hirshfeld, Hirshfeld-I and Hirshfeld-E. They are referred to in this answer
    as Hirshfeld-like schemes.

    In principle, one is free to choose any level of theory one prefers. In
    practice, several papers tend to be consistent in the level of theory (and
    basis set) that is used for the molecular and proatomic computations.
    See for example: [bultinck2007]_, [verstraelen2009]_, [verstraelen2011a]_,
    [verstraelen2011b]_, [vanduyfhuys2012]_, [verstraelen2012a]_,
    [verstraelen2012b]_, [verstraelen2013]_.

    One motivation for this consistency is that Hirshfeld-like partitioning
    schemes yield atoms-in-molecules that are maximally similar to the
    pro-atoms. [nalewajski2000]_ (Technically speaking, the sum over all atoms
    of the `Kullback-Leibler divergence
    <https://en.wikipedia.org/wiki/Kullback%E2%80%93Leibler_divergence>`_
    between the atom-in-molecule and proatom is minimized.)
    This principle is (or should be) one of the reasons that Hirhsfeld-like
    charges are somewhat transferable between chemically similar atoms. One
    could hope that the Kullback-Leibler divergence is easier to minimize when
    the molecular and proatomic densities are computed as consistently as
    possible, hence with the same level of theory and basis set.

    Another motivation is that such consistency may lead to a degree of error
    cancellation when comparing Hirshfeld-like charges computed at different
    levels of theory. For example, it is found that Hirshfeld-I charges have
    only a small basis set dependence. [bultinck2007b]_ In this work, the
    molecular and proatomic densities were computed consistently.

    At last, one could also argue that without the consistency in level of
    theory, there are some many possible combinations that it becomes impossible
    to make a well-motivated choice. Then again, this is a problem that
    computational chemist usually embrace with open arms in the hope that they
    will find a weird combination of levels of theory that turns out to be
    useful.

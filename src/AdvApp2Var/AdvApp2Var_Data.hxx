// Copyright (c) 1999-2014 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.

#ifndef AdvApp2Var_Data_HeaderFile
#define AdvApp2Var_Data_HeaderFile

#include <Standard_Macro.hxx>
#include <AdvApp2Var_Data_f2c.hxx>

//
struct mdnombr_1_1
{
  doublereal pi, deuxpi, pisur2, pis180, c180pi, zero, one, a180, a360, a90;
};

//
struct minombr_1_1
{
  integer nbr[1001];
};

//
struct maovpar_1_1
{
  doublereal r8und, r8ovr, x4und, x4ovr;
  real       r4und, r4ovr;
  integer    r4nbe, r8nbm, r8nbe, i4ovr, i4ovn, r4exp, r8exp, r4exn, r8exn, r4ncs, r8ncs, r4nbm;
  shortint   i2ovr, i2ovn;
};

//
struct maovpch_1_1
{
  char cnmmac[16], frmr4[8], frmr8[8], cdcode[8];
};

//
struct mlgdrtl_1_1
{
  doublereal rootab[930], // was [465][2]
    hiltab[930],          // was [465][2]
    hi0tab[31];
};

//
struct mmjcobi_1_1
{
  doublereal plgcan[3968]; // was [496][2][4]
  doublereal canjac[3968]; // was [496][2][4]
};

//
struct mmcmcnp_1_1
{
  doublereal cnp[3721]; // was [61][61] ;
};

//
struct mmapgss_1_1
{
  doublereal gslxjs[5017], gsl0js[52];
};

//
struct mmapgs0_1_1
{
  doublereal gslxj0[4761], gsl0j0[49];
};

//
struct mmapgs1_1_1
{
  doublereal gslxj1[4505], gsl0j1[46];
};

//
struct mmapgs2_1_1
{
  doublereal gslxj2[4249], gsl0j2[43];
};

////
class Data
{
public:
  Standard_EXPORT static mdnombr_1_1& Getmdnombr();
  Standard_EXPORT static minombr_1_1& Getminombr();
  Standard_EXPORT static maovpar_1_1& Getmaovpar();
  Standard_EXPORT static maovpch_1_1& Getmaovpch();
  Standard_EXPORT static mlgdrtl_1_1& Getmlgdrtl();
  Standard_EXPORT static mmjcobi_1_1& Getmmjcobi();
  Standard_EXPORT static mmcmcnp_1_1& Getmmcmcnp();
  Standard_EXPORT static mmapgss_1_1& Getmmapgss();
  Standard_EXPORT static mmapgs0_1_1& Getmmapgs0();
  Standard_EXPORT static mmapgs1_1_1& Getmmapgs1();
  Standard_EXPORT static mmapgs2_1_1& Getmmapgs2();
};

//
#define mdnombr_ Data::Getmdnombr()
#define minombr_ Data::Getminombr()
#define maovpar_ Data::Getmaovpar()
#define maovpch_ Data::Getmaovpch()
#define mlgdrtl_ Data::Getmlgdrtl()
#define mmjcobi_ Data::Getmmjcobi()
#define mmcmcnp_ Data::Getmmcmcnp()
#define mmapgss_ Data::Getmmapgss()
#define mmapgs0_ Data::Getmmapgs0()
#define mmapgs1_ Data::Getmmapgs1()
#define mmapgs2_ Data::Getmmapgs2()
//
#endif

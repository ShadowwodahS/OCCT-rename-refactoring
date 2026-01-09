// Created on: 1995-12-04
// Created by: Laurent BOURESCHE
// Copyright (c) 1995-1999 Matra Datavision
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

#include <GeomFill_Boundary.hxx>
#include <GeomFill_CoonsAlgPatch.hxx>
#include <GeomFill_TgtOnCoons.hxx>
#include <gp_Vec.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(GeomFill_TgtOnCoons, TangentField)

//=================================================================================================

GeomFill_TgtOnCoons::GeomFill_TgtOnCoons(const Handle(GeomFill_CoonsAlgPatch)& K,
                                         const Standard_Integer                I)
    : myK(K),
      ibound(I)
{
}

//=================================================================================================

Vector3d GeomFill_TgtOnCoons::Value(const Standard_Real W) const
{
  Standard_Real U = 0., V = 0., bid = 0.;
  switch (ibound)
  {
    case 0:
      myK->Bound(1)->Bounds(V, bid);
      break;
    case 1:
      myK->Bound(0)->Bounds(bid, U);
      break;
    case 2:
      myK->Bound(1)->Bounds(bid, V);
      break;
    case 3:
      myK->Bound(0)->Bounds(U, bid);
      break;
  }

  Vector3d tgk;

  switch (ibound)
  {
    case 0:
    case 2:
      U   = W;
      tgk = myK->D1V(U, V);
      break;
    case 1:
    case 3:
      V   = W;
      tgk = myK->D1U(U, V);
      break;
  }

  Vector3d        n    = myK->Bound(ibound)->Norm(W);
  Standard_Real scal = tgk.Dot(n);
  n.Multiply(-scal);
  tgk.Add(n);
  return tgk;
}

//=================================================================================================

Vector3d GeomFill_TgtOnCoons::D1(const Standard_Real W) const
{
  Standard_Real U = 0., V = 0., bid = 0.;
  switch (ibound)
  {
    case 0:
      myK->Bound(1)->Bounds(V, bid);
      break;
    case 1:
      myK->Bound(0)->Bounds(bid, U);
      break;
    case 2:
      myK->Bound(1)->Bounds(bid, V);
      break;
    case 3:
      myK->Bound(0)->Bounds(U, bid);
      break;
  }

  Vector3d tgsc, dtgsc;

  switch (ibound)
  {
    case 0:
    case 2:
      U    = W;
      tgsc = myK->D1V(U, V);
      break;
    case 1:
    case 3:
      V    = W;
      tgsc = myK->D1U(U, V);
      break;
  }
  dtgsc = myK->DUV(U, V);

  Vector3d n, dn;
  myK->Bound(ibound)->D1Norm(W, n, dn);

  Standard_Real scal  = tgsc.Dot(n);
  Vector3d        scaln = n.Multiplied(-scal);
  tgsc.Add(scaln);

  Vector3d scaldn = dn.Multiplied(-scal);

  Standard_Real scal2 = -dtgsc.Dot(n) - tgsc.Dot(dn);
  Vector3d        temp  = n.Multiplied(scal2);

  temp.Add(scaldn);
  Vector3d dtpur = dtgsc.Added(temp);

  return dtpur;
}

//=================================================================================================

void GeomFill_TgtOnCoons::D1(const Standard_Real W, Vector3d& T, Vector3d& DT) const
{
  Standard_Real U = 0., V = 0., bid = 0.;
  switch (ibound)
  {
    case 0:
      myK->Bound(1)->Bounds(V, bid);
      break;
    case 1:
      myK->Bound(0)->Bounds(bid, U);
      break;
    case 2:
      myK->Bound(1)->Bounds(bid, V);
      break;
    case 3:
      myK->Bound(0)->Bounds(U, bid);
      break;
  }

  Vector3d tgsc, dtgsc;

  switch (ibound)
  {
    case 0:
    case 2:
      U    = W;
      tgsc = myK->D1V(U, V);
      break;
    case 1:
    case 3:
      V    = W;
      tgsc = myK->D1U(U, V);
      break;
  }
  dtgsc = myK->DUV(U, V);

  Vector3d n, dn;
  myK->Bound(ibound)->D1Norm(W, n, dn);

  Standard_Real scal  = tgsc.Dot(n);
  Vector3d        scaln = n.Multiplied(-scal);
  T                   = tgsc.Added(scaln);

  Vector3d scaldn = dn.Multiplied(-scal);

  Standard_Real scal2 = -dtgsc.Dot(n) - tgsc.Dot(dn);
  Vector3d        temp  = n.Multiplied(scal2);

  temp.Add(scaldn);
  DT = dtgsc.Added(temp);
}

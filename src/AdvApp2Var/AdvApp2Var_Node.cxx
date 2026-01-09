// Created on: 1996-07-02
// Created by: Joelle CHAUVET
// Copyright (c) 1996-1999 Matra Datavision
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

#include <AdvApp2Var_Node.hxx>

IMPLEMENT_STANDARD_RTTIEXT(ApproximationNode, RefObject)

//=================================================================================================

ApproximationNode::ApproximationNode()
    : myTruePoints(0, 2, 0, 2),
      myErrors(0, 2, 0, 2),
      myOrdInU(2),
      myOrdInV(2)
{
  Point3d P0(0., 0., 0.);
  myTruePoints.Init(P0);
  myErrors.Init(0.);
}

//=================================================================================================

ApproximationNode::ApproximationNode(const Standard_Integer iu, const Standard_Integer iv)
    : myTruePoints(0, Max(0, iu), 0, Max(0, iv)),
      myErrors(0, Max(0, iu), 0, Max(0, iv)),
      myOrdInU(iu),
      myOrdInV(iv)
{
  Point3d P0(0., 0., 0.);
  myTruePoints.Init(P0);
  myErrors.Init(0.);
}

//=================================================================================================

ApproximationNode::ApproximationNode(const Coords2d&           UV,
                                 const Standard_Integer iu,
                                 const Standard_Integer iv)
    : myTruePoints(0, iu, 0, iv),
      myErrors(0, iu, 0, iv),
      myCoord(UV),
      myOrdInU(iu),
      myOrdInV(iv)
{
  Point3d P0(0., 0., 0.);
  myTruePoints.Init(P0);
  myErrors.Init(0.);
}

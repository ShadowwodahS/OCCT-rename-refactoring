// Created on: 1994-09-01
// Created by: Jacques GOUSSARD
// Copyright (c) 1994-1999 Matra Datavision
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

#include <Draft_VertexInfo.hxx>
#include <Standard_DomainError.hxx>
#include <TColStd_ListIteratorOfListOfReal.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>

//=================================================================================================

VertexInfo::VertexInfo() {}

//=================================================================================================

void VertexInfo::Add(const TopoEdge& E)
{
  for (myItEd.Initialize(myEdges); myItEd.More(); myItEd.Next())
  {
    if (E.IsSame(myItEd.Value()))
    {
      break;
    }
  }
  if (!myItEd.More())
  {
    myEdges.Append(E);
    myParams.Append(RealLast());
  }
}

//=================================================================================================

const Point3d& VertexInfo::Geometry() const
{
  return myGeom;
}

//=================================================================================================

Point3d& VertexInfo::ChangeGeometry()
{
  return myGeom;
}

//=================================================================================================

Standard_Real VertexInfo::Parameter(const TopoEdge& E)
{
  TColStd_ListIteratorOfListOfReal itp(myParams);
  myItEd.Initialize(myEdges);
  for (; myItEd.More(); myItEd.Next(), itp.Next())
  {
    if (myItEd.Value().IsSame(E))
    {
      return itp.Value();
    }
  }
  throw Standard_DomainError();
}

//=================================================================================================

Standard_Real& VertexInfo::ChangeParameter(const TopoEdge& E)
{
  TColStd_ListIteratorOfListOfReal itp(myParams);
  myItEd.Initialize(myEdges);
  for (; myItEd.More(); myItEd.Next(), itp.Next())
  {
    if (myItEd.Value().IsSame(E))
    {
      return itp.ChangeValue();
    }
  }
  throw Standard_DomainError();
}

//=================================================================================================

void VertexInfo::InitEdgeIterator()
{
  myItEd.Initialize(myEdges);
}

//=================================================================================================

const TopoEdge& VertexInfo::Edge() const
{
  return TopoDS::Edge(myItEd.Value());
}

//=================================================================================================

Standard_Boolean VertexInfo::MoreEdge() const
{
  return myItEd.More();
}

//=================================================================================================

void VertexInfo::NextEdge()
{
  myItEd.Next();
}

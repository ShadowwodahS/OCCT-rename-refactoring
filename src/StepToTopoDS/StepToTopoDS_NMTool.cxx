// Created on: 2010-11-15
// Created by: Sergey SLYADNEV
// Copyright (c) 2010-2014 OPEN CASCADE SAS
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

#include <StepToTopoDS_NMTool.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Shape.hxx>

// ============================================================================
// Method  : NamingTool2
// Purpose : Default constructor
// ============================================================================

NamingTool2::NamingTool2()
{
  myIDEASCase  = Standard_False;
  myActiveFlag = Standard_False;
}

// ============================================================================
// Method  : NamingTool2
// Purpose : Constructor with a Map for Representation Items and their names
// ============================================================================

NamingTool2::NamingTool2(const StepToTopoDS_DataMapOfRI&      MapOfRI,
                                         const StepToTopoDS_DataMapOfRINames& MapOfRINames)
{
  myIDEASCase  = Standard_False;
  myActiveFlag = Standard_False;
  Init(MapOfRI, MapOfRINames);
}

// ============================================================================
// Method  : Init
// Purpose : Initializes internal maps of the tool with the passed ones
// ============================================================================

void NamingTool2::Init(const StepToTopoDS_DataMapOfRI&      MapOfRI,
                               const StepToTopoDS_DataMapOfRINames& MapOfRINames)
{
  myRIMap      = MapOfRI;
  myRINamesMap = MapOfRINames;
}

// ============================================================================
// Method  : SetActive
// Purpose : Turns the tool ON/OFF (OFF by default)
// ============================================================================

void NamingTool2::SetActive(const Standard_Boolean isActive)
{
  myActiveFlag = isActive;
}

// ============================================================================
// Method  : IsActive
// Purpose : TRUE if active, FALSE - otherwise
// ============================================================================

Standard_Boolean NamingTool2::IsActive()
{
  return myActiveFlag;
}

// ============================================================================
// Method  : CleanUp
// Purpose : Clears all internal containers
// ============================================================================

void NamingTool2::CleanUp()
{
  myRIMap.Clear();
  myRINamesMap.Clear();
}

// ============================================================================
// Method  : IsBound
// Purpose : Indicates weither a RI is bound or not in the Map
// ============================================================================

Standard_Boolean NamingTool2::IsBound(const Handle(StepRepr_RepresentationItem)& RI)
{
  return myRIMap.IsBound(RI);
}

// ============================================================================
// Method  : IsBound
// Purpose : Indicates weither a RI is bound or not in the Map by name
// ============================================================================

Standard_Boolean NamingTool2::IsBound(const AsciiString1& RIName)
{
  return myRINamesMap.IsBound(RIName);
}

// ============================================================================
// Method  : Bind
// Purpose : Binds a RI with a Shape in the Map
// ============================================================================

void NamingTool2::Bind(const Handle(StepRepr_RepresentationItem)& RI, const TopoShape& S)
{
  myRIMap.Bind(RI, S);
}

// ============================================================================
// Method  : Bind
// Purpose : Binds a RI's name with a Shape in the Map
// ============================================================================

void NamingTool2::Bind(const AsciiString1& RIName, const TopoShape& S)
{
  myRINamesMap.Bind(RIName, S);
}

// ============================================================================
// Method  : Find
// Purpose : Returns the Shape corresponding to the bounded RI
// ============================================================================

const TopoShape& NamingTool2::Find(const Handle(StepRepr_RepresentationItem)& RI)
{
  return myRIMap.Find(RI);
}

// ============================================================================
// Method  : Find
// Purpose : Returns the Shape corresponding to the bounded RI's name
// ============================================================================

const TopoShape& NamingTool2::Find(const AsciiString1& RIName)
{
  return myRINamesMap.Find(RIName);
}

// ============================================================================
// Method  : RegisterNMEdge
// Purpose : Register non-manifold Edge in the internal storage if it wasn't
//           registered before
// ============================================================================

void NamingTool2::RegisterNMEdge(const TopoShape& Edge)
{
  if (!this->isEdgeRegisteredAsNM(Edge))
    myNMEdges.Append(Edge);
}

// ============================================================================
// Method  : IsSuspectedAsClosing
// Purpose : Checks whether SuspectedShell is pure non-manifold and adjacent
//           to BaseShell
// ============================================================================

Standard_Boolean NamingTool2::IsSuspectedAsClosing(const TopoShape& BaseShell,
                                                           const TopoShape& SuspectedShell)
{
  return this->IsPureNMShell(SuspectedShell) && this->isAdjacentShell(BaseShell, SuspectedShell);
}

// ============================================================================
// Method  : SetIDEASCase
// Purpose : Sets myIDEASCase flag (I-DEAS-like STP is processed)
// ============================================================================

void NamingTool2::SetIDEASCase(const Standard_Boolean IDEASCase)
{
  myIDEASCase = IDEASCase;
}

// ============================================================================
// Method  : GetIDEASCase
// Purpose : Gets myIDEASCase flag (I-DEAS-like STP is processed)
// ============================================================================

Standard_Boolean NamingTool2::IsIDEASCase()
{
  return myIDEASCase;
}

// ============================================================================
// Method  : IsPureNMShell
// Purpose : Checks if the Shell passed contains only non-manifold Edges
// ============================================================================

Standard_Boolean NamingTool2::IsPureNMShell(const TopoShape& Shell)
{
  Standard_Boolean result = Standard_True;
  ShapeExplorer  edgeExp(Shell, TopAbs_EDGE);
  for (; edgeExp.More(); edgeExp.Next())
  {
    const TopoShape& currentEdge = edgeExp.Current();
    if (!this->isEdgeRegisteredAsNM(currentEdge))
    {
      result = Standard_False;
      break;
    }
  }
  return result;
}

// ============================================================================
// Method  : isEdgeRegisteredAsNM
// Purpose : Checks if the Edge passed is registered as non-manifold one
// ============================================================================

Standard_Boolean NamingTool2::isEdgeRegisteredAsNM(const TopoShape& Edge)
{
  Standard_Boolean                   result = Standard_False;
  TopTools_ListIteratorOfListOfShape it(myNMEdges);
  for (; it.More(); it.Next())
  {
    TopoShape currentShape = it.Value();
    if (currentShape.IsSame(Edge))
    {
      result = Standard_True;
      break;
    }
  }
  return result;
}

// ============================================================================
// Method  : isAdjacentShell
// Purpose : Checks if the ShellA is adjacent to the ShellB
// ============================================================================

Standard_Boolean NamingTool2::isAdjacentShell(const TopoShape& ShellA,
                                                      const TopoShape& ShellB)
{
  if (ShellA.IsSame(ShellB))
    return Standard_False;

  ShapeExplorer edgeExpA(ShellA, TopAbs_EDGE);
  for (; edgeExpA.More(); edgeExpA.Next())
  {
    const TopoShape& currentEdgeA = edgeExpA.Current();
    ShapeExplorer     edgeExpB(ShellB, TopAbs_EDGE);
    for (; edgeExpB.More(); edgeExpB.Next())
    {
      const TopoShape& currentEdgeB = edgeExpB.Current();
      if (currentEdgeA.IsSame(currentEdgeB))
        return Standard_True;
    }
  }

  return Standard_False;
}

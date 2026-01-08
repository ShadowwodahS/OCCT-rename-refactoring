// Created on: 1994-04-21
// Created by: s:	Christophe GUYOT & Frederic UNTEREINER
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

#ifndef _IGESToBRep_TopoSurface_HeaderFile
#define _IGESToBRep_TopoSurface_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <IGESToBRep_CurveAndSurface.hxx>
class TopoShape;
class IGESData_IGESEntity;
class IGESGeom_RuledSurface;
class IGESGeom_SurfaceOfRevolution;
class IGESGeom_TabulatedCylinder;
class IGESGeom_OffsetSurface;
class IGESGeom_TrimmedSurface;
class IGESGeom_BoundedSurface;
class IGESGeom_Plane;
class IGESSolid_PlaneSurface;
class IGESBasic_SingleParent;
class gp_Pln;
class Transform3d;
class Transform2d;

//! Provides methods to transfer topologic surfaces entities
//! from IGES to CASCADE.
class IGESToBRep_TopoSurface : public IGESToBRep_CurveAndSurface
{
public:
  DEFINE_STANDARD_ALLOC

  //! Creates  a tool TopoSurface  ready  to  run, with
  //! epsilons  set  to  1.E-04,  TheModeTopo  to  True,  the
  //! optimization of  the continuity to False.
  Standard_EXPORT IGESToBRep_TopoSurface();

  //! Creates a tool TopoSurface ready to run and sets its
  //! fields as CS's.
  Standard_EXPORT IGESToBRep_TopoSurface(const IGESToBRep_CurveAndSurface& CS);

  //! Creates a tool TopoSurface ready to run.
  Standard_EXPORT IGESToBRep_TopoSurface(const Standard_Real    eps,
                                         const Standard_Real    epsGeom,
                                         const Standard_Real    epsCoeff,
                                         const Standard_Boolean mode,
                                         const Standard_Boolean modeapprox,
                                         const Standard_Boolean optimized);

  Standard_EXPORT TopoShape TransferTopoSurface(const Handle(IGESData_IGESEntity)& start);

  Standard_EXPORT TopoShape TransferTopoBasicSurface(const Handle(IGESData_IGESEntity)& start);

  Standard_EXPORT TopoShape TransferRuledSurface(const Handle(IGESGeom_RuledSurface)& start);

  Standard_EXPORT TopoShape
    TransferSurfaceOfRevolution(const Handle(IGESGeom_SurfaceOfRevolution)& start);

  Standard_EXPORT TopoShape
    TransferTabulatedCylinder(const Handle(IGESGeom_TabulatedCylinder)& start);

  Standard_EXPORT TopoShape TransferOffsetSurface(const Handle(IGESGeom_OffsetSurface)& start);

  Standard_EXPORT TopoShape TransferTrimmedSurface(const Handle(IGESGeom_TrimmedSurface)& start);

  Standard_EXPORT TopoShape TransferBoundedSurface(const Handle(IGESGeom_BoundedSurface)& start);

  Standard_EXPORT TopoShape TransferPlane(const Handle(IGESGeom_Plane)& start);

  Standard_EXPORT TopoShape TransferPlaneSurface(const Handle(IGESSolid_PlaneSurface)& start);

  Standard_EXPORT TopoShape TransferPerforate(const Handle(IGESBasic_SingleParent)& start);

  Standard_EXPORT TopoShape ParamSurface(const Handle(IGESData_IGESEntity)& start,
                                            Transform2d&                         trans,
                                            Standard_Real&                     uFact);

protected:
private:
  Standard_EXPORT TopoShape TransferPlaneParts(const Handle(IGESGeom_Plane)& start,
                                                  gp_Pln&                       gplan,
                                                  Transform3d&                      locat,
                                                  const Standard_Boolean        first);

  Standard_Real TheULength;
};

#endif // _IGESToBRep_TopoSurface_HeaderFile

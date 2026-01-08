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

#include <IGESData_DirChecker.hxx>
#include <IGESGeom_Boundary.hxx>
#include <IGESGeom_BoundedSurface.hxx>
#include <IGESGeom_BSplineCurve.hxx>
#include <IGESGeom_BSplineSurface.hxx>
#include <IGESGeom_CircularArc.hxx>
#include <IGESGeom_CompositeCurve.hxx>
#include <IGESGeom_ConicArc.hxx>
#include <IGESGeom_CopiousData.hxx>
#include <IGESGeom_Direction.hxx>
#include <IGESGeom_Flash.hxx>
#include <IGESGeom_GeneralModule.hxx>
#include <IGESGeom_Line.hxx>
#include <IGESGeom_OffsetCurve.hxx>
#include <IGESGeom_OffsetSurface.hxx>
#include <IGESGeom_Plane.hxx>
#include <IGESGeom_Point.hxx>
#include <IGESGeom_RuledSurface.hxx>
#include <IGESGeom_SplineCurve.hxx>
#include <IGESGeom_SplineSurface.hxx>
#include <IGESGeom_SurfaceOfRevolution.hxx>
#include <IGESGeom_TabulatedCylinder.hxx>
#include <IGESGeom_ToolBoundary.hxx>
#include <IGESGeom_ToolBoundedSurface.hxx>
#include <IGESGeom_ToolBSplineCurve.hxx>
#include <IGESGeom_ToolBSplineSurface.hxx>
#include <IGESGeom_ToolCircularArc.hxx>
#include <IGESGeom_ToolCompositeCurve.hxx>
#include <IGESGeom_ToolConicArc.hxx>
#include <IGESGeom_ToolCopiousData.hxx>
#include <IGESGeom_ToolCurveOnSurface.hxx>
#include <IGESGeom_ToolDirection.hxx>
#include <IGESGeom_ToolFlash.hxx>
#include <IGESGeom_ToolLine.hxx>
#include <IGESGeom_ToolOffsetCurve.hxx>
#include <IGESGeom_ToolOffsetSurface.hxx>
#include <IGESGeom_ToolPlane.hxx>
#include <IGESGeom_ToolPoint.hxx>
#include <IGESGeom_ToolRuledSurface.hxx>
#include <IGESGeom_ToolSplineCurve.hxx>
#include <IGESGeom_ToolSplineSurface.hxx>
#include <IGESGeom_ToolSurfaceOfRevolution.hxx>
#include <IGESGeom_ToolTabulatedCylinder.hxx>
#include <IGESGeom_ToolTransformationMatrix.hxx>
#include <IGESGeom_ToolTrimmedSurface.hxx>
#include <IGESGeom_TransformationMatrix.hxx>
#include <IGESGeom_TrimmedSurface.hxx>
#include <Interface_Category.hxx>
#include <Interface_Check.hxx>
#include <Interface_CopyTool.hxx>
#include <Interface_EntityIterator.hxx>
#include <Interface_Macros.hxx>
#include <Interface_ShareTool.hxx>
#include <Standard_Transient.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(IGESGeom_GeneralModule, IGESData_GeneralModule)

//  Each Module is attached to a Protocol : it must interpret Case Numbers1
//  (arguments <CN> of various methods) in accordance to values returned by
//  the method TypeNumber from this Protocol
IGESGeom_GeneralModule::IGESGeom_GeneralModule() {}

void IGESGeom_GeneralModule::OwnSharedCase(const Standard_Integer             CN,
                                           const Handle(IGESData_IGESEntity)& ent,
                                           Interface_EntityIterator&          iter) const
{
  switch (CN)
  {
    case 1: {
      DeclareAndCast(IGESGeom_BSplineCurve, anent, ent);
      if (anent.IsNull())
        return;
      BSplineCurveTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 2: {
      DeclareAndCast(IGESGeom_BSplineSurface, anent, ent);
      if (anent.IsNull())
        return;
      BSplineSurfaceTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 3: {
      DeclareAndCast(IGESGeom_Boundary, anent, ent);
      if (anent.IsNull())
        return;
      BoundaryTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 4: {
      DeclareAndCast(IGESGeom_BoundedSurface, anent, ent);
      if (anent.IsNull())
        return;
      BoundedSurfaceTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 5: {
      DeclareAndCast(IGESGeom_CircularArc, anent, ent);
      if (anent.IsNull())
        return;
      CircularArcTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 6: {
      DeclareAndCast(IGESGeom_CompositeCurve, anent, ent);
      if (anent.IsNull())
        return;
      CompositeCurveTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 7: {
      DeclareAndCast(IGESGeom_ConicArc, anent, ent);
      if (anent.IsNull())
        return;
      ConicArcTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 8: {
      DeclareAndCast(IGESGeom_CopiousData, anent, ent);
      if (anent.IsNull())
        return;
      CopiousDataTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 9: {
      DeclareAndCast(IGESGeom_CurveOnSurface, anent, ent);
      if (anent.IsNull())
        return;
      CurveOnSurfaceTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 10: {
      DeclareAndCast(IGESGeom_Direction, anent, ent);
      if (anent.IsNull())
        return;
      DirectionTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 11: {
      DeclareAndCast(IGESGeom_Flash, anent, ent);
      if (anent.IsNull())
        return;
      FlashTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 12: {
      DeclareAndCast(IGESGeom_Line, anent, ent);
      if (anent.IsNull())
        return;
      LineTool1 tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 13: {
      DeclareAndCast(IGESGeom_OffsetCurve, anent, ent);
      if (anent.IsNull())
        return;
      OffsetCurveTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 14: {
      DeclareAndCast(IGESGeom_OffsetSurface, anent, ent);
      if (anent.IsNull())
        return;
      OffsetSurfaceTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 15: {
      DeclareAndCast(IGESGeom_Plane, anent, ent);
      if (anent.IsNull())
        return;
      PlaneTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 16: {
      DeclareAndCast(IGESGeom_Point, anent, ent);
      if (anent.IsNull())
        return;
      PointTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 17: {
      DeclareAndCast(IGESGeom_RuledSurface, anent, ent);
      if (anent.IsNull())
        return;
      RuledSurfaceTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 18: {
      DeclareAndCast(IGESGeom_SplineCurve, anent, ent);
      if (anent.IsNull())
        return;
      SplineCurveTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 19: {
      DeclareAndCast(IGESGeom_SplineSurface, anent, ent);
      if (anent.IsNull())
        return;
      SplineSurfaceTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 20: {
      DeclareAndCast(IGESGeom_SurfaceOfRevolution, anent, ent);
      if (anent.IsNull())
        return;
      SurfaceOfRevolutionTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 21: {
      DeclareAndCast(IGESGeom_TabulatedCylinder, anent, ent);
      if (anent.IsNull())
        return;
      TabulatedCylinderTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 22: {
      DeclareAndCast(IGESGeom_TransformationMatrix, anent, ent);
      if (anent.IsNull())
        return;
      TransformationMatrixTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 23: {
      DeclareAndCast(IGESGeom_TrimmedSurface, anent, ent);
      if (anent.IsNull())
        return;
      TrimmedSurfaceTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    default:
      break;
  }
}

DirectoryChecker IGESGeom_GeneralModule::DirChecker(const Standard_Integer             CN,
                                                       const Handle(IGESData_IGESEntity)& ent) const
{
  switch (CN)
  {
    case 1: {
      DeclareAndCast(IGESGeom_BSplineCurve, anent, ent);
      if (anent.IsNull())
        break;
      BSplineCurveTool tool;
      return tool.DirChecker(anent);
    }
    case 2: {
      DeclareAndCast(IGESGeom_BSplineSurface, anent, ent);
      if (anent.IsNull())
        break;
      BSplineSurfaceTool tool;
      return tool.DirChecker(anent);
    }
    case 3: {
      DeclareAndCast(IGESGeom_Boundary, anent, ent);
      if (anent.IsNull())
        break;
      BoundaryTool tool;
      return tool.DirChecker(anent);
    }
    case 4: {
      DeclareAndCast(IGESGeom_BoundedSurface, anent, ent);
      if (anent.IsNull())
        break;
      BoundedSurfaceTool tool;
      return tool.DirChecker(anent);
    }
    case 5: {
      DeclareAndCast(IGESGeom_CircularArc, anent, ent);
      if (anent.IsNull())
        break;
      CircularArcTool tool;
      return tool.DirChecker(anent);
    }
    case 6: {
      DeclareAndCast(IGESGeom_CompositeCurve, anent, ent);
      if (anent.IsNull())
        break;
      CompositeCurveTool tool;
      return tool.DirChecker(anent);
    }
    case 7: {
      DeclareAndCast(IGESGeom_ConicArc, anent, ent);
      if (anent.IsNull())
        break;
      ConicArcTool tool;
      return tool.DirChecker(anent);
    }
    case 8: {
      DeclareAndCast(IGESGeom_CopiousData, anent, ent);
      if (anent.IsNull())
        break;
      CopiousDataTool tool;
      return tool.DirChecker(anent);
    }
    case 9: {
      DeclareAndCast(IGESGeom_CurveOnSurface, anent, ent);
      if (anent.IsNull())
        break;
      CurveOnSurfaceTool tool;
      return tool.DirChecker(anent);
    }
    case 10: {
      DeclareAndCast(IGESGeom_Direction, anent, ent);
      if (anent.IsNull())
        break;
      DirectionTool tool;
      return tool.DirChecker(anent);
    }
    case 11: {
      DeclareAndCast(IGESGeom_Flash, anent, ent);
      if (anent.IsNull())
        break;
      FlashTool tool;
      return tool.DirChecker(anent);
    }
    case 12: {
      DeclareAndCast(IGESGeom_Line, anent, ent);
      if (anent.IsNull())
        break;
      LineTool1 tool;
      return tool.DirChecker(anent);
    }
    case 13: {
      DeclareAndCast(IGESGeom_OffsetCurve, anent, ent);
      if (anent.IsNull())
        break;
      OffsetCurveTool tool;
      return tool.DirChecker(anent);
    }
    case 14: {
      DeclareAndCast(IGESGeom_OffsetSurface, anent, ent);
      if (anent.IsNull())
        break;
      OffsetSurfaceTool tool;
      return tool.DirChecker(anent);
    }
    case 15: {
      DeclareAndCast(IGESGeom_Plane, anent, ent);
      if (anent.IsNull())
        break;
      PlaneTool tool;
      return tool.DirChecker(anent);
    }
    case 16: {
      DeclareAndCast(IGESGeom_Point, anent, ent);
      if (anent.IsNull())
        break;
      PointTool tool;
      return tool.DirChecker(anent);
    }
    case 17: {
      DeclareAndCast(IGESGeom_RuledSurface, anent, ent);
      if (anent.IsNull())
        break;
      RuledSurfaceTool tool;
      return tool.DirChecker(anent);
    }
    case 18: {
      DeclareAndCast(IGESGeom_SplineCurve, anent, ent);
      if (anent.IsNull())
        break;
      SplineCurveTool tool;
      return tool.DirChecker(anent);
    }
    case 19: {
      DeclareAndCast(IGESGeom_SplineSurface, anent, ent);
      if (anent.IsNull())
        break;
      SplineSurfaceTool tool;
      return tool.DirChecker(anent);
    }
    case 20: {
      DeclareAndCast(IGESGeom_SurfaceOfRevolution, anent, ent);
      if (anent.IsNull())
        break;
      SurfaceOfRevolutionTool tool;
      return tool.DirChecker(anent);
    }
    case 21: {
      DeclareAndCast(IGESGeom_TabulatedCylinder, anent, ent);
      if (anent.IsNull())
        break;
      TabulatedCylinderTool tool;
      return tool.DirChecker(anent);
    }
    case 22: {
      DeclareAndCast(IGESGeom_TransformationMatrix, anent, ent);
      if (anent.IsNull())
        break;
      TransformationMatrixTool tool;
      return tool.DirChecker(anent);
    }
    case 23: {
      DeclareAndCast(IGESGeom_TrimmedSurface, anent, ent);
      if (anent.IsNull())
        break;
      TrimmedSurfaceTool tool;
      return tool.DirChecker(anent);
    }
    default:
      break;
  }
  return DirectoryChecker(); // by default, no specific criterium
}

void IGESGeom_GeneralModule::OwnCheckCase(const Standard_Integer             CN,
                                          const Handle(IGESData_IGESEntity)& ent,
                                          const Interface_ShareTool&         shares,
                                          Handle(Interface_Check)&           ach) const
{
  switch (CN)
  {
    case 1: {
      DeclareAndCast(IGESGeom_BSplineCurve, anent, ent);
      if (anent.IsNull())
        return;
      BSplineCurveTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 2: {
      DeclareAndCast(IGESGeom_BSplineSurface, anent, ent);
      if (anent.IsNull())
        return;
      BSplineSurfaceTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 3: {
      DeclareAndCast(IGESGeom_Boundary, anent, ent);
      if (anent.IsNull())
        return;
      BoundaryTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 4: {
      DeclareAndCast(IGESGeom_BoundedSurface, anent, ent);
      if (anent.IsNull())
        return;
      BoundedSurfaceTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 5: {
      DeclareAndCast(IGESGeom_CircularArc, anent, ent);
      if (anent.IsNull())
        return;
      CircularArcTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 6: {
      DeclareAndCast(IGESGeom_CompositeCurve, anent, ent);
      if (anent.IsNull())
        return;
      CompositeCurveTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 7: {
      DeclareAndCast(IGESGeom_ConicArc, anent, ent);
      if (anent.IsNull())
        return;
      ConicArcTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 8: {
      DeclareAndCast(IGESGeom_CopiousData, anent, ent);
      if (anent.IsNull())
        return;
      CopiousDataTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 9: {
      DeclareAndCast(IGESGeom_CurveOnSurface, anent, ent);
      if (anent.IsNull())
        return;
      CurveOnSurfaceTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 10: {
      DeclareAndCast(IGESGeom_Direction, anent, ent);
      if (anent.IsNull())
        return;
      DirectionTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 11: {
      DeclareAndCast(IGESGeom_Flash, anent, ent);
      if (anent.IsNull())
        return;
      FlashTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 12: {
      DeclareAndCast(IGESGeom_Line, anent, ent);
      if (anent.IsNull())
        return;
      LineTool1 tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 13: {
      DeclareAndCast(IGESGeom_OffsetCurve, anent, ent);
      if (anent.IsNull())
        return;
      OffsetCurveTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 14: {
      DeclareAndCast(IGESGeom_OffsetSurface, anent, ent);
      if (anent.IsNull())
        return;
      OffsetSurfaceTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 15: {
      DeclareAndCast(IGESGeom_Plane, anent, ent);
      if (anent.IsNull())
        return;
      PlaneTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 16: {
      DeclareAndCast(IGESGeom_Point, anent, ent);
      if (anent.IsNull())
        return;
      PointTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 17: {
      DeclareAndCast(IGESGeom_RuledSurface, anent, ent);
      if (anent.IsNull())
        return;
      RuledSurfaceTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 18: {
      DeclareAndCast(IGESGeom_SplineCurve, anent, ent);
      if (anent.IsNull())
        return;
      SplineCurveTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 19: {
      DeclareAndCast(IGESGeom_SplineSurface, anent, ent);
      if (anent.IsNull())
        return;
      SplineSurfaceTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 20: {
      DeclareAndCast(IGESGeom_SurfaceOfRevolution, anent, ent);
      if (anent.IsNull())
        return;
      SurfaceOfRevolutionTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 21: {
      DeclareAndCast(IGESGeom_TabulatedCylinder, anent, ent);
      if (anent.IsNull())
        return;
      TabulatedCylinderTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 22: {
      DeclareAndCast(IGESGeom_TransformationMatrix, anent, ent);
      if (anent.IsNull())
        return;
      TransformationMatrixTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 23: {
      DeclareAndCast(IGESGeom_TrimmedSurface, anent, ent);
      if (anent.IsNull())
        return;
      TrimmedSurfaceTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    default:
      break;
  }
}

Standard_Boolean IGESGeom_GeneralModule::NewVoid(const Standard_Integer      CN,
                                                 Handle(RefObject)& ent) const
{
  switch (CN)
  {
    case 1:
      ent = new IGESGeom_BSplineCurve;
      break;
    case 2:
      ent = new IGESGeom_BSplineSurface;
      break;
    case 3:
      ent = new IGESGeom_Boundary;
      break;
    case 4:
      ent = new IGESGeom_BoundedSurface;
      break;
    case 5:
      ent = new IGESGeom_CircularArc;
      break;
    case 6:
      ent = new IGESGeom_CompositeCurve;
      break;
    case 7:
      ent = new IGESGeom_ConicArc;
      break;
    case 8:
      ent = new IGESGeom_CopiousData;
      break;
    case 9:
      ent = new IGESGeom_CurveOnSurface;
      break;
    case 10:
      ent = new IGESGeom_Direction;
      break;
    case 11:
      ent = new IGESGeom_Flash;
      break;
    case 12:
      ent = new IGESGeom_Line;
      break;
    case 13:
      ent = new IGESGeom_OffsetCurve;
      break;
    case 14:
      ent = new IGESGeom_OffsetSurface;
      break;
    case 15:
      ent = new IGESGeom_Plane;
      break;
    case 16:
      ent = new IGESGeom_Point;
      break;
    case 17:
      ent = new IGESGeom_RuledSurface;
      break;
    case 18:
      ent = new IGESGeom_SplineCurve;
      break;
    case 19:
      ent = new IGESGeom_SplineSurface;
      break;
    case 20:
      ent = new IGESGeom_SurfaceOfRevolution;
      break;
    case 21:
      ent = new IGESGeom_TabulatedCylinder;
      break;
    case 22:
      ent = new IGESGeom_TransformationMatrix;
      break;
    case 23:
      ent = new IGESGeom_TrimmedSurface;
      break;
    default:
      return Standard_False; // by default, Failure on Recognize
  }
  return Standard_True;
}

void IGESGeom_GeneralModule::OwnCopyCase(const Standard_Integer             CN,
                                         const Handle(IGESData_IGESEntity)& entfrom,
                                         const Handle(IGESData_IGESEntity)& entto,
                                         Interface_CopyTool&                TC) const
{
  switch (CN)
  {
    case 1: {
      DeclareAndCast(IGESGeom_BSplineCurve, enfr, entfrom);
      DeclareAndCast(IGESGeom_BSplineCurve, ento, entto);
      BSplineCurveTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 2: {
      DeclareAndCast(IGESGeom_BSplineSurface, enfr, entfrom);
      DeclareAndCast(IGESGeom_BSplineSurface, ento, entto);
      BSplineSurfaceTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 3: {
      DeclareAndCast(IGESGeom_Boundary, enfr, entfrom);
      DeclareAndCast(IGESGeom_Boundary, ento, entto);
      BoundaryTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 4: {
      DeclareAndCast(IGESGeom_BoundedSurface, enfr, entfrom);
      DeclareAndCast(IGESGeom_BoundedSurface, ento, entto);
      BoundedSurfaceTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 5: {
      DeclareAndCast(IGESGeom_CircularArc, enfr, entfrom);
      DeclareAndCast(IGESGeom_CircularArc, ento, entto);
      CircularArcTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 6: {
      DeclareAndCast(IGESGeom_CompositeCurve, enfr, entfrom);
      DeclareAndCast(IGESGeom_CompositeCurve, ento, entto);
      CompositeCurveTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 7: {
      DeclareAndCast(IGESGeom_ConicArc, enfr, entfrom);
      DeclareAndCast(IGESGeom_ConicArc, ento, entto);
      ConicArcTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 8: {
      DeclareAndCast(IGESGeom_CopiousData, enfr, entfrom);
      DeclareAndCast(IGESGeom_CopiousData, ento, entto);
      CopiousDataTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 9: {
      DeclareAndCast(IGESGeom_CurveOnSurface, enfr, entfrom);
      DeclareAndCast(IGESGeom_CurveOnSurface, ento, entto);
      CurveOnSurfaceTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 10: {
      DeclareAndCast(IGESGeom_Direction, enfr, entfrom);
      DeclareAndCast(IGESGeom_Direction, ento, entto);
      DirectionTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 11: {
      DeclareAndCast(IGESGeom_Flash, enfr, entfrom);
      DeclareAndCast(IGESGeom_Flash, ento, entto);
      FlashTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 12: {
      DeclareAndCast(IGESGeom_Line, enfr, entfrom);
      DeclareAndCast(IGESGeom_Line, ento, entto);
      LineTool1 tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 13: {
      DeclareAndCast(IGESGeom_OffsetCurve, enfr, entfrom);
      DeclareAndCast(IGESGeom_OffsetCurve, ento, entto);
      OffsetCurveTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 14: {
      DeclareAndCast(IGESGeom_OffsetSurface, enfr, entfrom);
      DeclareAndCast(IGESGeom_OffsetSurface, ento, entto);
      OffsetSurfaceTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 15: {
      DeclareAndCast(IGESGeom_Plane, enfr, entfrom);
      DeclareAndCast(IGESGeom_Plane, ento, entto);
      PlaneTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 16: {
      DeclareAndCast(IGESGeom_Point, enfr, entfrom);
      DeclareAndCast(IGESGeom_Point, ento, entto);
      PointTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 17: {
      DeclareAndCast(IGESGeom_RuledSurface, enfr, entfrom);
      DeclareAndCast(IGESGeom_RuledSurface, ento, entto);
      RuledSurfaceTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 18: {
      DeclareAndCast(IGESGeom_SplineCurve, enfr, entfrom);
      DeclareAndCast(IGESGeom_SplineCurve, ento, entto);
      SplineCurveTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 19: {
      DeclareAndCast(IGESGeom_SplineSurface, enfr, entfrom);
      DeclareAndCast(IGESGeom_SplineSurface, ento, entto);
      SplineSurfaceTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 20: {
      DeclareAndCast(IGESGeom_SurfaceOfRevolution, enfr, entfrom);
      DeclareAndCast(IGESGeom_SurfaceOfRevolution, ento, entto);
      SurfaceOfRevolutionTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 21: {
      DeclareAndCast(IGESGeom_TabulatedCylinder, enfr, entfrom);
      DeclareAndCast(IGESGeom_TabulatedCylinder, ento, entto);
      TabulatedCylinderTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 22: {
      DeclareAndCast(IGESGeom_TransformationMatrix, enfr, entfrom);
      DeclareAndCast(IGESGeom_TransformationMatrix, ento, entto);
      TransformationMatrixTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 23: {
      DeclareAndCast(IGESGeom_TrimmedSurface, enfr, entfrom);
      DeclareAndCast(IGESGeom_TrimmedSurface, ento, entto);
      TrimmedSurfaceTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    default:
      break;
  }
}

Standard_Integer IGESGeom_GeneralModule::CategoryNumber(const Standard_Integer            CN,
                                                        const Handle(RefObject)& ent,
                                                        const Interface_ShareTool&) const
{
  if (CN == 11)
    return Interface_Category::Number("Drawing");
  if (CN == 15)
  {
    DeclareAndCast(IGESGeom_Plane, anent, ent);
    if (anent->HasSymbolAttach())
      return Interface_Category::Number("Drawing");
  }
  if (CN == 16)
  {
    DeclareAndCast(IGESGeom_Point, anent, ent);
    if (anent->HasDisplaySymbol())
      return Interface_Category::Number("Drawing");
  }
  if (CN == 22)
    return Interface_Category::Number("Auxiliary");
  return Interface_Category::Number("Shape");
}

// Created on: 1992-03-30
// Created by: Laurent BUCHARD
// Copyright (c) 1992-1999 Matra Datavision
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

#include <ElCLib.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Vec2d.hxx>
#include <IntCurve_PConic.hxx>
#include <IntCurve_PConicTool.hxx>

//----------------------------------------------------------------------
gp_Pnt2d ParametricConicTool::Value(const ParametricConic& PConic, const Standard_Real X)
{
  switch (PConic.TypeCurve())
  {

    case GeomAbs_Line:
      return (ElCLib1::LineValue(X, PConic.Axis2().XAxis()));

    case GeomAbs_Circle:
      return (ElCLib1::CircleValue(X, PConic.Axis2(), PConic.Param1()));

    case GeomAbs_Ellipse:
      return (ElCLib1::EllipseValue(X, PConic.Axis2(), PConic.Param1(), PConic.Param2()));

    case GeomAbs_Parabola:
      return (ElCLib1::ParabolaValue(X, PConic.Axis2(), PConic.Param1()));

    default: //-- case GeomAbs_Hyperbola:
      return (ElCLib1::HyperbolaValue(X, PConic.Axis2(), PConic.Param1(), PConic.Param2()));
  }
}

//----------------------------------------------------------------------
void ParametricConicTool::D1(const ParametricConic& PConic,
                             const Standard_Real    U,
                             gp_Pnt2d&              Pt,
                             gp_Vec2d&              Tan)
{

  switch (PConic.TypeCurve())
  {

    case GeomAbs_Line:
      ElCLib1::LineD1(U, PConic.Axis2().XAxis(), Pt, Tan);
      break;

    case GeomAbs_Circle:
      ElCLib1::CircleD1(U, PConic.Axis2(), PConic.Param1(), Pt, Tan);
      break;

    case GeomAbs_Ellipse:
      ElCLib1::EllipseD1(U, PConic.Axis2(), PConic.Param1(), PConic.Param2(), Pt, Tan);
      break;

    case GeomAbs_Parabola:
      ElCLib1::ParabolaD1(U, PConic.Axis2(), PConic.Param1(), Pt, Tan);
      break;

    case GeomAbs_Hyperbola:
      ElCLib1::HyperbolaD1(U, PConic.Axis2(), PConic.Param1(), PConic.Param2(), Pt, Tan);
      break;
    default:
      break;
  }
}

//----------------------------------------------------------------------
void ParametricConicTool::D2(const ParametricConic& PConic,
                             const Standard_Real    U,
                             gp_Pnt2d&              Pt,
                             gp_Vec2d&              Tan,
                             gp_Vec2d&              Norm)
{

  switch (PConic.TypeCurve())
  {

    case GeomAbs_Line:
      Tan.SetCoord(0.0, 0.0);
      ElCLib1::LineD1(U, PConic.Axis2().XAxis(), Pt, Tan);
      break;

    case GeomAbs_Circle:
      ElCLib1::CircleD2(U, PConic.Axis2(), PConic.Param1(), Pt, Tan, Norm);
      break;

    case GeomAbs_Ellipse:
      ElCLib1::EllipseD2(U, PConic.Axis2(), PConic.Param1(), PConic.Param2(), Pt, Tan, Norm);
      break;

    case GeomAbs_Parabola:
      ElCLib1::ParabolaD2(U, PConic.Axis2(), PConic.Param1(), Pt, Tan, Norm);
      break;

    case GeomAbs_Hyperbola:
      ElCLib1::HyperbolaD2(U, PConic.Axis2(), PConic.Param1(), PConic.Param2(), Pt, Tan, Norm);
      break;
    default:
      break;
  }
}

Standard_Real ParametricConicTool::EpsX(const ParametricConic& PConic)
{
  return (PConic.EpsX());
}

Standard_Integer ParametricConicTool::NbSamples(const ParametricConic& PConic)
{
  return (PConic.Accuracy());
}

// Standard_Integer ParametricConicTool::NbSamples(const ParametricConic& PConic,const Standard_Real
// U0,const Standard_Real U1) {
Standard_Integer ParametricConicTool::NbSamples(const ParametricConic& PConic,
                                                const Standard_Real,
                                                const Standard_Real)
{
  return (PConic.Accuracy());
}

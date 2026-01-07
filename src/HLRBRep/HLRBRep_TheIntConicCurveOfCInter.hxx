// Created on: 1992-10-14
// Created by: Christophe MARION
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

#ifndef _HLRBRep_TheIntConicCurveOfCInter_HeaderFile
#define _HLRBRep_TheIntConicCurveOfCInter_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>

#include <IntRes2d_Intersection.hxx>
class ConicTool;
class CurveTool5;
class ProjPCurOfCInter;
class HLRBRep_TheIntersectorOfTheIntConicCurveOfCInter;
class HLRBRep_MyImpParToolOfTheIntersectorOfTheIntConicCurveOfCInter;
class gp_Lin2d;
class Domain2;
class gp_Circ2d;
class gp_Elips2d;
class gp_Parab2d;
class gp_Hypr2d;

class HLRBRep_TheIntConicCurveOfCInter : public Intersection2
{
public:
  DEFINE_STANDARD_ALLOC

  //! Empty constructor.
  HLRBRep_TheIntConicCurveOfCInter();

  //! Intersection between a line and a parametric curve.
  HLRBRep_TheIntConicCurveOfCInter(const gp_Lin2d&         L,
                                   const Domain2&  D1,
                                   const Standard_Address& PCurve,
                                   const Domain2&  D2,
                                   const Standard_Real     TolConf,
                                   const Standard_Real     Tol);

  //! Intersection between a line and a parametric curve.
  Standard_EXPORT HLRBRep_TheIntConicCurveOfCInter(const gp_Circ2d&        C,
                                                   const Domain2&  D1,
                                                   const Standard_Address& PCurve,
                                                   const Domain2&  D2,
                                                   const Standard_Real     TolConf,
                                                   const Standard_Real     Tol);

  //! Intersection between an ellipse and a parametric curve.
  Standard_EXPORT HLRBRep_TheIntConicCurveOfCInter(const gp_Elips2d&       E,
                                                   const Domain2&  D1,
                                                   const Standard_Address& PCurve,
                                                   const Domain2&  D2,
                                                   const Standard_Real     TolConf,
                                                   const Standard_Real     Tol);

  //! Intersection between a parabola and a parametric curve.
  Standard_EXPORT HLRBRep_TheIntConicCurveOfCInter(const gp_Parab2d&       Prb,
                                                   const Domain2&  D1,
                                                   const Standard_Address& PCurve,
                                                   const Domain2&  D2,
                                                   const Standard_Real     TolConf,
                                                   const Standard_Real     Tol);

  //! Intersection between the main branch of an hyperbola
  //! and a parametric curve.
  Standard_EXPORT HLRBRep_TheIntConicCurveOfCInter(const gp_Hypr2d&        H,
                                                   const Domain2&  D1,
                                                   const Standard_Address& PCurve,
                                                   const Domain2&  D2,
                                                   const Standard_Real     TolConf,
                                                   const Standard_Real     Tol);

  //! Intersection between a line and a parametric curve.
  void Perform(const gp_Lin2d&         L,
               const Domain2&  D1,
               const Standard_Address& PCurve,
               const Domain2&  D2,
               const Standard_Real     TolConf,
               const Standard_Real     Tol);

  //! Intersection between a line and a parametric curve.
  void Perform(const gp_Circ2d&        C,
               const Domain2&  D1,
               const Standard_Address& PCurve,
               const Domain2&  D2,
               const Standard_Real     TolConf,
               const Standard_Real     Tol);

  //! Intersection between an ellipse and a parametric curve.
  void Perform(const gp_Elips2d&       E,
               const Domain2&  D1,
               const Standard_Address& PCurve,
               const Domain2&  D2,
               const Standard_Real     TolConf,
               const Standard_Real     Tol);

  //! Intersection between a parabola and a parametric curve.
  void Perform(const gp_Parab2d&       Prb,
               const Domain2&  D1,
               const Standard_Address& PCurve,
               const Domain2&  D2,
               const Standard_Real     TolConf,
               const Standard_Real     Tol);

  //! Intersection between the main branch of an hyperbola
  //! and a parametric curve.
  void Perform(const gp_Hypr2d&        H,
               const Domain2&  D1,
               const Standard_Address& PCurve,
               const Domain2&  D2,
               const Standard_Real     TolConf,
               const Standard_Real     Tol);

protected:
private:
  //! Intersection between a conic fom gp
  //! and a parametric curve.
  void Perform(const ConicTool& ICurve,
               const Domain2&     D1,
               const Standard_Address&    PCurve,
               const Domain2&     D2,
               const Standard_Real        TolConf,
               const Standard_Real        Tol);
};

#define TheImpTool ConicTool
#define TheImpTool_hxx <IntCurve_IConicTool.hxx>
#define ThePCurve Standard_Address
#define ThePCurve_hxx <Standard_Address.hxx>
#define ThePCurveTool CurveTool5
#define ThePCurveTool_hxx <HLRBRep_CurveTool.hxx>
#define TheProjPCur ProjPCurOfCInter
#define TheProjPCur_hxx <HLRBRep_TheProjPCurOfCInter.hxx>
#define IntCurve_TheIntersector HLRBRep_TheIntersectorOfTheIntConicCurveOfCInter
#define IntCurve_TheIntersector_hxx <HLRBRep_TheIntersectorOfTheIntConicCurveOfCInter.hxx>
#define IntCurve_MyImpParToolOfTheIntersector                                                      \
  HLRBRep_MyImpParToolOfTheIntersectorOfTheIntConicCurveOfCInter
#define IntCurve_MyImpParToolOfTheIntersector_hxx                                                  \
  <HLRBRep_MyImpParToolOfTheIntersectorOfTheIntConicCurveOfCInter.hxx>
#define IntCurve_MyImpParToolOfTheIntersector                                                      \
  HLRBRep_MyImpParToolOfTheIntersectorOfTheIntConicCurveOfCInter
#define IntCurve_MyImpParToolOfTheIntersector_hxx                                                  \
  <HLRBRep_MyImpParToolOfTheIntersectorOfTheIntConicCurveOfCInter.hxx>
#define IntCurve_IntConicCurveGen HLRBRep_TheIntConicCurveOfCInter
#define IntCurve_IntConicCurveGen_hxx <HLRBRep_TheIntConicCurveOfCInter.hxx>

#include <IntCurve_IntConicCurveGen.lxx>

#undef TheImpTool
#undef TheImpTool_hxx
#undef ThePCurve
#undef ThePCurve_hxx
#undef ThePCurveTool
#undef ThePCurveTool_hxx
#undef TheProjPCur
#undef TheProjPCur_hxx
#undef IntCurve_TheIntersector
#undef IntCurve_TheIntersector_hxx
#undef IntCurve_MyImpParToolOfTheIntersector
#undef IntCurve_MyImpParToolOfTheIntersector_hxx
#undef IntCurve_MyImpParToolOfTheIntersector
#undef IntCurve_MyImpParToolOfTheIntersector_hxx
#undef IntCurve_IntConicCurveGen
#undef IntCurve_IntConicCurveGen_hxx

#endif // _HLRBRep_TheIntConicCurveOfCInter_HeaderFile

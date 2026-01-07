// Created on: 1994-08-03
// Created by: Christophe MARION
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

#ifndef _HLRTopoBRep_OutLiner_HeaderFile
#define _HLRTopoBRep_OutLiner_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <TopoDS_Shape.hxx>
#include <HLRTopoBRep_Data.hxx>
#include <Standard_Transient.hxx>
#include <BRepTopAdaptor_MapOfShapeTool.hxx>
#include <Standard_Integer.hxx>
class HLRAlgo_Projector;
class TopoFace;

class HLRTopoBRep_OutLiner;
DEFINE_STANDARD_HANDLE(HLRTopoBRep_OutLiner, RefObject)

class HLRTopoBRep_OutLiner : public RefObject
{

public:
  Standard_EXPORT HLRTopoBRep_OutLiner();

  Standard_EXPORT HLRTopoBRep_OutLiner(const TopoShape& OriSh);

  Standard_EXPORT HLRTopoBRep_OutLiner(const TopoShape& OriS, const TopoShape& OutS);

  void OriginalShape(const TopoShape& OriS);

  TopoShape& OriginalShape();

  void OutLinedShape(const TopoShape& OutS);

  TopoShape& OutLinedShape();

  HLRTopoBRep_Data& DataStructure();

  Standard_EXPORT void Fill(const HLRAlgo_Projector&       P,
                            BRepTopAdaptor_MapOfShapeTool& MST,
                            const Standard_Integer         nbIso);

  DEFINE_STANDARD_RTTIEXT(HLRTopoBRep_OutLiner, RefObject)

protected:
private:
  //! Builds faces from F and add them to S.
  Standard_EXPORT void ProcessFace(const TopoFace&             F,
                                   TopoShape&                  S,
                                   BRepTopAdaptor_MapOfShapeTool& M);

  Standard_EXPORT void BuildShape(BRepTopAdaptor_MapOfShapeTool& M);

  TopoShape     myOriginalShape;
  TopoShape     myOutLinedShape;
  HLRTopoBRep_Data myDS;
};

#include <HLRTopoBRep_OutLiner.lxx>

#endif // _HLRTopoBRep_OutLiner_HeaderFile

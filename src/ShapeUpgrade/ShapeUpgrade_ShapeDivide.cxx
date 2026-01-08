// Created on: 1999-04-27
// Created by: Pavel DURANDIN
// Copyright (c) 1999-1999 Matra Datavision
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

//    abv 16.06.99 returning ReShape context; processing shared subshapes in compounds
//    sln 29.11.01 Bug21: in method Perform(..) nullify location of compound's faces only if mode
//    myConsiderLocation is on

#include <BRep_Builder.hxx>
#include <Message_Msg.hxx>
#include <Precision.hxx>
#include <ShapeBuild_ReShape.hxx>
#include <ShapeExtend.hxx>
#include <ShapeExtend_BasicMsgRegistrator.hxx>
#include <ShapeUpgrade_FaceDivide.hxx>
#include <ShapeUpgrade_ShapeDivide.hxx>
#include <ShapeUpgrade_WireDivide.hxx>
#include <Standard_ErrorHandler.hxx>
#include <Standard_Failure.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>

//=================================================================================================

ShapeUpgrade_ShapeDivide::ShapeUpgrade_ShapeDivide()
    : myStatus(0)
{
  myPrecision = myMinTol = Precision::Confusion();
  myMaxTol               = 1; // Precision::Infinite() ?? pdn
  mySplitFaceTool        = new ShapeUpgrade_FaceDivide;
  myContext              = new ShapeBuild_ReShape;
  // myMsgReg = new ShapeExtend_BasicMsgRegistrator;
  mySegmentMode = Standard_True;
  myEdgeMode    = 2;
}

//=================================================================================================

ShapeUpgrade_ShapeDivide::ShapeUpgrade_ShapeDivide(const TopoShape& S)
    : myStatus(0)
{
  myPrecision = myMinTol = Precision::Confusion();
  myMaxTol               = 1; // Precision::Infinite() ?? pdn
  mySplitFaceTool        = new ShapeUpgrade_FaceDivide;
  myContext              = new ShapeBuild_ReShape;
  mySegmentMode          = Standard_True;
  myEdgeMode             = 2;
  Init(S);
}

//=================================================================================================

void ShapeUpgrade_ShapeDivide::Init(const TopoShape& S)
{
  myShape = S;
}

//=================================================================================================

ShapeUpgrade_ShapeDivide::~ShapeUpgrade_ShapeDivide() {}

//=================================================================================================

void ShapeUpgrade_ShapeDivide::SetPrecision(const Standard_Real Prec)
{
  myPrecision = Prec;
}

//=================================================================================================

void ShapeUpgrade_ShapeDivide::SetMaxTolerance(const Standard_Real maxtol)
{
  myMaxTol = maxtol;
}

//=================================================================================================

void ShapeUpgrade_ShapeDivide::SetMinTolerance(const Standard_Real mintol)
{
  myMinTol = mintol;
}

//=================================================================================================

void ShapeUpgrade_ShapeDivide::SetSurfaceSegmentMode(const Standard_Boolean Segment1)
{
  mySegmentMode = Segment1;
}

//=================================================================================================

Standard_Boolean ShapeUpgrade_ShapeDivide::Perform(const Standard_Boolean newContext)
{
  myStatus = ShapeExtend1::EncodeStatus(ShapeExtend_OK);
  if (myShape.IsNull())
  {
    myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL1);
    return Standard_False;
  }

  if (newContext || myContext.IsNull())
    myContext = new ShapeBuild_ReShape;

  // Process COMPOUNDs separately in order to handle sharing in assemblies
  // NOTE: not optimized: subshape can be processed twice (second time - no modif)
  if (myShape.ShapeType() == TopAbs_COMPOUND)
  {
    Standard_Integer locStatus = myStatus;
    TopoCompound  C;
    ShapeBuilder     B;
    B.MakeCompound(C);
    TopoShape savShape = myShape;
    for (TopoDS_Iterator it(savShape, Standard_False); it.More(); it.Next())
    {
      TopoShape    shape = it.Value();
      TopLoc_Location L     = shape.Location();
      if (myContext->ModeConsiderLocation())
      {
        TopLoc_Location nullLoc;
        shape.Location(nullLoc);
      }
      myShape = myContext->Apply(shape);
      Perform(Standard_False);
      if (myContext->ModeConsiderLocation())
        myResult.Location(L);
      myResult.Orientation(TopAbs1::Compose(myResult.Orientation(), savShape.Orientation()));
      B.Add(C, myResult);
      locStatus |= myStatus;
    }

    myShape  = savShape;
    myStatus = locStatus;
    if (Status(ShapeExtend_DONE))
    {
      myResult = myContext->Apply(C, TopAbs_SHAPE);
      myContext->Replace(myShape, myResult);
      return Standard_True;
    }
    myResult = myShape;
    return Standard_False;
  }

  // Process FACEs
  Handle(ShapeUpgrade_FaceDivide) SplitFace = GetSplitFaceTool();
  if (!SplitFace.IsNull())
  {
    SplitFace->SetPrecision(myPrecision);
    SplitFace->SetMaxTolerance(myMaxTol);
    SplitFace->SetSurfaceSegmentMode(mySegmentMode);
    Handle(ShapeUpgrade_WireDivide) SplitWire = SplitFace->GetWireDivideTool();
    if (!SplitWire.IsNull())
    {
      SplitWire->SetMinTolerance(myMinTol);
      SplitWire->SetEdgeMode(myEdgeMode);
    }
    Message_Msg doneMsg = GetFaceMsg();

    for (ShapeExplorer exp(myShape, TopAbs_FACE); exp.More(); exp.Next())
    {
      TopoShape tmpF = exp.Current().Oriented(TopAbs_FORWARD);
      TopoFace  F    = TopoDS::Face(tmpF); // protection against INTERNAL shapes: cts20105a.rle
      TopoShape sh   = myContext->Apply(F, TopAbs_SHAPE);
      for (ShapeExplorer exp2(sh, TopAbs_FACE); exp2.More(); exp2.Next())
      {
        try
        {
          OCC_CATCH_SIGNALS
          for (; exp2.More(); exp2.Next())
          {
            TopoFace face = TopoDS::Face(exp2.Current());
            SplitFace->Init(face);
            SplitFace->SetContext(myContext);
            SplitFace->Perform();
            if (SplitFace->Status(ShapeExtend_FAIL))
            {
              myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL2);
            }
            if (SplitFace->Status(ShapeExtend_DONE))
            {
              myContext->Replace(face, SplitFace->Result());
              SendMsg(face, doneMsg, Message_Info);
              if (SplitFace->Status(ShapeExtend_DONE1))
              {
                myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE1);
              }
              if (SplitFace->Status(ShapeExtend_DONE2))
              {
                myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE2);
              }
            }
          }
        }
        catch (ExceptionBase const& anException)
        {
#ifdef OCCT_DEBUG
          std::cout << "\nError: Exception in ShapeUpgrade_FaceDivide::Perform(): ";
          anException.Print(std::cout);
          std::cout << std::endl;
#endif
          (void)anException;
          myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL2);
        }

        if (!exp2.More())
        {
          break;
        }
      }
    }
  }

  // Process free WIREs
  Handle(ShapeUpgrade_WireDivide) SplitWire = SplitFace->GetWireDivideTool();
  if (!SplitWire.IsNull())
  {
    SplitWire->SetFace(TopoFace());
    SplitWire->SetPrecision(myPrecision);
    SplitWire->SetMaxTolerance(myMaxTol);
    SplitWire->SetMinTolerance(myMinTol);
    SplitWire->SetEdgeMode(myEdgeMode);
    Message_Msg doneMsg = GetWireMsg();

    ShapeExplorer exp; // svv Jan 10 2000 : porting on DEC
    for (exp.Init(myShape, TopAbs_WIRE, TopAbs_FACE); exp.More(); exp.Next())
    {
      // smh#8
      TopoShape tmpW = exp.Current().Oriented(TopAbs_FORWARD);
      TopoWire  W    = TopoDS::Wire(tmpW); // protection against INTERNAL shapes
      TopoShape sh   = myContext->Apply(W, TopAbs_SHAPE);
      for (ShapeExplorer exp2(sh, TopAbs_WIRE); exp2.More(); exp2.Next())
      {
        TopoWire wire = TopoDS::Wire(exp2.Current());
        SplitWire->Load(wire);
        SplitWire->SetContext(myContext);
        SplitWire->Perform();
        if (SplitWire->Status(ShapeExtend_FAIL))
        {
          myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL3);
        }
        if (SplitWire->Status(ShapeExtend_DONE))
        {
          myContext->Replace(wire, SplitWire->Wire());
          SendMsg(wire, doneMsg, Message_Info);
          myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE1);
        }
      }
    }

    // Process free EDGEs
    Message_Msg edgeDoneMsg = GetEdgeMsg();
    for (exp.Init(myShape, TopAbs_EDGE, TopAbs_WIRE); exp.More(); exp.Next())
    {
      // smh#8
      TopoShape tmpE = exp.Current().Oriented(TopAbs_FORWARD);
      // clang-format off
      TopoEdge E = TopoDS::Edge (tmpE );                                       // protection against INTERNAL shapes
      // clang-format on
      TopoVertex V1, V2;
      TopExp1::Vertices(E, V2, V1);
      if (V1.IsNull() && V2.IsNull())
        continue; // skl 27.10.2004 for OCC5624
      TopoShape sh = myContext->Apply(E, TopAbs_SHAPE);
      for (ShapeExplorer exp2(sh, TopAbs_EDGE); exp2.More(); exp2.Next())
      {
        TopoEdge edge = TopoDS::Edge(exp2.Current());
        SplitWire->Load(edge);
        SplitWire->SetContext(myContext);
        SplitWire->Perform();
        if (SplitWire->Status(ShapeExtend_FAIL))
        {
          myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL3);
        }
        if (SplitWire->Status(ShapeExtend_DONE))
        {
          myContext->Replace(edge, SplitWire->Wire());
          SendMsg(edge, edgeDoneMsg, Message_Info);
          myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE1);
        }
      }
    }
  }
  myResult = myContext->Apply(myShape, TopAbs_SHAPE);
  return !myResult.IsSame(myShape);
}

//=================================================================================================

TopoShape ShapeUpgrade_ShapeDivide::Result() const
{
  return myResult;
}

//=================================================================================================

Handle(ShapeBuild_ReShape) ShapeUpgrade_ShapeDivide::GetContext() const
{
  // if ( myContext.IsNull() ) myContext = new ShapeBuild_ReShape;
  return myContext;
}

//=================================================================================================

void ShapeUpgrade_ShapeDivide::SetContext(const Handle(ShapeBuild_ReShape)& context)
{
  myContext = context;
}

//=================================================================================================

void ShapeUpgrade_ShapeDivide::SetSplitFaceTool(
  const Handle(ShapeUpgrade_FaceDivide)& splitFaceTool)
{
  mySplitFaceTool = splitFaceTool;
}

//=================================================================================================

Handle(ShapeUpgrade_FaceDivide) ShapeUpgrade_ShapeDivide::GetSplitFaceTool() const
{
  return mySplitFaceTool;
}

//=================================================================================================

Standard_Boolean ShapeUpgrade_ShapeDivide::Status(const ShapeExtend_Status status) const
{
  return ShapeExtend1::DecodeStatus(myStatus, status);
}

//=================================================================================================

void ShapeUpgrade_ShapeDivide::SetEdgeMode(const Standard_Integer aEdgeMode)
{
  myEdgeMode = aEdgeMode;
}

//=================================================================================================

void ShapeUpgrade_ShapeDivide::SetMsgRegistrator(
  const Handle(ShapeExtend_BasicMsgRegistrator)& msgreg)
{
  myMsgReg = msgreg;
}

//=================================================================================================

Handle(ShapeExtend_BasicMsgRegistrator) ShapeUpgrade_ShapeDivide::MsgRegistrator() const
{
  return myMsgReg;
}

//=================================================================================================

void ShapeUpgrade_ShapeDivide::SendMsg(const TopoShape&   shape,
                                       const Message_Msg&    message,
                                       const Message_Gravity gravity) const
{
  if (!myMsgReg.IsNull())
    myMsgReg->Send(shape, message, gravity);
}

Message_Msg ShapeUpgrade_ShapeDivide::GetFaceMsg() const
{
  return "ShapeDivide.FaceDivide.MSG0";
}

Message_Msg ShapeUpgrade_ShapeDivide::GetWireMsg() const
{
  return "ShapeDivide.WireDivide.MSG0";
}

Message_Msg ShapeUpgrade_ShapeDivide::GetEdgeMsg() const
{
  return "ShapeDivide.EdgeDivide.MSG0";
}

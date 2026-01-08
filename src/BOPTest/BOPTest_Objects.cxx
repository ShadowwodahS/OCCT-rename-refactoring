// Created by: Peter KURNEV
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

#include <BOPAlgo_BOP.hxx>
#include <BOPAlgo_Builder.hxx>
#include <BOPAlgo_PaveFiller.hxx>
#include <BOPAlgo_Section.hxx>
#include <BOPTest_Objects.hxx>
#include <BOPAlgo_CellsBuilder.hxx>
#include <BOPAlgo_Splitter.hxx>
#include <NCollection_BaseAllocator.hxx>
#include <Precision.hxx>

static Handle(NCollection_BaseAllocator)& Allocator1();

//=================================================================================================

class BOPTest_Session
{
public:
  //
  BOPTest_Session() { Init(); };

  //
  ~BOPTest_Session() { Clear(); };

  //
  // Init
  void Init()
  {
    Handle(NCollection_BaseAllocator) pA1, pA2;
    //
    pA1 = NCollection_BaseAllocator::CommonBaseAllocator();
    pA2 = NCollection_BaseAllocator::CommonBaseAllocator();
    //
    myPaveFiller     = new BooleanPaveFiller(pA1);
    myBuilderDefault = new BOPAlgo_Builder(pA2);
    //
    myBuilder = myBuilderDefault;
    SetDefaultOptions();
  };

  //
  // Clear
  void Clear()
  {
    if (myPaveFiller)
    {
      delete myPaveFiller;
      myPaveFiller = NULL;
    }
    //
    if (myBuilderDefault)
    {
      delete myBuilderDefault;
      myBuilderDefault = NULL;
    }
  };

  //
  // IsValid
  Standard_Boolean IsValid() const { return (myPaveFiller != NULL); }

  // PaveFiller
  BooleanPaveFiller& PaveFiller() { return *myPaveFiller; };

  //
  // Builder
  BOPAlgo_Builder& Builder() { return *myBuilder; };

  //
  // SetBuilder
  void SetBuilder(BOPAlgo_Builder* pBuilder) { myBuilder = pBuilder; };

  //
  // SetBuilderDef
  void SetBuilderDefault() { myBuilder = myBuilderDefault; };

  //
  ShapeList& Shapes() { return myShapes; }

  //
  ShapeList& Tools() { return myTools; }

  // Resets all options to default values
  void SetDefaultOptions()
  {
    myRunParallel    = Standard_False;
    myNonDestructive = Standard_False;
    myFuzzyValue     = Precision1::Confusion();
    myGlue           = BOPAlgo_GlueOff;
    myDrawWarnShapes = Standard_False;
    myCheckInverted  = Standard_True;
    myUseOBB         = Standard_False;
    myUnifyEdges     = Standard_False;
    myUnifyFaces     = Standard_False;
    myAngTol         = Precision1::Angular();
  }

  //
  void SetRunParallel(const Standard_Boolean bFlag) { myRunParallel = bFlag; };

  //
  Standard_Boolean RunParallel() const { return myRunParallel; };

  //
  void SetFuzzyValue(const Standard_Real aValue) { myFuzzyValue = aValue; };

  //
  Standard_Real FuzzyValue() const { return myFuzzyValue; };

  //
  void SetNonDestructive(const Standard_Boolean theFlag) { myNonDestructive = theFlag; };

  //
  Standard_Boolean NonDestructive() const { return myNonDestructive; };

  //
  void SetGlue(const BOPAlgo_GlueEnum theGlue) { myGlue = theGlue; };

  //
  BOPAlgo_GlueEnum Glue() const { return myGlue; };

  //
  void SetDrawWarnShapes(const Standard_Boolean bDraw) { myDrawWarnShapes = bDraw; };

  //
  Standard_Boolean DrawWarnShapes() const { return myDrawWarnShapes; };

  //
  void SetCheckInverted(const Standard_Boolean bCheck) { myCheckInverted = bCheck; };

  //
  Standard_Boolean CheckInverted() const { return myCheckInverted; };

  //
  void SetUseOBB(const Standard_Boolean bUse) { myUseOBB = bUse; };

  //
  Standard_Boolean UseOBB() const { return myUseOBB; };

  // Controls the Unification of Edges after BOP
  void SetUnifyEdges(const Standard_Boolean bUE) { myUnifyEdges = bUE; }

  // Returns flag of Edges unification
  Standard_Boolean UnifyEdges() const { return myUnifyEdges; }

  // Controls the Unification of Faces after BOP
  void SetUnifyFaces(const Standard_Boolean bUF) { myUnifyFaces = bUF; }

  // Returns flag of Faces unification
  Standard_Boolean UnifyFaces() const { return myUnifyFaces; }

  // Sets angular tolerance for Edges and Faces unification
  void SetAngular(const Standard_Real theAngTol) { myAngTol = theAngTol; }

  // Returns angular tolerance
  Standard_Real Angular() const { return myAngTol; }

protected:
  //
  BOPTest_Session(const BOPTest_Session&);
  BOPTest_Session& operator=(const BOPTest_Session&);
  //
protected:
  //
  BooleanPaveFiller* myPaveFiller;
  BOPAlgo_Builder*    myBuilder;
  BOPAlgo_Builder*    myBuilderDefault;
  //
  ShapeList myShapes;
  ShapeList myTools;
  Standard_Boolean     myRunParallel;
  Standard_Boolean     myNonDestructive;
  Standard_Real        myFuzzyValue;
  BOPAlgo_GlueEnum     myGlue;
  Standard_Boolean     myDrawWarnShapes;
  Standard_Boolean     myCheckInverted;
  Standard_Boolean     myUseOBB;
  Standard_Boolean     myUnifyEdges;
  Standard_Boolean     myUnifyFaces;
  Standard_Real        myAngTol;
};

//
//=================================================================================================

static BOPTest_Session& GetSession()
{
  static BOPTest_Session* pSession = new BOPTest_Session;
  //
  if (!pSession->IsValid())
  {
    pSession->Init();
  }
  return *pSession;
}

//
//=================================================================================================

void Objects::Init()
{
  GetSession().Init();
}

//=================================================================================================

void Objects::Clear()
{
  GetSession().Clear();
  Shapes().Clear();
  Tools().Clear();
}

//=================================================================================================

BooleanPaveFiller& Objects::PaveFiller()
{
  return GetSession().PaveFiller();
}

//=================================================================================================

BOPDS_PDS Objects::PDS()
{
  return Objects::PaveFiller().PDS();
}

//=================================================================================================

BOPAlgo_Builder& Objects::Builder()
{
  return GetSession().Builder();
}

//=================================================================================================

void Objects::SetBuilder(const BOPAlgo_PBuilder& theBuilder)
{
  BOPAlgo_Builder* pB;
  //
  pB = (BOPAlgo_Builder*)theBuilder;
  GetSession().SetBuilder(pB);
}

//=================================================================================================

void Objects::SetBuilderDefault()
{
  GetSession().SetBuilderDefault();
}

//=================================================================================================

BOPAlgo_BOP& Objects::BOP()
{
  static BOPAlgo_BOP sBOP(Allocator1());
  return sBOP;
}

//=================================================================================================

BOPAlgo_Section& Objects::Section()
{
  static BOPAlgo_Section sSection(Allocator1());
  return sSection;
}

//=================================================================================================

BOPAlgo_CellsBuilder& Objects::CellsBuilder()
{
  static BOPAlgo_CellsBuilder sCBuilder(Allocator1());
  return sCBuilder;
}

//=================================================================================================

BOPAlgo_Splitter& Objects::Splitter()
{
  static BOPAlgo_Splitter aSplitter(Allocator1());
  return aSplitter;
}

//=================================================================================================

ShapeList& Objects::Shapes()
{
  return GetSession().Shapes();
}

//=================================================================================================

ShapeList& Objects::Tools()
{
  return GetSession().Tools();
}

//=================================================================================================

void Objects::SetDefaultOptions()
{
  GetSession().SetDefaultOptions();
}

//=================================================================================================

void Objects::SetRunParallel(const Standard_Boolean bFlag)
{
  GetSession().SetRunParallel(bFlag);
}

//=================================================================================================

Standard_Boolean Objects::RunParallel()
{
  return GetSession().RunParallel();
}

//=================================================================================================

void Objects::SetFuzzyValue(const Standard_Real aValue)
{
  GetSession().SetFuzzyValue(aValue);
}

//=================================================================================================

Standard_Real Objects::FuzzyValue()
{
  return GetSession().FuzzyValue();
}

//=================================================================================================

void Objects::SetNonDestructive(const Standard_Boolean theFlag)
{
  GetSession().SetNonDestructive(theFlag);
}

//=================================================================================================

Standard_Boolean Objects::NonDestructive()
{
  return GetSession().NonDestructive();
}

//=================================================================================================

void Objects::SetGlue(const BOPAlgo_GlueEnum theGlue)
{
  GetSession().SetGlue(theGlue);
}

//=================================================================================================

BOPAlgo_GlueEnum Objects::Glue()
{
  return GetSession().Glue();
}

//=================================================================================================

void Objects::SetDrawWarnShapes(const Standard_Boolean bDraw)
{
  GetSession().SetDrawWarnShapes(bDraw);
}

//=================================================================================================

Standard_Boolean Objects::DrawWarnShapes()
{
  return GetSession().DrawWarnShapes();
}

//=================================================================================================

void Objects::SetCheckInverted(const Standard_Boolean bCheck)
{
  GetSession().SetCheckInverted(bCheck);
}

//=================================================================================================

Standard_Boolean Objects::CheckInverted()
{
  return GetSession().CheckInverted();
}

//=================================================================================================

void Objects::SetUseOBB(const Standard_Boolean bUseOBB)
{
  GetSession().SetUseOBB(bUseOBB);
}

//=================================================================================================

Standard_Boolean Objects::UseOBB()
{
  return GetSession().UseOBB();
}

//=================================================================================================

void Objects::SetUnifyEdges(const Standard_Boolean bUE)
{
  GetSession().SetUnifyEdges(bUE);
}

//=================================================================================================

Standard_Boolean Objects::UnifyEdges()
{
  return GetSession().UnifyEdges();
}

//=================================================================================================

void Objects::SetUnifyFaces(const Standard_Boolean bUF)
{
  GetSession().SetUnifyFaces(bUF);
}

//=================================================================================================

Standard_Boolean Objects::UnifyFaces()
{
  return GetSession().UnifyFaces();
}

//=================================================================================================

void Objects::SetAngular(const Standard_Real theAngTol)
{
  GetSession().SetAngular(theAngTol);
}

//=================================================================================================

Standard_Real Objects::Angular()
{
  return GetSession().Angular();
}

//=================================================================================================

Handle(NCollection_BaseAllocator)& Allocator1()
{
  static Handle(NCollection_BaseAllocator) sAL1 = NCollection_BaseAllocator::CommonBaseAllocator();
  return sAL1;
}

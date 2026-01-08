// Copyright (c) 2023 OPEN CASCADE SAS
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

#include <XSDRAWSTL.hxx>

#include <AIS_InteractiveContext.hxx>
#include <BRep_Builder.hxx>
#include <DBRep.hxx>
#include <DDocStd.hxx>
#include <DDocStd_DrawDocument.hxx>
#include <Draw.hxx>
#include <Draw_Interpretor.hxx>
#include <Draw_PluginMacro.hxx>
#include <Draw_ProgressIndicator.hxx>
#include <MeshVS_DeformedDataSource.hxx>
#include <MeshVS_Drawer.hxx>
#include <MeshVS_DrawerAttribute.hxx>
#include <MeshVS_ElementalColorPrsBuilder.hxx>
#include <MeshVS_Mesh.hxx>
#include <MeshVS_MeshEntityOwner.hxx>
#include <MeshVS_MeshPrsBuilder.hxx>
#include <MeshVS_NodalColorPrsBuilder.hxx>
#include <MeshVS_TextPrsBuilder.hxx>
#include <MeshVS_VectorPrsBuilder.hxx>
#include <RWStl.hxx>
#include <StlAPI.hxx>
#include <StlAPI_Writer.hxx>
#include <TColStd_HPackedMapOfInteger.hxx>
#include <TDataStd_Name.hxx>
#include <TDocStd_Application.hxx>
#include <TopoDS_Shape.hxx>
#include <V3d_View.hxx>
#include <ViewerTest.hxx>
#include <XSControl_WorkSession.hxx>
#include <XSDRAW.hxx>
#include <XSDRAWSTL_DataSource.hxx>
#include <XSDRAWSTL_DataSource3D.hxx>
#include <XSDRAWSTL_DrawableMesh.hxx>

extern Standard_Boolean VDisplayAISObject(const AsciiString1&       theName,
                                          const Handle(VisualEntity)& theAISObj,
                                          Standard_Boolean theReplaceIfExists = Standard_True);

//=================================================================================================

static Standard_Integer writestl(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc < 3 || argc > 4)
  {
    di << "Use: " << argv[0] << " shape file [ascii/binary (0/1) : 1 by default]\n";
  }
  else
  {
    TopoShape     aShape      = DBRep1::Get(argv[1]);
    Standard_Boolean isASCIIMode = Standard_False;
    if (argc == 4)
    {
      isASCIIMode = (Draw1::Atoi(argv[3]) == 0);
    }
    StlWriter aWriter;
    aWriter.ASCIIMode()                      = isASCIIMode;
    Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(di);
    Standard_Boolean               isOK      = aWriter.Write(aShape, argv[2], aProgress->Start());
    if (!isOK)
      di << "** Error **: Mesh1 writing has been failed.\n";
  }
  return 0;
}

//=============================================================================
// function : readstl
// purpose  : Reads stl file
//=============================================================================
static Standard_Integer readstl(DrawInterpreter& theDI,
                                Standard_Integer  theArgc,
                                const char**      theArgv)
{
  AsciiString1 aShapeName, aFilePath;
  bool                    toCreateCompOfTris = false;
  bool                    anIsMulti          = false;
  double                  aMergeAngle        = M_PI / 2.0;
  for (Standard_Integer anArgIter = 1; anArgIter < theArgc; ++anArgIter)
  {
    AsciiString1 anArg(theArgv[anArgIter]);
    anArg.LowerCase();
    if (aShapeName.IsEmpty())
    {
      aShapeName = theArgv[anArgIter];
    }
    else if (aFilePath.IsEmpty())
    {
      aFilePath = theArgv[anArgIter];
    }
    else if (anArg == "-brep")
    {
      toCreateCompOfTris = true;
      if (anArgIter + 1 < theArgc && Draw1::ParseOnOff(theArgv[anArgIter + 1], toCreateCompOfTris))
      {
        ++anArgIter;
      }
    }
    else if (anArg == "-multi")
    {
      anIsMulti = true;
      if (anArgIter + 1 < theArgc && Draw1::ParseOnOff(theArgv[anArgIter + 1], anIsMulti))
      {
        ++anArgIter;
      }
    }
    else if (anArg == "-mergeangle" || anArg == "-smoothangle" || anArg == "-nomergeangle"
             || anArg == "-nosmoothangle")
    {
      if (anArg.StartsWith("-no"))
      {
        aMergeAngle = M_PI / 2.0;
      }
      else
      {
        aMergeAngle = M_PI / 4.0;
        if (anArgIter + 1 < theArgc && Draw1::ParseReal(theArgv[anArgIter + 1], aMergeAngle))
        {
          if (aMergeAngle < 0.0 || aMergeAngle > 90.0)
          {
            theDI << "Syntax error: angle should be within [0,90] range";
            return 1;
          }

          ++anArgIter;
          aMergeAngle = aMergeAngle * M_PI / 180.0;
        }
      }
    }
    else
    {
      Message1::SendFail() << "Syntax error: unknown argument '" << theArgv[anArgIter] << "'";
      return 1;
    }
  }
  if (aFilePath.IsEmpty())
  {
    Message1::SendFail() << "Syntax error: not enough arguments";
    return 1;
  }

  TopoShape aShape;
  if (!toCreateCompOfTris)
  {
    Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(theDI, 1);
    if (anIsMulti)
    {
      NCollection_Sequence<Handle(MeshTriangulation)> aTriangList;
      // Read STL file to the triangulation list.
      RWStl1::ReadFile(aFilePath.ToCString(), aMergeAngle, aTriangList, aProgress->Start());
      ShapeBuilder aB;
      TopoFace  aFace;
      if (aTriangList.Size() == 1)
      {
        aB.MakeFace(aFace);
        aB.UpdateFace(aFace, aTriangList.First());
        aShape = aFace;
      }
      else
      {
        TopoCompound aCmp;
        aB.MakeCompound(aCmp);

        NCollection_Sequence<Handle(MeshTriangulation)>::Iterator anIt(aTriangList);
        for (; anIt.More(); anIt.Next())
        {
          aB.MakeFace(aFace);
          aB.UpdateFace(aFace, anIt.Value());
          aB.Add(aCmp, aFace);
        }
        aShape = aCmp;
      }
    }
    else
    {
      // Read STL file to the triangulation.
      Handle(MeshTriangulation) aTriangulation =
        RWStl1::ReadFile(aFilePath.ToCString(), aMergeAngle, aProgress->Start());

      TopoFace  aFace;
      ShapeBuilder aB;
      aB.MakeFace(aFace);
      aB.UpdateFace(aFace, aTriangulation);
      aShape = aFace;
    }
  }
  else
  {
    Standard_DISABLE_DEPRECATION_WARNINGS StlAPI1::Read(aShape, aFilePath.ToCString());
    Standard_ENABLE_DEPRECATION_WARNINGS
  }
  DBRep1::Set(aShapeName.ToCString(), aShape);
  return 0;
}

//=================================================================================================

static Standard_Integer createmesh(DrawInterpreter& theDI,
                                   Standard_Integer  theNbArgs,
                                   const char**      theArgVec)
{
  if (theNbArgs < 3)
  {
    theDI << "Wrong number of parameters\n";
    theDI << "Use: " << theArgVec[0] << " <mesh name> <stl file>\n";
    return 0;
  }

  Handle(VisualContext) aContext = ViewerTest1::GetAISContext();
  if (aContext.IsNull())
  {
    theDI << "No active view. Please call 'vinit' first\n";
    return 0;
  }

  // Progress indicator
  SystemPath                       aFile(theArgVec[2]);
  Handle(Draw_ProgressIndicator) aProgress = new Draw_ProgressIndicator(theDI, 1);
  Handle(MeshTriangulation)     aSTLMesh  = RWStl1::ReadFile(aFile, aProgress->Start());

  theDI << "Reading OK...\n";
  Handle(XSDRAWSTL_DataSource) aDS = new XSDRAWSTL_DataSource(aSTLMesh);
  theDI << "Data source is created successful\n";
  Handle(MeshVS_Mesh) aMesh = new MeshVS_Mesh();
  theDI << "MeshVS_Mesh is created successful\n";

  aMesh->SetDataSource(aDS);
  aMesh->AddBuilder(new MeshVS_MeshPrsBuilder(aMesh.operator->()), Standard_True);

  aMesh->GetDrawer()->SetColor(MeshVS_DA_EdgeColor, Quantity_NOC_YELLOW);

  // Hide all nodes by default
  Handle(TColStd_HPackedMapOfInteger) aNodes = new TColStd_HPackedMapOfInteger();
  const Standard_Integer              aLen   = aSTLMesh->NbNodes();
  for (Standard_Integer anIndex = 1; anIndex <= aLen; anIndex++)
    aNodes->ChangeMap().Add(anIndex);
  aMesh->SetHiddenNodes(aNodes);
  aMesh->SetSelectableNodes(aNodes);

  VDisplayAISObject(theArgVec[1], aMesh);
  aContext->Deactivate(aMesh);

  Draw1::Set(theArgVec[1], new XSDRAWSTL_DrawableMesh(aMesh));
  Handle(ViewWindow) aView = ViewerTest1::CurrentView();
  if (!aView.IsNull())
    aView->FitAll();

  return 0;
}

//=================================================================================================

static Standard_Integer create3d(DrawInterpreter& theDI,
                                 Standard_Integer  theNbArgs,
                                 const char**      theArgVec)
{
  if (theNbArgs < 2)
  {
    theDI << "Wrong number of parameters\n";
    theDI << "Use: " << theArgVec[0] << " <mesh name>\n";
    return 0;
  }

  Handle(VisualContext) aContext = ViewerTest1::GetAISContext();
  if (aContext.IsNull())
  {
    theDI << "No active view. Please call 'vinit' first\n";
    return 0;
  }

  Handle(XSDRAWSTL_DataSource3D) aDS = new XSDRAWSTL_DataSource3D();
  theDI << "Data source is created successful\n";
  Handle(MeshVS_Mesh) aMesh = new MeshVS_Mesh();
  theDI << "MeshVS_Mesh is created successful\n";

  aMesh->SetDataSource(aDS);
  aMesh->AddBuilder(new MeshVS_MeshPrsBuilder(aMesh.operator->()), Standard_True);

  aMesh->GetDrawer()->SetColor(MeshVS_DA_EdgeColor, Quantity_NOC_YELLOW);

  // Hide all nodes by default
  Handle(TColStd_HPackedMapOfInteger) aNodes = new TColStd_HPackedMapOfInteger();
  Standard_Integer                    aLen   = aDS->GetAllNodes().Extent();
  for (Standard_Integer anIndex = 1; anIndex <= aLen; anIndex++)
    aNodes->ChangeMap().Add(anIndex);
  aMesh->SetHiddenNodes(aNodes);
  aMesh->SetSelectableNodes(aNodes);

  VDisplayAISObject(theArgVec[1], aMesh);
  aContext->Deactivate(aMesh);

  Draw1::Set(theArgVec[1], new XSDRAWSTL_DrawableMesh(aMesh));
  Handle(ViewWindow) aView = ViewerTest1::CurrentView();
  if (!aView.IsNull())
    aView->FitAll();

  return 0;
}

//=================================================================================================

Handle(MeshVS_Mesh) getMesh(const char* theName, DrawInterpreter& theDI)
{
  Handle(XSDRAWSTL_DrawableMesh) aDrawMesh =
    Handle(XSDRAWSTL_DrawableMesh)::DownCast(Draw1::Get(theName));

  if (aDrawMesh.IsNull())
  {
    theDI << "There is no such object\n";
    return NULL;
  }
  else
  {
    Handle(MeshVS_Mesh) aMesh = aDrawMesh->GetMesh();
    if (aMesh.IsNull())
    {
      theDI << "There is invalid mesh\n";
      return NULL;
    }
    else
      return aMesh;
  }
}

//=================================================================================================

static Standard_Integer setcolor(DrawInterpreter& theDI,
                                 Standard_Integer  theNbArgs,
                                 const char**      theArgVec,
                                 Standard_Integer  theParam)
{
  if (theNbArgs < 5)
    theDI << "Wrong number of parameters\n";
  else
  {
    Handle(MeshVS_Mesh) aMesh = getMesh(theArgVec[1], theDI);
    if (!aMesh.IsNull())
    {
      Standard_Real aRed   = Draw1::Atof(theArgVec[2]);
      Standard_Real aGreen = Draw1::Atof(theArgVec[3]);
      Standard_Real aBlue  = Draw1::Atof(theArgVec[4]);
      aMesh->GetDrawer()->SetColor((MeshVS_DrawerAttribute)theParam,
                                   Quantity_Color(aRed, aGreen, aBlue, Quantity_TOC_RGB));

      Handle(VisualContext) aContext = ViewerTest1::GetAISContext();

      if (aContext.IsNull())
        theDI << "The context is null\n";
      else
        aContext->Redisplay(aMesh, Standard_True);
    }
  }
  return 0;
}

//=================================================================================================

static Standard_Integer meshcolor(DrawInterpreter& theInterp,
                                  Standard_Integer  theNbArgs,
                                  const char**      theArgVec)
{
  return setcolor(theInterp, theNbArgs, theArgVec, MeshVS_DA_InteriorColor);
}

//=================================================================================================

static Standard_Integer linecolor(DrawInterpreter& theInterp,
                                  Standard_Integer  theNbArgs,
                                  const char**      theArgVec)
{
  return setcolor(theInterp, theNbArgs, theArgVec, MeshVS_DA_EdgeColor);
}

//=================================================================================================

static Standard_Integer meshmat(DrawInterpreter& theDI,
                                Standard_Integer  theNbArgs,
                                const char**      theArgVec)
{
  if (theNbArgs < 3)
    theDI << "Wrong number of parameters\n";
  else
  {
    Handle(MeshVS_Mesh) aMesh = getMesh(theArgVec[1], theDI);
    if (!aMesh.IsNull())
    {
      Standard_Integer aMaterial = Draw1::Atoi(theArgVec[2]);

      Graphic3d_MaterialAspect aMatAsp =
        (Graphic3d_MaterialAspect)(Graphic3d_NameOfMaterial)aMaterial;

      if (theNbArgs == 4)
      {
        Standard_Real aTransparency = Draw1::Atof(theArgVec[3]);
        aMatAsp.SetTransparency(Standard_ShortReal(aTransparency));
      }
      aMesh->GetDrawer()->SetMaterial(MeshVS_DA_FrontMaterial, aMatAsp);
      aMesh->GetDrawer()->SetMaterial(MeshVS_DA_BackMaterial, aMatAsp);

      Handle(VisualContext) aContext = ViewerTest1::GetAISContext();

      if (aContext.IsNull())
        theDI << "The context is null\n";
      else
        aContext->Redisplay(aMesh, Standard_True);
    }
  }
  return 0;
}

//=================================================================================================

static Standard_Integer shrink(DrawInterpreter& theDI,
                               Standard_Integer  theNbArgs,
                               const char**      theArgVec)
{
  if (theNbArgs < 3)
    theDI << "Wrong number of parameters\n";
  else
  {
    Handle(MeshVS_Mesh) aMesh = getMesh(theArgVec[1], theDI);
    if (!aMesh.IsNull())
    {
      Standard_Real aShrinkCoeff = Draw1::Atof(theArgVec[2]);
      aMesh->GetDrawer()->SetDouble(MeshVS_DA_ShrinkCoeff, aShrinkCoeff);

      Handle(VisualContext) aContext = ViewerTest1::GetAISContext();

      if (aContext.IsNull())
        theDI << "The context is null\n";
      else
        aContext->Redisplay(aMesh, Standard_True);
    }
  }
  return 0;
}

//=================================================================================================

static Standard_Integer closed(DrawInterpreter& theDI,
                               Standard_Integer  theArgc,
                               const char**      theArgv)
{
  if (theArgc < 3)
  {
    theDI << "Wrong number of parameters.\n";
  }
  else
  {
    Handle(MeshVS_Mesh) aMesh = getMesh(theArgv[1], theDI);
    if (!aMesh.IsNull())
    {
      Standard_Boolean aFlag = Draw1::Atoi(theArgv[2]) != 0;
      aMesh->GetDrawer()->SetBoolean(MeshVS_DA_SupressBackFaces, aFlag);

      Handle(VisualContext) aContext = ViewerTest1::GetAISContext();
      if (aContext.IsNull())
      {
        theDI << "The context is null\n";
      }
      else
      {
        aContext->Redisplay(aMesh, Standard_True);
      }
    }
  }
  return 0;
}

//=================================================================================================

static Standard_Integer mdisplay(DrawInterpreter& theDI,
                                 Standard_Integer  theNbArgs,
                                 const char**      theArgVec)
{
  if (theNbArgs < 2)
    theDI << "Wrong number of parameters\n";
  else
  {
    Handle(MeshVS_Mesh) aMesh = getMesh(theArgVec[1], theDI);
    if (!aMesh.IsNull())
    {
      Handle(VisualContext) aContext = ViewerTest1::GetAISContext();

      if (aContext.IsNull())
        theDI << "The context is null\n";
      else
      {
        aContext->Display(aMesh, Standard_True);
      }
    }
  }
  return 0;
}

//=================================================================================================

static Standard_Integer merase(DrawInterpreter& theDI,
                               Standard_Integer  theNbArgs,
                               const char**      theArgVec)
{
  if (theNbArgs < 2)
    theDI << "Wrong number of parameters\n";
  else
  {
    Handle(MeshVS_Mesh) aMesh = getMesh(theArgVec[1], theDI);
    if (!aMesh.IsNull())
    {
      Handle(VisualContext) aContext = ViewerTest1::GetAISContext();

      if (aContext.IsNull())
        theDI << "The context is null\n";
      else
      {
        aContext->Erase(aMesh, Standard_True);
      }
    }
    else
      theDI << "Mesh1 is null\n";
  }
  return 0;
}

//=================================================================================================

static Standard_Integer hidesel(DrawInterpreter& theDI,
                                Standard_Integer  theNbArgs,
                                const char**      theArgVec)
{
  if (theNbArgs < 2)
  {
    theDI << "Wrong number of parameters\n";
    theDI << "Use: " << theArgVec[0] << " <mesh name>\n";
    return 0;
  }

  Handle(VisualContext) aContext = ViewerTest1::GetAISContext();
  Handle(MeshVS_Mesh)            aMesh    = getMesh(theArgVec[1], theDI);
  if (aMesh.IsNull())
  {
    theDI << "The mesh is invalid\n";
    return 0;
  }

  if (aContext.IsNull())
    theDI << "The context is null\n";
  else
  {
    Handle(TColStd_HPackedMapOfInteger) aHiddenNodes = aMesh->GetHiddenNodes();
    if (aHiddenNodes.IsNull())
    {
      aHiddenNodes = new TColStd_HPackedMapOfInteger();
    }
    Handle(TColStd_HPackedMapOfInteger) aHiddenElements = aMesh->GetHiddenElems();
    if (aHiddenElements.IsNull())
    {
      aHiddenElements = new TColStd_HPackedMapOfInteger();
    }
    for (aContext->InitSelected(); aContext->MoreSelected(); aContext->NextSelected())
    {
      Handle(MeshVS_MeshEntityOwner) anOwner =
        Handle(MeshVS_MeshEntityOwner)::DownCast(aContext->SelectedOwner());
      if (!anOwner.IsNull())
      {
        if (anOwner->Type() == MeshVS_ET_Node)
        {
          aHiddenNodes->ChangeMap().Add(anOwner->ID());
        }
        else
        {
          aHiddenElements->ChangeMap().Add(anOwner->ID());
        }
      }
    }
    aContext->ClearSelected(Standard_False);
    aMesh->SetHiddenNodes(aHiddenNodes);
    aMesh->SetHiddenElems(aHiddenElements);
    aContext->Redisplay(aMesh, Standard_True);
  }

  return 0;
}

//=================================================================================================

static Standard_Integer showonly(DrawInterpreter& theDI,
                                 Standard_Integer  theNbArgs,
                                 const char**      theArgVec)
{
  if (theNbArgs < 2)
  {
    theDI << "Wrong number of parameters\n";
    theDI << "Use: " << theArgVec[0] << " <mesh name>\n";
    return 0;
  }

  Handle(VisualContext) aContext = ViewerTest1::GetAISContext();
  Handle(MeshVS_Mesh)            aMesh    = getMesh(theArgVec[1], theDI);
  if (aMesh.IsNull())
  {
    theDI << "The mesh is invalid\n";
    return 0;
  }

  if (aContext.IsNull())
    theDI << "The context is null\n";
  else
  {
    Handle(TColStd_HPackedMapOfInteger) aHiddenNodes =
      new TColStd_HPackedMapOfInteger(aMesh->GetDataSource()->GetAllNodes());
    Handle(TColStd_HPackedMapOfInteger) aHiddenElements =
      new TColStd_HPackedMapOfInteger(aMesh->GetDataSource()->GetAllElements());
    for (aContext->InitSelected(); aContext->MoreSelected(); aContext->NextSelected())
    {
      Handle(MeshVS_MeshEntityOwner) anOwner =
        Handle(MeshVS_MeshEntityOwner)::DownCast(aContext->SelectedOwner());
      if (!anOwner.IsNull())
      {
        if (anOwner->Type() == MeshVS_ET_Node)
        {
          aHiddenNodes->ChangeMap().Remove(anOwner->ID());
        }
        else
        {
          aHiddenElements->ChangeMap().Remove(anOwner->ID());
        }
      }
    }
    aMesh->SetHiddenNodes(aHiddenNodes);
    aMesh->SetHiddenElems(aHiddenElements);
    aContext->Redisplay(aMesh, Standard_True);
  }

  return 0;
}

//=================================================================================================

static Standard_Integer showall(DrawInterpreter& theDI,
                                Standard_Integer  theNbArgs,
                                const char**      theArgVec)
{
  if (theNbArgs < 2)
  {
    theDI << "Wrong number of parameters\n";
    theDI << "Use: " << theArgVec[0] << " <mesh name>\n";
    return 0;
  }

  Handle(VisualContext) aContext = ViewerTest1::GetAISContext();
  Handle(MeshVS_Mesh)            aMesh    = getMesh(theArgVec[1], theDI);
  if (aMesh.IsNull())
  {
    theDI << "The mesh is invalid\n";
    return 0;
  }

  if (aContext.IsNull())
    theDI << "The context is null\n";
  else
  {
    aMesh->SetHiddenNodes(new TColStd_HPackedMapOfInteger());
    aMesh->SetHiddenElems(new TColStd_HPackedMapOfInteger());
    aContext->Redisplay(aMesh, Standard_True);
  }

  return 0;
}

//=================================================================================================

static Standard_Integer meshcolors(DrawInterpreter& theDI,
                                   Standard_Integer  theNbArgs,
                                   const char**      theArgVec)
{
  try
  {
    OCC_CATCH_SIGNALS
    if (theNbArgs < 4)
    {
      theDI << "Wrong number of parameters\n";
      theDI << "Use : meshcolors <mesh name> <mode> <isreflect>\n";
      theDI << "mode : {elem1|elem2|nodal|nodaltex|none}\n";
      theDI << "       elem1 - different color for each element\n";
      theDI << "       elem2 - one color for one side\n";
      theDI << "       nodal - different color for each node\n";
      theDI << "       nodaltex - different color for each node with texture interpolation\n";
      theDI << "       none  - clear\n";
      theDI << "isreflect : {0|1} \n";

      return 0;
    }

    Handle(MeshVS_Mesh) aMesh = getMesh(theArgVec[1], theDI);

    if (aMesh.IsNull())
    {
      theDI << "Mesh1 not found\n";
      return 0;
    }
    Handle(VisualContext) anIC = ViewerTest1::GetAISContext();
    if (anIC.IsNull())
    {
      theDI << "The context is null\n";
      return 0;
    }
    if (!aMesh.IsNull())
    {
      AsciiString1 aMode = AsciiString1(theArgVec[2]);
      Quantity_Color          aColor1(Quantity_NOC_BLUE1);
      Quantity_Color          aColor2(Quantity_NOC_RED1);
      if (aMode.IsEqual("elem1") || aMode.IsEqual("elem2") || aMode.IsEqual("nodal")
          || aMode.IsEqual("nodaltex") || aMode.IsEqual("none"))
      {
        Handle(MeshVS_PrsBuilder) aTempBuilder;
        Standard_Integer          aReflection = Draw1::Atoi(theArgVec[3]);

        for (Standard_Integer aCount = 0; aCount < aMesh->GetBuildersCount(); aCount++)
        {
          aTempBuilder = aMesh->FindBuilder(STANDARD_TYPE(MeshVS_ElementalColorPrsBuilder));
          if (!aTempBuilder.IsNull())
            aMesh->RemoveBuilderById(aTempBuilder->GetId());

          aTempBuilder = aMesh->FindBuilder(STANDARD_TYPE(MeshVS_NodalColorPrsBuilder));
          if (!aTempBuilder.IsNull())
            aMesh->RemoveBuilderById(aTempBuilder->GetId());
        }

        if (aMode.IsEqual("elem1") || aMode.IsEqual("elem2"))
        {
          Handle(MeshVS_ElementalColorPrsBuilder) aBuilder = new MeshVS_ElementalColorPrsBuilder(
            aMesh,
            MeshVS_DMF_ElementalColorDataPrs | MeshVS_DMF_OCCMask);
          // Color
          const TColStd_PackedMapOfInteger& anAllElements =
            aMesh->GetDataSource()->GetAllElements();

          if (aMode.IsEqual("elem1"))
            for (TColStd_PackedMapOfInteger::Iterator anIter(anAllElements); anIter.More();
                 anIter.Next())
            {
              Quantity_Color aColor((Quantity_NameOfColor)(anIter.Key1() % Quantity_NOC_WHITE));
              aBuilder->SetColor1(anIter.Key1(), aColor);
            }
          else
            for (TColStd_PackedMapOfInteger::Iterator anIter(anAllElements); anIter.More();
                 anIter.Next())
            {
              aBuilder->SetColor2(anIter.Key1(), aColor1, aColor2);
            }

          aMesh->AddBuilder(aBuilder, Standard_True);
        }

        if (aMode.IsEqual("nodal"))
        {
          Handle(MeshVS_NodalColorPrsBuilder) aBuilder =
            new MeshVS_NodalColorPrsBuilder(aMesh,
                                            MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
          aMesh->AddBuilder(aBuilder, Standard_True);

          // Color
          const TColStd_PackedMapOfInteger& anAllNodes = aMesh->GetDataSource()->GetAllNodes();
          for (TColStd_PackedMapOfInteger::Iterator anIter(anAllNodes); anIter.More();
               anIter.Next())
          {
            Quantity_Color aColor((Quantity_NameOfColor)(anIter.Key1() % Quantity_NOC_WHITE));
            aBuilder->SetColor(anIter.Key1(), aColor);
          }
          aMesh->AddBuilder(aBuilder, Standard_True);
        }

        if (aMode.IsEqual("nodaltex"))
        {
          // assign nodal builder to the mesh
          Handle(MeshVS_NodalColorPrsBuilder) aBuilder =
            new MeshVS_NodalColorPrsBuilder(aMesh,
                                            MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
          aMesh->AddBuilder(aBuilder, Standard_True);
          aBuilder->UseTexture(Standard_True);

          // prepare color map for texture
          Aspect_SequenceOfColor aColorMap;
          aColorMap.Append((Quantity_NameOfColor)Quantity_NOC_RED);
          aColorMap.Append((Quantity_NameOfColor)Quantity_NOC_YELLOW);
          aColorMap.Append((Quantity_NameOfColor)Quantity_NOC_BLUE1);

          // prepare scale map for mesh - it will be assigned to mesh as texture coordinates
          // make mesh color interpolated from minimum X coord to maximum X coord
          Handle(MeshVS_DataSource) aDataSource = aMesh->GetDataSource();
          Standard_Real             aMinX, aMinY, aMinZ, aMaxX, aMaxY, aMaxZ;

          // get bounding box for calculations
          aDataSource->GetBoundingBox().Get(aMinX, aMinY, aMinZ, aMaxX, aMaxY, aMaxZ);
          Standard_Real aDelta = aMaxX - aMinX;

          // assign color scale map values (0..1) to nodes
          TColStd_DataMapOfIntegerReal aScaleMap;
          TColStd_Array1OfReal         aCoords(1, 3);
          Standard_Integer             aNbNodes;
          MeshVS_EntityType            aType;

          // iterate nodes
          const TColStd_PackedMapOfInteger& anAllNodes = aMesh->GetDataSource()->GetAllNodes();
          for (TColStd_PackedMapOfInteger::Iterator anIter(anAllNodes); anIter.More();
               anIter.Next())
          {
            // get node coordinates to aCoord variable
            aDataSource->GetGeom(anIter.Key1(), Standard_False, aCoords, aNbNodes, aType);

            Standard_Real aScaleValue;
            try
            {
              OCC_CATCH_SIGNALS
              aScaleValue = (aCoords.Value(1) - (Standard_Real)aMinX) / aDelta;
            }
            catch (ExceptionBase const&)
            {
              aScaleValue = 0;
            }

            aScaleMap.Bind(anIter.Key1(), aScaleValue);
          }

          // set color map for builder and a color for invalid scale value
          aBuilder->SetColorMap(aColorMap);
          aBuilder->SetInvalidColor(Quantity_NOC_BLACK);
          aBuilder->SetTextureCoords(aScaleMap);
          aMesh->AddBuilder(aBuilder, Standard_True);
        }

        aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ColorReflection, aReflection != 0);

        anIC->Redisplay(aMesh, Standard_True);
      }
      else
      {
        theDI << "Wrong mode name\n";
        return 0;
      }
    }
  }
  catch (ExceptionBase const&)
  {
    theDI << "Error\n";
  }

  return 0;
}

//=================================================================================================

static Standard_Integer meshvectors(DrawInterpreter& theDI,
                                    Standard_Integer  theNbArgs,
                                    const char**      theArgVec)
{
  if (theNbArgs < 3)
  {
    theDI << "Wrong number of parameters\n";
    theDI << "Use : meshvectors <mesh name> < -mode {elem|nodal|none} > [-maxlen len] [-color "
             "name] [-arrowpart ratio] [-issimple {1|0}]\n";
    theDI << "Supported mode values:\n";
    theDI << "       elem  - vector per element\n";
    theDI << "       nodal - vector per node\n";
    theDI << "       none  - clear\n";

    return 0;
  }

  Handle(MeshVS_Mesh) aMesh = getMesh(theArgVec[1], theDI);

  if (aMesh.IsNull())
  {
    theDI << "Mesh1 not found\n";
    return 0;
  }
  Handle(VisualContext) anIC = ViewerTest1::GetAISContext();
  if (anIC.IsNull())
  {
    theDI << "The context is null\n";
    return 0;
  }

  AsciiString1 aParam;
  AsciiString1 aMode("none");
  Standard_Real           aMaxlen(1.0);
  Quantity_Color          aColor(Quantity_NOC_ORANGE);
  Standard_Real           anArrowPart(0.1);
  Standard_Boolean        isSimplePrs(Standard_False);

  for (Standard_Integer anIdx = 2; anIdx < theNbArgs; anIdx++)
  {
    if (!aParam.IsEmpty())
    {
      if (aParam == "-mode")
      {
        aMode = theArgVec[anIdx];
      }
      else if (aParam == "-maxlen")
      {
        aMaxlen = Draw1::Atof(theArgVec[anIdx]);
      }
      else if (aParam == "-color")
      {
        if (!Quantity_Color::ColorFromName(theArgVec[anIdx], aColor))
        {
          theDI << "Syntax error at " << aParam << "\n";
          return 1;
        }
      }
      else if (aParam == "-arrowpart")
      {
        anArrowPart = Draw1::Atof(theArgVec[anIdx]);
      }
      else if (aParam == "-issimple")
      {
        isSimplePrs = Draw1::Atoi(theArgVec[anIdx]) != 0;
      }
      aParam.Clear();
    }
    else if (theArgVec[anIdx][0] == '-')
    {
      aParam = theArgVec[anIdx];
    }
  }

  if (!aMode.IsEqual("elem") && !aMode.IsEqual("nodal") && !aMode.IsEqual("none"))
  {
    theDI << "Wrong mode name\n";
    return 0;
  }

  Handle(MeshVS_PrsBuilder) aTempBuilder;

  aTempBuilder = aMesh->FindBuilder(STANDARD_TYPE(MeshVS_VectorPrsBuilder));
  if (!aTempBuilder.IsNull())
    aMesh->RemoveBuilderById(aTempBuilder->GetId());

  if (!aMode.IsEqual("none"))
  {
    Handle(MeshVS_VectorPrsBuilder) aBuilder = new MeshVS_VectorPrsBuilder(aMesh.operator->(),
                                                                           aMaxlen,
                                                                           aColor,
                                                                           MeshVS_DMF_VectorDataPrs,
                                                                           0,
                                                                           -1,
                                                                           MeshVS_BP_Vector,
                                                                           isSimplePrs);

    Standard_Boolean                  anIsElement = aMode.IsEqual("elem");
    const TColStd_PackedMapOfInteger& anAllIDs    = anIsElement
                                                      ? aMesh->GetDataSource()->GetAllElements()
                                                      : aMesh->GetDataSource()->GetAllNodes();

    Standard_Integer  aNbNodes;
    MeshVS_EntityType aEntType;

    TColStd_Array1OfReal aCoords(1, 3);
    aCoords.Init(0.);
    for (TColStd_PackedMapOfInteger::Iterator anIter(anAllIDs); anIter.More(); anIter.Next())
    {
      Standard_Boolean IsValidData = Standard_False;
      if (anIsElement)
      {
        aMesh->GetDataSource()->GetGeomType(anIter.Key1(), anIsElement, aEntType);
        if (aEntType == MeshVS_ET_Face)
          IsValidData = aMesh->GetDataSource()->GetNormal(anIter.Key1(),
                                                          3,
                                                          aCoords.ChangeValue(1),
                                                          aCoords.ChangeValue(2),
                                                          aCoords.ChangeValue(3));
      }
      else
        IsValidData = aMesh->GetDataSource()->GetGeom(anIter.Key1(),
                                                      Standard_False,
                                                      aCoords,
                                                      aNbNodes,
                                                      aEntType);

      Vector3d aNorm;
      if (IsValidData)
      {
        aNorm = Vector3d(aCoords.Value(1), aCoords.Value(2), aCoords.Value(3));
        if (aNorm.Magnitude() < gp1::Resolution())
        {
          aNorm = Vector3d(0, 0, 1); // method GetGeom(...) returns coordinates of nodes
        }
      }
      else
      {
        aNorm = Vector3d(0, 0, 1);
      }
      aBuilder->SetVector(anIsElement, anIter.Key1(), aNorm.Normalized());
    }

    aMesh->AddBuilder(aBuilder, Standard_False);
    aMesh->GetDrawer()->SetDouble(MeshVS_DA_VectorArrowPart, anArrowPart);
  }

  anIC->Redisplay(aMesh, Standard_True);

  return 0;
}

//=================================================================================================

static Standard_Integer meshtext(DrawInterpreter& theDI,
                                 Standard_Integer  theNbArgs,
                                 const char**      theArgVec)
{
  if (theNbArgs < 2)
  {
    theDI << "Wrong number of parameters\n";
    theDI << "Use : meshtext <mesh name>\n";
    return 0;
  }

  Handle(MeshVS_Mesh) aMesh = getMesh(theArgVec[1], theDI);

  if (aMesh.IsNull())
  {
    theDI << "Mesh1 not found\n";
    return 0;
  }

  Handle(VisualContext) anIC = ViewerTest1::GetAISContext();
  if (anIC.IsNull())
  {
    theDI << "The context is null\n";
    return 0;
  }

  // Prepare triangle labels
  MeshVS_DataMapOfIntegerAsciiString aLabels;
  Standard_Integer                   aLen = aMesh->GetDataSource()->GetAllElements().Extent();
  for (Standard_Integer anIndex = 1; anIndex <= aLen; anIndex++)
  {
    aLabels.Bind(anIndex, AsciiString1(anIndex));
  }

  Handle(MeshVS_TextPrsBuilder) aTextBuilder =
    new MeshVS_TextPrsBuilder(aMesh.operator->(), 20., Quantity_NOC_YELLOW);
  aTextBuilder->SetTexts(Standard_True, aLabels);
  aMesh->AddBuilder(aTextBuilder);

  return 0;
}

//=================================================================================================

static Standard_Integer meshdeform(DrawInterpreter& theDI,
                                   Standard_Integer  theNbArgs,
                                   const char**      theArgVec)
{
  if (theNbArgs < 3)
  {
    theDI << "Wrong number of parameters\n";
    theDI << "Use : meshdeform <mesh name> < -mode {on|off} > [-scale scalefactor]\n";
    return 0;
  }

  Handle(MeshVS_Mesh) aMesh = getMesh(theArgVec[1], theDI);

  if (aMesh.IsNull())
  {
    theDI << "Mesh1 not found\n";
    return 0;
  }
  Handle(VisualContext) anIC = ViewerTest1::GetAISContext();
  if (anIC.IsNull())
  {
    theDI << "The context is null\n";
    return 0;
  }

  AsciiString1 aParam;
  AsciiString1 aMode("off");
  Standard_Real           aScale(1.0);

  for (Standard_Integer anIdx = 2; anIdx < theNbArgs; anIdx++)
  {
    if (!aParam.IsEmpty())
    {
      if (aParam == "-mode")
      {
        aMode = theArgVec[anIdx];
      }
      else if (aParam == "-scale")
      {
        aScale = Draw1::Atof(theArgVec[anIdx]);
      }
      aParam.Clear();
    }
    else if (theArgVec[anIdx][0] == '-')
    {
      aParam = theArgVec[anIdx];
    }
  }

  if (!aMode.IsEqual("on") && !aMode.IsEqual("off"))
  {
    theDI << "Wrong mode name\n";
    return 0;
  }

  Handle(MeshVS_DeformedDataSource) aDefDS =
    new MeshVS_DeformedDataSource(aMesh->GetDataSource(), aScale);

  const TColStd_PackedMapOfInteger& anAllIDs = aMesh->GetDataSource()->GetAllNodes();

  Standard_Integer  aNbNodes;
  MeshVS_EntityType aEntType;

  for (TColStd_PackedMapOfInteger::Iterator anIter(anAllIDs); anIter.More(); anIter.Next())
  {
    TColStd_Array1OfReal aCoords(1, 3);
    aMesh->GetDataSource()->GetGeom(anIter.Key1(), Standard_False, aCoords, aNbNodes, aEntType);

    Vector3d aNorm = Vector3d(aCoords.Value(1), aCoords.Value(2), aCoords.Value(3));
    if (!aNorm.Magnitude())
      aNorm = Vector3d(0, 0, 1);
    aDefDS->SetVector(anIter.Key1(), aNorm.Normalized());
  }

  aMesh->SetDataSource(aDefDS);

  anIC->Redisplay(aMesh, Standard_False);

  Handle(ViewWindow) aView = ViewerTest1::CurrentView();
  if (!aView.IsNull())
    aView->FitAll();

  return 0;
}

//=================================================================================================

static Standard_Integer mesh_edge_width(DrawInterpreter& theDI,
                                        Standard_Integer  theNbArgs,
                                        const char**      theArgVec)
{
  try
  {
    OCC_CATCH_SIGNALS
    if (theNbArgs < 3)
    {
      theDI << "Wrong number of parameters\n";
      theDI << "Use : mesh_edge_width <mesh name> <width>\n";
      return 0;
    }

    Handle(MeshVS_Mesh) aMesh = getMesh(theArgVec[1], theDI);
    if (aMesh.IsNull())
    {
      theDI << "Mesh1 not found\n";
      return 0;
    }

    const char* aWidthStr = theArgVec[2];
    if (aWidthStr == 0 || Draw1::Atof(aWidthStr) <= 0)
    {
      theDI << "Width must be real value more than zero\n";
      return 0;
    }

    double aWidth = Draw1::Atof(aWidthStr);

    Handle(VisualContext) anIC = ViewerTest1::GetAISContext();
    if (anIC.IsNull())
    {
      theDI << "The context is null\n";
      return 0;
    }

    Handle(MeshVS_Drawer) aDrawer = aMesh->GetDrawer();
    if (aDrawer.IsNull())
    {
      theDI << "The drawer is null\n";
      return 0;
    }

    aDrawer->SetDouble(MeshVS_DA_EdgeWidth, aWidth);
    anIC->Redisplay(aMesh, Standard_True);
  }
  catch (ExceptionBase const&)
  {
    theDI << "Error\n";
  }

  return 0;
}

//=================================================================================================

static Standard_Integer meshinfo(DrawInterpreter& theDI,
                                 Standard_Integer  theNbArgs,
                                 const char**      theArgVec)
{
  if (theNbArgs != 2)
  {
    theDI << "Wrong number of parameters. Use : meshinfo mesh\n";
    return 0;
  }

  Handle(MeshVS_Mesh) aMesh = getMesh(theArgVec[1], theDI);
  if (aMesh.IsNull())
  {
    theDI << "Mesh1 not found\n";
    return 0;
  }

  Handle(XSDRAWSTL_DataSource) stlMeshSource =
    Handle(XSDRAWSTL_DataSource)::DownCast(aMesh->GetDataSource());
  if (!stlMeshSource.IsNull())
  {
    const TColStd_PackedMapOfInteger& nodes = stlMeshSource->GetAllNodes();
    const TColStd_PackedMapOfInteger& tris  = stlMeshSource->GetAllElements();

    theDI << "Nb nodes = " << nodes.Extent() << "\n";
    theDI << "Nb triangles = " << tris.Extent() << "\n";
  }

  return 0;
}

//=================================================================================================

void XSDRAWSTL1::Factory(DrawInterpreter& theDI)
{
  static Standard_Boolean aIsActivated = Standard_False;
  if (aIsActivated)
  {
    return;
  }
  aIsActivated = Standard_True;

  const char* aGroup = "XSTEP-STL/VRML"; // Step transfer file commands

  theDI.Add("writestl",
            "shape file [ascii/binary (0/1) : 1 by default] [InParallel (0/1) : 0 by default]",
            __FILE__,
            writestl,
            aGroup);
  theDI.Add(
    "readstl",
    "readstl shape file [-brep] [-mergeAngle Angle] [-multi]"
    "\n\t\t: Reads STL file and creates a new shape with specified name."
    "\n\t\t: When -brep is specified, creates a Compound of per-triangle Faces."
    "\n\t\t: Single triangulation-only Face is created otherwise (default)."
    "\n\t\t: -mergeAngle specifies maximum angle in degrees between triangles to merge equal "
    "nodes; disabled by default."
    "\n\t\t: -multi creates a face per solid in multi-domain files; ignored when -brep is set.",
    __FILE__,
    readstl,
    aGroup);

  theDI.Add("meshfromstl", "creates MeshVS_Mesh from STL file", __FILE__, createmesh, aGroup);
  theDI.Add("mesh3delem", "creates 3d element mesh to test", __FILE__, create3d, aGroup);
  theDI.Add("meshshadcolor", "change MeshVS_Mesh shading color", __FILE__, meshcolor, aGroup);
  theDI.Add("meshlinkcolor", "change MeshVS_Mesh line color", __FILE__, linecolor, aGroup);
  theDI.Add("meshmat", "change MeshVS_Mesh material and transparency", __FILE__, meshmat, aGroup);
  theDI.Add("meshshrcoef", "change MeshVS_Mesh shrink coeff", __FILE__, shrink, aGroup);
  theDI.Add("meshclosed",
            "meshclosed meshname (0/1) \nChange MeshVS_Mesh drawing mode. 0 - not closed object, 1 "
            "- closed object",
            __FILE__,
            closed,
            aGroup);
  theDI.Add("meshshow", "display MeshVS_Mesh object", __FILE__, mdisplay, aGroup);
  theDI.Add("meshhide", "erase MeshVS_Mesh object", __FILE__, merase, aGroup);
  theDI.Add("meshhidesel", "hide selected entities", __FILE__, hidesel, aGroup);
  theDI.Add("meshshowsel", "show only selected entities", __FILE__, showonly, aGroup);
  theDI.Add("meshshowall", "show all entities", __FILE__, showall, aGroup);
  theDI.Add("meshcolors", "display color presentation", __FILE__, meshcolors, aGroup);
  theDI.Add("meshvectors", "display sample vectors", __FILE__, meshvectors, aGroup);
  theDI.Add("meshtext", "display text labels", __FILE__, meshtext, aGroup);
  theDI.Add("meshdeform", "display deformed mesh", __FILE__, meshdeform, aGroup);
  theDI.Add("mesh_edge_width", "set width of edges", __FILE__, mesh_edge_width, aGroup);
  theDI.Add("meshinfo", "displays the number of nodes and triangles", __FILE__, meshinfo, aGroup);

  // Load XSDRAW1 session for pilot activation
  XSDRAW1::LoadDraw(theDI);
}

// Declare entry point PLUGINFACTORY
DPLUGIN(XSDRAWSTL1)

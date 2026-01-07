// Created on: 1996-12-18
// Created by: Yves FRICAUD
// Copyright (c) 1996-1999 Matra Datavision
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

#include <BRepBuilderAPI_MakeVertex.hxx>
#include <Standard.hxx>
#include <Standard_ConstructionError.hxx>
#include <Standard_GUID.hxx>
#include <Standard_NoMoreObject.hxx>
#include <Standard_NoSuchObject.hxx>
#include <Standard_NullObject.hxx>
#include <Standard_Type.hxx>
#include <TDF_AttributeDelta.hxx>
#include <TDF_DataSet.hxx>
#include <TDF_DeltaOnAddition.hxx>
#include <TDF_Label.hxx>
#include <TDF_RelocationTable.hxx>
#include <TDF_Tool.hxx>
#include <TNaming_Builder.hxx>
#include <TNaming_CopyShape.hxx>
#include <TNaming_DeltaOnModification.hxx>
#include <TNaming_DeltaOnRemoval.hxx>
#include <TNaming_Iterator.hxx>
#include <TNaming_NamedShape.hxx>
#include <TNaming_NewShapeIterator.hxx>
#include <TNaming_OldShapeIterator.hxx>
#include <TNaming_PtrNode.hxx>
#include <TNaming_RefShape.hxx>
#include <TNaming_SameShapeIterator.hxx>
#include <TNaming_Tool.hxx>
#include <TNaming_UsedShapes.hxx>
#include <TopoDS_Shape.hxx>

IMPLEMENT_STANDARD_RTTIEXT(ShapeAttribute, TDF_Attribute)

// Defines the nodes classes
// #define MDTV_DEB_HASL
//=================================================================================================

const Standard_GUID& ShapeAttribute::GetID()
{
  static Standard_GUID TNaming_NamedShapeID("c4ef4200-568f-11d1-8940-080009dc3333");
  return TNaming_NamedShapeID;
}

//=======================================================================
// class: TNaming_Node
//=======================================================================

class TNaming_Node
{
public:
  TNaming_Node(TNaming_PtrRefShape Old, TNaming_PtrRefShape New)
      : myOld(Old),
        myNew(New),
        myAtt(0L),
        nextSameAttribute(0L),
        nextSameOld(0L),
        nextSameNew(0L)
  {
  }

  // Label : Donne le Label
  DataLabel Label();

  // NextSameShape
  TNaming_Node* NextSameShape(TNaming_RefShape* prs);

  // Test si l evolution est valide dans la transaction Trans
  // ie : Trans n est pas anterieure a sa creation
  //      et Trans n est pas posterieure a son BackUp
  Standard_Boolean IsValidInTrans(Standard_Integer Trans);

  //! Dumps the content of me into the stream
  Standard_EXPORT void DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth = -1) const;

  // Memory management
  DEFINE_STANDARD_ALLOC

  TNaming_PtrRefShape myOld;
  TNaming_PtrRefShape myNew;
  ShapeAttribute* myAtt;
  TNaming_PtrNode     nextSameAttribute;
  TNaming_PtrNode     nextSameOld;
  TNaming_PtrNode     nextSameNew;
};

//=================================================================================================

TNaming_Node* TNaming_Node::NextSameShape(TNaming_RefShape* prs)
{
  if (myOld == prs)
    return nextSameOld;
  if (myNew == prs)
    return nextSameNew;
  return nextSameNew;
}

//=================================================================================================

DataLabel TNaming_Node::Label()
{
  return myAtt->Label();
}

//=================================================================================================

Standard_Boolean TNaming_Node::IsValidInTrans(Standard_Integer Trans)
{
  if (myAtt->Transaction() <= Trans && Trans <= myAtt->UntilTransaction())
  {
    return 1;
  }
  return 0;
}

//=================================================================================================

void TNaming_Node::DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth) const {
  OCCT_DUMP_CLASS_BEGIN(theOStream, TNaming_Node)

    OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, myOld)
      OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, myNew)
        OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, myAtt)

          OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, nextSameAttribute)
            OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, nextSameOld)
              OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, nextSameNew)}

//**********************************************************************
// Methods of ShapeAttribute
//**********************************************************************

//=================================================================================================

ShapeAttribute::ShapeAttribute()
{
  myNode      = 0L;
  myVersion   = 0;
  myEvolution = TNaming_PRIMITIVE;
}

//=================================================================================================

Standard_Boolean ShapeAttribute::IsEmpty() const
{
  Iterator1 it(this);
  return !it.More();
}

//=================================================================================================

TopoShape ShapeAttribute::Get() const
{
  return Tool11::GetShape(this);
}

//=================================================================================================

static void RemoveNode(Standard_Boolean                   MapExist,
                       TNaming_DataMapOfShapePtrRefShape& M,
                       TNaming_Node*&                     N)
{
  TNaming_RefShape* pos = N->myOld;
  if (pos != 0L)
  {
    if (pos->FirstUse() == N)
    {
      TNaming_Node* nextOld = N->nextSameOld;
      if (nextOld != 0L)
        pos->FirstUse(nextOld);
      else
      {
        // le shape disparait
        if (MapExist)
          M.UnBind(pos->Shape());
        N->myOld = 0L;
        if (pos != N->myNew)
        {
          delete pos;
          pos = 0L;
        }
      }
    }
    else
    {
      TNaming_Node* pdn = pos->FirstUse();
      while (pdn != 0L)
      {

        if (pdn->NextSameShape(pos) == N)
        {
          if (pdn->myOld == pos)
            pdn->nextSameOld = N->nextSameOld;
          else
            pdn->nextSameNew = N->nextSameOld;
          break;
        }
        pdn = pdn->NextSameShape(pos);
      }
    }
  }

  TNaming_RefShape* pns = N->myNew;
  if (pns != 0L)
  {
    if (pns->FirstUse() == N)
    {
      TNaming_Node* nextNew = N->nextSameNew;
      if (nextNew != 0L)
        pns->FirstUse(nextNew);
      else
      {
        // le shape disparait
        if (MapExist)
          M.UnBind(pns->Shape());

        pns->FirstUse(0L);
        delete pns;
        pns = 0L;

        N->myNew = 0L;
      }
    }
    else
    {
      TNaming_Node* pdn = pns->FirstUse();
      while (pdn != 0L)
      {
        if (pdn->NextSameShape(pns) == N)
        {
          if (pdn->myOld == pns)
            pdn->nextSameOld = N->nextSameNew;
          else
            pdn->nextSameNew = N->nextSameNew;
          break;
        }
        pdn = pdn->NextSameShape(pns);
      }
    }
  }
}

//=================================================================================================

void ShapeAttribute::Clear()
{
  if (Label().IsNull())
  {
#ifdef OCCT_DEBUG_BUILDER
    std::cout << "attention etat fantomatique" << std::endl;
#endif
    return;
  }

  Handle(TNaming_UsedShapes) US;

  TNaming_DataMapOfShapePtrRefShape* M = NULL;

  // Recuperation de la map si celle-ci n est pas deja detruite.
  // Standard_Boolean MapExist = Ins.FindInRoot(TNaming_UsedShapes::GetID(),US);

  Standard_Boolean MapExist = Label().Root().FindAttribute(TNaming_UsedShapes::GetID(), US);
  if (MapExist)
    M = &(US->Map());

  TNaming_Node* p = myNode;
  while (p != 0L)
  {
    RemoveNode(MapExist, *M, p);
    p = p->nextSameAttribute;
  }

  p = myNode;
  TNaming_Node* q;
  while (p != 0L)
  {
    q = p;
    p = p->nextSameAttribute;
    if (q != 0L)
    {
      delete q;
      q = 0L;
    }
  }

  myNode = 0L;
}

//=================================================================================================

void ShapeAttribute::BeforeRemoval()
{
  Clear();
}

//=======================================================================
// function : BeforeUndo
// purpose  : before application of a TDF_Delta.
//=======================================================================

Standard_Boolean ShapeAttribute::BeforeUndo(const Handle(TDF_AttributeDelta)& /*anAttDelta*/,
                                                const Standard_Boolean /*forceIt*/)
{
  //  if (anAttDelta->IsKind(STANDARD_TYPE(TDF_DeltaOnAddition))) {
  //    anAttDelta->Attribute()->BeforeRemoval();
  //  }
  return Standard_True;
}

//=======================================================================
// function : AfterUndo
// purpose  : After application of a TDF_Delta.
//=======================================================================

Standard_Boolean ShapeAttribute::AfterUndo(const Handle(TDF_AttributeDelta)& anAttDelta,
                                               const Standard_Boolean /*forceIt*/)
{
  if (anAttDelta->IsKind(STANDARD_TYPE(TDF_DeltaOnAddition)))
  {
    Handle(TNaming_UsedShapes) US;

    TNaming_DataMapOfShapePtrRefShape* M = NULL;

    // Recuperation de la map si celle-ci n est pas deja detruite.
    // Standard_Boolean MapExist = Ins.FindInRoot(TNaming_UsedShapes::GetID(),US);

    Standard_Boolean MapExist =
      anAttDelta->Label().Root().FindAttribute(TNaming_UsedShapes::GetID(), US);

    if (MapExist)
      M = &(US->Map());

    TNaming_Node* p = myNode;
    while (p != 0L)
    {
      RemoveNode(MapExist, *M, p);
      p = p->nextSameAttribute;
    }

    p = myNode;
    TNaming_Node* q;
    while (p != 0L)
    {
      q = p;
      p = p->nextSameAttribute;
      if (q != 0L)
      {
        delete q;
        q = 0L;
      }
    }

    myNode = 0L;
  }
  return Standard_True;
}

//=================================================================================================

Handle(TDF_Attribute) ShapeAttribute::BackupCopy() const
{
  // Remarque dans le copy il est important de reporter le noeud de l attribut
  // pour ne pas casser le chemin nextSameShape.

  Handle(ShapeAttribute) Cop = new ShapeAttribute();
  Cop->myNode                    = myNode;
  Cop->myEvolution               = myEvolution;
  Cop->myVersion                 = myVersion;

  // Mise a jour de myAtt sur les noeuds dans l attribut.
  TNaming_Node* CN = Cop->myNode;

  Handle(ShapeAttribute) A = this;
  A->myNode                    = 0L;

  while (CN != 0L)
  {
    CN->myAtt = Cop.operator->();
    CN        = CN->nextSameAttribute;
  }
  return Cop;
}

//=================================================================================================

void ShapeAttribute::Restore(const Handle(TDF_Attribute)& anAttribute)
{
  Clear();

  ShapeAttribute* PAtt = (ShapeAttribute*)anAttribute.operator->();
  myNode                   = PAtt->myNode;
  myEvolution              = PAtt->myEvolution;
  myVersion                = PAtt->myVersion;

  // Mise a jour de myAtt sur les noeuds dans l attribut.
  TNaming_Node* CN = myNode;
  while (CN != 0L)
  {
    CN->myAtt = this;
    CN        = CN->nextSameAttribute;
  }
  PAtt->myNode = 0L; // un noeud est dans un seul attribut.
}

//=================================================================================================

Handle(TDF_DeltaOnModification) ShapeAttribute::DeltaOnModification(
  const Handle(TDF_Attribute)& anOldAttribute) const
{

  return new TNaming_DeltaOnModification(Handle(ShapeAttribute)::DownCast(anOldAttribute));
}

//=================================================================================================

void ShapeAttribute::DeltaOnModification(const Handle(TDF_DeltaOnModification)& aDelta)
{
  aDelta->Apply();
}

//=================================================================================================

Handle(TDF_DeltaOnRemoval) ShapeAttribute::DeltaOnRemoval() const
{
  return new TNaming_DeltaOnRemoval(this);
}

//=================================================================================================

Handle(TDF_Attribute) ShapeAttribute::NewEmpty() const
{
  return new ShapeAttribute();
}

//=================================================================================================

void ShapeAttribute::Paste(const Handle(TDF_Attribute)&       into,
                               const Handle(TDF_RelocationTable)& Tab) const
{
  DataLabel Lab = into->Label();
  if (Lab.IsNull())
  {
    throw Standard_NullObject("ShapeAttribute::Paste");
  }
  TNaming_Builder B(Lab);

  Iterator1 It(this);
  for (; It.More(); It.Next())
  {
    const TopoShape& OS      = It.OldShape();
    const TopoShape& NS      = It.NewShape();
    TNaming_Evolution   aStatus = It.Evolution();

    // Modification_1 24.06.99 (szy)
    TopoShape copOS, copNS;
    if (aStatus != TNaming_PRIMITIVE)
      ShapeCopier::CopyTool(OS, Tab->TransientTable(), copOS);
    else
      copOS.Nullify();
    if (aStatus != TNaming_DELETE)
      ShapeCopier::CopyTool(NS, Tab->TransientTable(), copNS);
    else
      copNS.Nullify();

    switch (aStatus)
    {
      case TNaming_PRIMITIVE: {
        B.Generated(copNS);
        break;
      }
      case TNaming_GENERATED: {
        B.Generated(copOS, copNS);
        break;
      }
      case TNaming_MODIFY: {
        B.Modify(copOS, copNS);
        break;
      }
      case TNaming_DELETE: {
        B.Delete(copOS);
        break;
      }
      case TNaming_SELECTED: {
        B.Select(copNS, copOS);
        break;
      }

      default:
        break;
    }
  }
}

//=================================================================================================

void ShapeAttribute::References(const Handle(TDF_DataSet)& aDataSet) const
{
  // Recherche des dependances.
  // Pour chaque OldShape de l attribut on ajoute au dataSet son label d origine.
  TNaming_Node* Current = myNode;
  while (Current != NULL)
  {
    if (Current->myOld != NULL)
    {
      TNaming_RefShape* prs = Current->myOld;
      TNaming_Node*     pdn = prs->FirstUse();

      while (pdn != NULL)
      {
        if (pdn->myNew == prs && pdn->myAtt->Evolution() != TNaming_SELECTED)
        {
          aDataSet->AddLabel(pdn->Label());
        }
        pdn = pdn->NextSameShape(prs);
      }
    }
    Current = Current->nextSameAttribute;
  }
}

//=================================================================================================

void ShapeAttribute::Add(TNaming_Node*& pdn)
{
  pdn->myAtt = this;
  if (myNode != 0L)
  {
    pdn->nextSameAttribute = myNode;
  }
  myNode = pdn;
}

//=================================================================================================

Standard_OStream& ShapeAttribute::Dump(Standard_OStream& anOS) const
{
  return anOS;
}

//***************************************
//       Fin Class Named_Shape.
//***************************************

//**********************************************************************
// Methods of the TNaming_Builder class
//**********************************************************************

///=================================================================================================

TNaming_Builder::TNaming_Builder(const DataLabel& L)
{
  // Find or Build Map;
  const DataLabel& root = L.Root();
  if (!root.FindAttribute(TNaming_UsedShapes::GetID(), myShapes))
  {
    myShapes = new TNaming_UsedShapes();
    root.AddAttribute(myShapes);
  }

  // Find Or Build Attribute in LIns.
  if (!L.FindAttribute(ShapeAttribute::GetID(), myAtt))
  {
    myAtt = new ShapeAttribute();
    L.AddAttribute(myAtt);
  }
  else
  {
    myAtt->Backup();
    myAtt->Clear();
    myAtt->myVersion++;
  }
}

//=================================================================================================

Handle(ShapeAttribute) TNaming_Builder::NamedShape() const
{
  return myAtt;
}

//=================================================================================================

static void UpdateFirstUseOrNextSameShape(TNaming_RefShape*& prs, TNaming_Node*& pdn)
{
  TNaming_Node* ldn = prs->FirstUse();
  if (ldn == 0L)
  {
    prs->FirstUse(pdn);
  }
  else
  {
    TNaming_Node* cdn = ldn;
    while (cdn != 0L)
    {
      ldn = cdn;
      cdn = cdn->NextSameShape(prs);
      if (ldn == cdn)
      {
        throw Standard_ConstructionError("UpdateFirstUseOrNextSameShape");
        break;
      }
    }
    // boucle interdite et inutile.
    if (ldn != pdn)
    {
      if (ldn->myOld == prs)
        ldn->nextSameOld = pdn;
      if (ldn->myNew == prs)
        ldn->nextSameNew = pdn;
    }
  }
}

//=================================================================================================

void TNaming_Builder::Generated(const TopoShape& newShape)
{
  if (myAtt->myNode == 0L)
    myAtt->myEvolution = TNaming_PRIMITIVE;
  else
  {
    if (myAtt->myEvolution != TNaming_PRIMITIVE)
      throw Standard_ConstructionError("TNaming_Builder : not same evolution");
  }

  TNaming_RefShape* pos = 0L;
  TNaming_RefShape* pns;

  if (myShapes->myMap.IsBound(newShape))
  {
#ifdef OCCT_DEBUG_BUILDER
    std::cout << "TNaming_Builder::Generate : the shape is already in the attribute" << std::endl;
#endif
    pns = myShapes->myMap.ChangeFind(newShape);
    if (pns->FirstUse()->myAtt == myAtt.operator->())
    {
      throw Standard_ConstructionError("TNaming_Builder::Generate");
    }
    TNaming_Node* pdn = new TNaming_Node(pos, pns);
    myAtt->Add(pdn);
    UpdateFirstUseOrNextSameShape(pns, pdn);
  }
  else
  {
    pns               = new TNaming_RefShape(newShape);
    TNaming_Node* pdn = new TNaming_Node(pos, pns);
    pns->FirstUse(pdn);
    myShapes->myMap.Bind(newShape, pns);
    myAtt->Add(pdn);
  }
}

//=================================================================================================

void TNaming_Builder::Delete(const TopoShape& oldShape)
{
  if (myAtt->myNode == 0L)
    myAtt->myEvolution = TNaming_DELETE;
  else
  {
    if (myAtt->myEvolution != TNaming_DELETE)
      throw Standard_ConstructionError("TNaming_Builder : not same evolution");
  }

  TNaming_RefShape* pns;
  TNaming_RefShape* pos;

  if (myShapes->myMap.IsBound(oldShape))
    pos = myShapes->myMap.ChangeFind(oldShape);
  else
  {
#ifdef OCCT_DEBUG_BUILDER
    std::cout << "TNaming_Builder::Delete : the shape is not in the data" << std::endl;
#endif
    pos = new TNaming_RefShape(oldShape);
    myShapes->myMap.Bind(oldShape, pos);
  }

  TopoShape nullShape;
  pns = new TNaming_RefShape(nullShape);
  myShapes->myMap.Bind(nullShape, pns);

  TNaming_Node* pdn = new TNaming_Node(pos, pns);
  myAtt->Add(pdn);
  UpdateFirstUseOrNextSameShape(pos, pdn);
  UpdateFirstUseOrNextSameShape(pns, pdn);
}

//=================================================================================================

void TNaming_Builder::Generated(const TopoShape& oldShape, const TopoShape& newShape)
{
  if (myAtt->myNode == 0L)
    myAtt->myEvolution = TNaming_GENERATED;
  else
  {
    if (myAtt->myEvolution != TNaming_GENERATED)
      throw Standard_ConstructionError("TNaming_Builder : not same evolution");
  }

  if (oldShape.IsSame(newShape))
  {
#ifdef OCCT_DEBUG_BUILDER
    std::cout << "TNaming_Builder::Generate : oldShape IsSame newShape" << std::endl;
#endif
    return;
  }
  TNaming_RefShape* pos;
  if (!myShapes->myMap.IsBound(oldShape))
  {
    pos = new TNaming_RefShape(oldShape);
    myShapes->myMap.Bind(oldShape, pos);
  }
  else
    pos = myShapes->myMap.ChangeFind(oldShape);

  TNaming_RefShape* pns;
  if (!myShapes->myMap.IsBound(newShape))
  {
    pns = new TNaming_RefShape(newShape);
    myShapes->myMap.Bind(newShape, pns);
  }
  else
    pns = myShapes->myMap.ChangeFind(newShape);

  TNaming_Node* pdn = new TNaming_Node(pos, pns);
  myAtt->Add(pdn);
  UpdateFirstUseOrNextSameShape(pos, pdn);
  UpdateFirstUseOrNextSameShape(pns, pdn);
}

//=================================================================================================

void TNaming_Builder::Modify(const TopoShape& oldShape, const TopoShape& newShape)
{
  if (myAtt->myNode == 0L)
    myAtt->myEvolution = TNaming_MODIFY;
  else
  {
    if (myAtt->myEvolution != TNaming_MODIFY)
      throw Standard_ConstructionError("TNaming_Builder : not same evolution");
  }

  if (oldShape.IsSame(newShape))
  {
#ifdef OCCT_DEBUG_BUILDER
    std::cout << "TNaming_Builder::Modify : oldShape IsSame newShape" << std::endl;
#endif
    return;
  }
  TNaming_RefShape* pos;
  if (!myShapes->myMap.IsBound(oldShape))
  {
    pos = new TNaming_RefShape(oldShape);
    myShapes->myMap.Bind(oldShape, pos);
  }
  else
    pos = myShapes->myMap.ChangeFind(oldShape);

  TNaming_RefShape* pns;
  if (!myShapes->myMap.IsBound(newShape))
  {
    pns = new TNaming_RefShape(newShape);
    myShapes->myMap.Bind(newShape, pns);
  }
  else
    pns = myShapes->myMap.ChangeFind(newShape);

  TNaming_Node* pdn = new TNaming_Node(pos, pns);
  myAtt->Add(pdn);
  UpdateFirstUseOrNextSameShape(pos, pdn);
  UpdateFirstUseOrNextSameShape(pns, pdn);
}

//=================================================================================================

void TNaming_Builder::Select(const TopoShape& S, const TopoShape& InS)
{
  if (myAtt->myNode == 0L)
    myAtt->myEvolution = TNaming_SELECTED;
  else
  {
    if (myAtt->myEvolution != TNaming_SELECTED)
      throw Standard_ConstructionError("TNaming_Builder : not same evolution");
  }

  TNaming_RefShape* pos;
  if (!myShapes->myMap.IsBound(InS))
  {
    pos = new TNaming_RefShape(InS);
    myShapes->myMap.Bind(InS, pos);
  }
  else
    pos = myShapes->myMap.ChangeFind(InS);

  TNaming_RefShape* pns;
  if (!myShapes->myMap.IsBound(S))
  {
    pns = new TNaming_RefShape(S);
    myShapes->myMap.Bind(S, pns);
  }
  else
    pns = myShapes->myMap.ChangeFind(S);

  TNaming_Node* pdn = new TNaming_Node(pos, pns);
  myAtt->Add(pdn);
  UpdateFirstUseOrNextSameShape(pos, pdn);
  UpdateFirstUseOrNextSameShape(pns, pdn);
}

//**********************************************************************
// Methods of the Iterator1 class
//**********************************************************************

//=================================================================================================

Iterator1::Iterator1(const Handle(ShapeAttribute)& Att)
    : myTrans(-1)
{
  myNode = Att->myNode;
}

//=================================================================================================

Iterator1::Iterator1(const DataLabel& Lab)
    : myTrans(-1)
{
  Handle(ShapeAttribute) Att;
  if (Lab.FindAttribute(ShapeAttribute::GetID(), Att))
  {
    myNode = Att->myNode;
  }
  else
  {
    myNode = 0L;
  }
}

//=================================================================================================

Iterator1::Iterator1(const DataLabel& Lab, const Standard_Integer Trans)
    : myTrans(Trans)
{
  Handle(TDF_Attribute) Att;
  if (Lab.FindAttribute(ShapeAttribute::GetID(), Trans, Att))
  {
    myNode = Handle(ShapeAttribute)::DownCast(Att)->myNode;
  }
  else
  {
    myNode = 0L;
#ifdef OCCT_DEBUG
    std::cout << "Iterator1 : No Shape for this label" << std::endl;
#endif
  }
}

//=================================================================================================

void Iterator1::Next()
{
  Standard_NoMoreObject_Raise_if(myNode == 0L, "Iterator1::Next");
  myNode = myNode->nextSameAttribute;
}

//=================================================================================================

const TopoShape& Iterator1::OldShape() const
{
  Standard_NoSuchObject_Raise_if(myNode == 0L, "Iterator1::OldShape");
  if (myNode->myOld == 0L)
  {
    static TopoShape NullShape;
    return NullShape;
  }
  return myNode->myOld->Shape();
}

//=================================================================================================

const TopoShape& Iterator1::NewShape() const
{
  Standard_NoSuchObject_Raise_if(myNode == 0L, "Iterator1::NewShape");
  if (myNode->myNew == 0L)
  {
    static TopoShape NullShape;
    return NullShape;
  }
  return myNode->myNew->Shape();
}

//=================================================================================================

Standard_Boolean Iterator1::IsModification() const
{
  Standard_NoSuchObject_Raise_if(myNode == 0L, "Iterator1::IsModification");
  return (myNode->myAtt->myEvolution == TNaming_MODIFY
          || myNode->myAtt->myEvolution == TNaming_DELETE);
}

//=================================================================================================

TNaming_Evolution Iterator1::Evolution() const
{
  Standard_NoSuchObject_Raise_if(myNode == 0L, "Iterator1::IsModification");
  return myNode->myAtt->myEvolution;
}

//**********************************************************************
// Methods of the NewShapeIterator class
//**********************************************************************

//=======================================================================
// function : SelectSameShape
// purpose  : Selectionne le prochain noeud ou le shape est le meme que celui
//           de RS. Old = 0 si il doit etre new dans le noeud a chercher.
//           selection dans la transaction valide.
//           On saute aussi les noeud ou OS = NS;
//=======================================================================

static void SelectSameShape(TNaming_Node*&          myNode,
                            Standard_Boolean        Old,
                            TNaming_RefShape*&      RS,
                            const Standard_Integer& Trans)
{
  TNaming_Node* pdn = myNode;

  while (pdn != 0L)
  {
    Standard_Boolean Valid;
    if (Trans < 0)
      Valid = pdn->myAtt->IsValid();
    else
      Valid = pdn->IsValidInTrans(Trans);

    if (Valid)
    {
      if (Old)
      {
        if (pdn->myOld == RS && pdn->myNew != 0L && pdn->myNew != RS)
        {
          break;
        }
      }
      else
      {
        if (pdn->myNew == RS && pdn->myOld != 0L && pdn->myOld != RS)
        {
          break;
        }
      }
    }
    pdn = pdn->NextSameShape(RS);
  }
  myNode = pdn;
}

//=================================================================================================

NewShapeIterator::NewShapeIterator(const TopoShape&               aShape,
                                                   const Standard_Integer            Trans,
                                                   const Handle(TNaming_UsedShapes)& Shapes)
    : myTrans(Trans)
{
  Standard_NoSuchObject_Raise_if(!Shapes->Map().IsBound(aShape),
                                 "NewShapeIterator::NewShapeIterator aShape");
  TNaming_RefShape* RS = Shapes->Map().ChangeFind(aShape);
  myNode               = RS->FirstUse();
  Standard_Boolean Old(Standard_True);
  SelectSameShape(myNode, Old, RS, myTrans);
}

//=================================================================================================

NewShapeIterator::NewShapeIterator(const TopoShape&    aShape,
                                                   const Standard_Integer Trans,
                                                   const DataLabel&       access)
    : myTrans(Trans)
{
  Handle(TNaming_UsedShapes) Shapes;
  if (access.Root().FindAttribute(TNaming_UsedShapes::GetID(), Shapes))
  {
    Standard_NoSuchObject_Raise_if(!Shapes->Map().IsBound(aShape),
                                   "NewShapeIterator::NewShapeIterator aShape");
    TNaming_RefShape* RS = Shapes->Map().ChangeFind(aShape);
    myNode               = RS->FirstUse();
    Standard_Boolean Old(Standard_True);
    SelectSameShape(myNode, Old, RS, myTrans);
  }
}

//=================================================================================================

NewShapeIterator::NewShapeIterator(const Iterator1& anIterator)
    : myTrans(anIterator.myTrans)
{
  Standard_NoSuchObject_Raise_if(anIterator.myNode == 0L,
                                 "NewShapeIterator::NewShapeIterator");
  myNode               = anIterator.myNode;
  TNaming_RefShape* RS = myNode->myNew;
  if (RS == 0L)
    myNode = 0L; // No descendant
  else
  {
    // il faut repartir de la premiere utilisation.
    myNode = RS->FirstUse();
    Standard_Boolean Old(Standard_True);
    SelectSameShape(myNode, Old, RS, myTrans);
  }
}

//=================================================================================================

NewShapeIterator::NewShapeIterator(const TopoShape&               aShape,
                                                   const Handle(TNaming_UsedShapes)& Shapes)
    : myTrans(-1)
{
  Standard_NoSuchObject_Raise_if(!Shapes->Map().IsBound(aShape),
                                 "NewShapeIterator::NewShapeIterator aShape");
  TNaming_RefShape* RS = Shapes->Map().ChangeFind(aShape);
  myNode               = RS->FirstUse();
  Standard_Boolean Old(Standard_True);
  SelectSameShape(myNode, Old, RS, myTrans);
}

//=================================================================================================

NewShapeIterator::NewShapeIterator(const TopoShape& aShape,
                                                   const DataLabel&    access)
    : myTrans(-1)
{
  Handle(TNaming_UsedShapes) Shapes;
  if (access.Root().FindAttribute(TNaming_UsedShapes::GetID(), Shapes))
  {
    Standard_NoSuchObject_Raise_if(!Shapes->Map().IsBound(aShape),
                                   "NewShapeIterator::NewShapeIterator aShape");
    Standard_Boolean  Old(Standard_True);
    TNaming_RefShape* RS = Shapes->Map().ChangeFind(aShape);
    myNode               = RS->FirstUse();
    SelectSameShape(myNode, Old, RS, myTrans);
  }
}

//=================================================================================================

NewShapeIterator::NewShapeIterator(const NewShapeIterator& anIterator)
    : myTrans(anIterator.myTrans)
{
  Standard_NoSuchObject_Raise_if(anIterator.myNode == 0L,
                                 "NewShapeIterator::NewShapeIterator");
  myNode               = anIterator.myNode;
  TNaming_RefShape* RS = myNode->myNew;
  if (RS == 0L)
    myNode = 0L; // No descendant
  else
  {
    // il faut repartir de la premiere utilisation.
    myNode = RS->FirstUse();
    Standard_Boolean Old(Standard_True);
    SelectSameShape(myNode, Old, RS, myTrans);
  }
}

//=================================================================================================

void NewShapeIterator::Next()
{
  TNaming_RefShape* RS = myNode->myOld;
  myNode               = myNode->NextSameShape(RS);
  Standard_Boolean Old(Standard_True);
  SelectSameShape(myNode, Old, RS, myTrans);
}

//=================================================================================================

DataLabel NewShapeIterator::Label() const
{
  Standard_NoSuchObject_Raise_if(myNode == 0L, "NewShapeIterator::Label");
  return myNode->Label();
}

//=================================================================================================

Handle(ShapeAttribute) NewShapeIterator::NamedShape() const
{
  Standard_NoSuchObject_Raise_if(myNode == 0L, "NewShapeIterator::Label");
  return myNode->myAtt;
}

//=================================================================================================

const TopoShape& NewShapeIterator::Shape() const
{
  Standard_NoSuchObject_Raise_if(myNode == 0L, "NewShapeIterator::Shape");
  return myNode->myNew->Shape();
}

//=================================================================================================

Standard_Boolean NewShapeIterator::IsModification() const
{
  Standard_NoSuchObject_Raise_if(myNode == 0L, "NewShapeIterator::IsModification");

  return (myNode->myAtt->myEvolution == TNaming_MODIFY
          || myNode->myAtt->myEvolution == TNaming_DELETE);
}

//**********************************************************************
// Methods of the OldShapeIterator class
//**********************************************************************
//=================================================================================================

OldShapeIterator::OldShapeIterator(const TopoShape&               aShape,
                                                   const Standard_Integer            Trans,
                                                   const Handle(TNaming_UsedShapes)& Shapes)
    : myTrans(Trans)
{
  Standard_NoSuchObject_Raise_if(!Shapes->Map().IsBound(aShape),
                                 "OldShapeIterator::OldShapeIterator aShape");
  TNaming_RefShape* RS = Shapes->Map().ChangeFind(aShape);
  myNode               = RS->FirstUse();
  Standard_Boolean Old(Standard_False);
  SelectSameShape(myNode, Old, RS, myTrans);
}

//=================================================================================================

OldShapeIterator::OldShapeIterator(const TopoShape&    aShape,
                                                   const Standard_Integer Trans,
                                                   const DataLabel&       access)
    : myTrans(Trans)
{
  Handle(TNaming_UsedShapes) Shapes;
  if (access.Root().FindAttribute(TNaming_UsedShapes::GetID(), Shapes))
  {
    Standard_NoSuchObject_Raise_if(!Shapes->Map().IsBound(aShape),
                                   "OldShapeIterator::OldShapeIterator aShape");
    TNaming_RefShape* RS = Shapes->Map().ChangeFind(aShape);
    myNode               = RS->FirstUse();
    Standard_Boolean Old(Standard_False);
    SelectSameShape(myNode, Old, RS, myTrans);
  }
}

//=================================================================================================

OldShapeIterator::OldShapeIterator(const TopoShape&               aShape,
                                                   const Handle(TNaming_UsedShapes)& Shapes)
    : myTrans(-1)
{
  Standard_NoSuchObject_Raise_if(!Shapes->Map().IsBound(aShape),
                                 "OldShapeIterator::OldShapeIterator aShape");
  TNaming_RefShape* RS = Shapes->Map().ChangeFind(aShape);
  myNode               = RS->FirstUse();
  Standard_Boolean Old(Standard_False);
  SelectSameShape(myNode, Old, RS, myTrans);
}

//=================================================================================================

OldShapeIterator::OldShapeIterator(const TopoShape& aShape,
                                                   const DataLabel&    access)
    : myTrans(-1)
{
  Handle(TNaming_UsedShapes) Shapes;
  if (access.Root().FindAttribute(TNaming_UsedShapes::GetID(), Shapes))
  {
    Standard_NoSuchObject_Raise_if(!Shapes->Map().IsBound(aShape),
                                   "OldShapeIterator::OldShapeIterator aShape");
    TNaming_RefShape* RS = Shapes->Map().ChangeFind(aShape);
    myNode               = RS->FirstUse();
    Standard_Boolean Old(Standard_False);
    SelectSameShape(myNode, Old, RS, myTrans);
  }
}

//=================================================================================================

OldShapeIterator::OldShapeIterator(const Iterator1& anIterator)
    : myTrans(anIterator.myTrans)
{
  Standard_NoSuchObject_Raise_if(anIterator.myNode == 0L,
                                 "OldShapeIterator::OldShapeIterator");
  myNode               = anIterator.myNode;
  TNaming_RefShape* RS = myNode->myNew;
  if (RS == 0L)
    myNode = 0L; // No descendant
  else
  {
    // il faut repartir de la premiere utilisation.
    myNode = RS->FirstUse();
    Standard_Boolean Old(Standard_False);
    SelectSameShape(myNode, Old, RS, myTrans);
  }
}

//=================================================================================================

OldShapeIterator::OldShapeIterator(const OldShapeIterator& anIterator)
    : myTrans(anIterator.myTrans)
{
  Standard_NoSuchObject_Raise_if(anIterator.myNode == 0L,
                                 "OldShapeIterator::OldShapeIterator");
  myNode               = anIterator.myNode;
  TNaming_RefShape* RS = myNode->myOld;
  if (RS == 0L)
    myNode = 0L; // No descendant
  else
  {
    // il faut repartir de la premiere utilisation.
    myNode = RS->FirstUse();
    Standard_Boolean Old(Standard_False);
    SelectSameShape(myNode, Old, RS, myTrans);
  }
}

//=================================================================================================

void OldShapeIterator::Next()
{
  Standard_Boolean  Old = Standard_False;
  TNaming_RefShape* RS  = myNode->myNew;
  myNode                = myNode->NextSameShape(RS);
  SelectSameShape(myNode, Old, RS, myTrans);
}

//=================================================================================================

DataLabel OldShapeIterator::Label() const
{
  if (myNode == 0L)
    throw Standard_NoSuchObject("OldShapeIterator::Label");
  return myNode->Label();
}

//=================================================================================================

Handle(ShapeAttribute) OldShapeIterator::NamedShape() const
{
  if (myNode == 0L)
    throw Standard_NoSuchObject("OldShapeIterator::Label");
  return myNode->myAtt;
}

//=================================================================================================

const TopoShape& OldShapeIterator::Shape() const
{
  if (myNode == 0L)
    throw Standard_NoSuchObject("OldShapeIterator::Shape");
  return myNode->myOld->Shape();
}

//=================================================================================================

Standard_Boolean OldShapeIterator::IsModification() const
{
  Standard_NoSuchObject_Raise_if(myNode == 0L, "OldShapeIterator::IsModification");
  return (myNode->myAtt->myEvolution == TNaming_MODIFY
          || myNode->myAtt->myEvolution == TNaming_DELETE);
}

//**********************************************************************
// Methods of the SameShapeIterator
//**********************************************************************

//=================================================================================================

SameShapeIterator::SameShapeIterator(const TopoShape&               aShape,
                                                     const Handle(TNaming_UsedShapes)& Shapes)
{
  TNaming_RefShape* RS = Shapes->Map().ChangeFind(aShape);
  myNode               = RS->FirstUse();
  myIsNew              = (myNode->myNew == RS);
}

//=================================================================================================

SameShapeIterator::SameShapeIterator(const TopoShape& aShape,
                                                     const DataLabel&    access)
{
  Handle(TNaming_UsedShapes) Shapes;
  if (access.Root().FindAttribute(TNaming_UsedShapes::GetID(), Shapes))
  {
    TNaming_RefShape* RS = Shapes->Map().ChangeFind(aShape);
    myNode               = RS->FirstUse();
    myIsNew              = (myNode->myNew == RS);
  }
}

//=================================================================================================

void SameShapeIterator::Next()
{
  TNaming_RefShape* prs;
  if (myIsNew)
    prs = myNode->myNew;
  else
    prs = myNode->myOld;

  myNode = myNode->NextSameShape(prs);
  if (myNode != 0L)
    myIsNew = (myNode->myNew == prs);
}

//=================================================================================================

DataLabel SameShapeIterator::Label() const
{
  Standard_NoSuchObject_Raise_if(myNode == 0L, "SameShapeIterator::Label");
  return myNode->Label();
}

//**********************************************************************
// Methods of the TNaming_RefShape
//**********************************************************************
//=================================================================================================

DataLabel TNaming_RefShape::Label() const
{
  return myFirstUse->myAtt->Label();
}

//=================================================================================================

Handle(ShapeAttribute) TNaming_RefShape::NamedShape() const
{
  return myFirstUse->myAtt;
}

//=================================================================================================

void TNaming_RefShape::DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth) const
{
  OCCT_DUMP_CLASS_BEGIN(theOStream, ShapeAttribute);

  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, &myShape);
  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, myFirstUse);
}

//**********************************************************************
// Methods of the Tool11 class
//**********************************************************************

//=================================================================================================

Standard_Boolean Tool11::HasLabel(const DataLabel& access, const TopoShape& S)
{
  Handle(TNaming_UsedShapes) US;
  if (access.Root().FindAttribute(TNaming_UsedShapes::GetID(), US))
  {
    return (US->Map().IsBound(S));
  }
#ifdef OCCT_DEBUG_HASL
  std::cout << "##==> Sub-Shape has no Label!" << std::endl;
#endif
  return Standard_False;
}

//=================================================================================================

DataLabel Tool11::Label(const DataLabel&    access,
                              const TopoShape& S,
                              Standard_Integer&   Trans)
{
  Standard_NoSuchObject_Raise_if(!Tool11::HasLabel(access, S), "Tool11::Label");
  Handle(TNaming_UsedShapes) US;
  access.Root().FindAttribute(TNaming_UsedShapes::GetID(), US);
  return Tool11::Label(US, S, Trans);
}

//=======================================================================
// function : IsValidInTrans
// purpose  : un shape est valid tant que l attribut ou il est cree est valid
//=======================================================================

Standard_Integer Tool11::ValidUntil(const DataLabel& access, const TopoShape& S)
{
  Standard_NoSuchObject_Raise_if(!Tool11::HasLabel(access, S), "Tool11::ValidUntil");
  Handle(TNaming_UsedShapes) US;
  access.Root().FindAttribute(TNaming_UsedShapes::GetID(), US);
  return Tool11::ValidUntil(S, US);
}

//=================================================================================================

Standard_Boolean Tool11::HasLabel(const Handle(TNaming_UsedShapes)& Shapes,
                                        const TopoShape&               S)
{
  return (Shapes->Map().IsBound(S));
}

//=================================================================================================

DataLabel Tool11::Label(const Handle(TNaming_UsedShapes)& Shapes,
                              const TopoShape&               S,
                              Standard_Integer&                 Trans)
{
  Standard_NoSuchObject_Raise_if(!Tool11::HasLabel(Shapes, S), "Tool11::Label");
  TNaming_RefShape* prs = Shapes->Map().Find(S);
  TNaming_Node*     pdn = prs->FirstUse();

  while (pdn != 0L && !(pdn->myNew == prs && pdn->myAtt->Evolution() != TNaming_SELECTED))
  {
    pdn = pdn->NextSameShape(prs);
  }
  if (pdn == 0L)
    pdn = prs->FirstUse();

  DataLabel L = pdn->Label();
  Trans       = pdn->myAtt->Transaction();
  return L;
}

//=================================================================================================

Handle(ShapeAttribute) Tool11::NamedShape(const TopoShape& S, const DataLabel& Acces)
{
  Handle(TNaming_UsedShapes) US;
  Acces.Root().FindAttribute(TNaming_UsedShapes::GetID(), US);
  Handle(ShapeAttribute) NS;

  if (!Tool11::HasLabel(US, S))
  {
    return NS;
  }

  TNaming_RefShape* prs = US->Map().Find(S);
  TNaming_Node*     pdn = prs->FirstUse();
  TNaming_Node*     res = 0L;

  while (pdn != 0L)
  {
    if (pdn->myNew == prs && pdn->myAtt->Evolution() != TNaming_SELECTED)
    {
      res = pdn;
      if (pdn->myAtt->Evolution() != TNaming_GENERATED)
      {
        // Les modifications sont privilegiees par rapport au generation.
        // Dans le cas des shapes qui sont a la fois des modifications et des generations
        // faces tangentes.
        break;
      }
    }
    pdn = pdn->NextSameShape(prs);
  }

  if (res == 0L)
    return NS;

  // VERUE EN ATTENDANT DE REVOIR ABORT 03/11/98
  // Protection pour eviter de renvoyer un attribut backuped
  DataLabel Lab = res->Label();
  Lab.FindAttribute(ShapeAttribute::GetID(), NS);
  return NS;
  //  return res->myAtt;
}

//=======================================================================
// function : IsValidInTrans
// purpose  : un shape est valid tant que l attribut ou il est cree est valid
//=======================================================================

Standard_Integer Tool11::ValidUntil(const TopoShape&               S,
                                          const Handle(TNaming_UsedShapes)& US)
{
  Standard_NoSuchObject_Raise_if(!Tool11::HasLabel(US, S), "Tool11::ValidUntil");

  TNaming_RefShape* RS = US->Map().ChangeFind(S);
  Standard_Integer  Cur;
  Standard_Integer  Until = 0;
  TNaming_Node*     Node  = RS->FirstUse();

  while (Node != 0L)
  {
    if (Node->myNew != 0L && Node->myNew == RS)
    {
      Cur = Node->myAtt->UntilTransaction();
      if (Cur > Until)
        Until = Cur;
    }
    Node = Node->NextSameShape(RS);
  }
  return Until;
}

//=================================================================================================

void ShapeAttribute::DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN(theOStream)

  OCCT_DUMP_BASE_CLASS(theOStream, theDepth, TDF_Attribute)

  TNaming_Node* p = myNode;
  if (p != 0L)
  {
    AsciiString1 aLabel;
    TDF_Tool::Entry(myNode->Label(), aLabel);
    OCCT_DUMP_FIELD_VALUE_STRING(theOStream, aLabel)
  }
  OCCT_DUMP_FIELD_VALUE_STRING(theOStream, myEvolution)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, myVersion)
}

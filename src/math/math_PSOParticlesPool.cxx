// Created on: 2014-07-18
// Created by: Alexander Malyshev
// Copyright (c) 2014-2014 OPEN CASCADE SAS
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

#include <math_PSOParticlesPool.hxx>
#include <algorithm>

//=================================================================================================

PSOParticlesPool::PSOParticlesPool(const Standard_Integer theParticlesCount,
                                             const Standard_Integer theDimensionCount)
    : myParticlesPool(1, theParticlesCount),
      myMemory(0,
               theParticlesCount
                   * (theDimensionCount    // Position1
                      + theDimensionCount  // Velocity
                      + theDimensionCount) // BestPosition
                 - 1)
{
  myParticlesCount = theParticlesCount;
  myDimensionCount = theDimensionCount;
  myMemory.Init(0.);
  // Pointers adjusting.
  Standard_Integer aParIdx, aShiftIdx;
  for (aParIdx = 1; aParIdx <= myParticlesCount; ++aParIdx)
  {
    aShiftIdx                             = (theDimensionCount * 3) * (aParIdx - 1);
    myParticlesPool(aParIdx).Position1     = &myMemory(aShiftIdx);
    myParticlesPool(aParIdx).Velocity     = &myMemory(aShiftIdx + theDimensionCount);
    myParticlesPool(aParIdx).BestPosition = &myMemory(aShiftIdx + 2 * theDimensionCount);
  }
}

//=================================================================================================

PSOParticlesPool::~PSOParticlesPool() {}

//=================================================================================================

PSO_Particle1* PSOParticlesPool::GetParticle(const Standard_Integer theIdx)
{
  return &myParticlesPool(theIdx);
}

//=================================================================================================

PSO_Particle1* PSOParticlesPool::GetBestParticle()
{
  return &*std::min_element(myParticlesPool.begin(), myParticlesPool.end());
}

//=================================================================================================

PSO_Particle1* PSOParticlesPool::GetWorstParticle()
{
  return &*std::max_element(myParticlesPool.begin(), myParticlesPool.end());
}
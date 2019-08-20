//
// Copyright 2017 Animal Logic
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include "AL/usdmaya/nodes/ProxyShape.h"
#include "AL/usdmaya/nodes/proxy/PrimFilter.h"
#include "AL/usdmaya/fileio/SchemaPrims.h"

namespace AL {
namespace usdmaya {
namespace nodes {
namespace proxy {

//----------------------------------------------------------------------------------------------------------------------
PrimFilter::PrimFilter(
  const SdfPathVector& previousPrims,
  const std::vector<UsdPrim>& newPrimSet,
  PrimFilterInterface* proxy,
  bool forceImport)
        : m_newPrimSet(newPrimSet), m_transformsToCreate(), m_updatablePrimSet(), m_removedPrimSet()
{
  // copy over original prims
  m_removedPrimSet.assign(previousPrims.begin(), previousPrims.end());
  std::sort(m_removedPrimSet.begin(), m_removedPrimSet.end(),  [](const SdfPath& a, const SdfPath& b){ return b < a; } );

  for(auto it = m_newPrimSet.begin(); it != m_newPrimSet.end(); )
  {
    UsdPrim prim = *it;
    auto lastIt = it;
    ++it;

    SdfPath path = prim.GetPath();

    // check previous prim type (if it exists at all?)
    TfToken type = proxy->getTypeForPath(path);
    TfToken newType = prim.GetTypeName();

    bool supportsUpdate = false;
    bool requiresParent = false;
    bool importableByDefault = false;
    proxy->getTypeInfo(newType, supportsUpdate, requiresParent, importableByDefault);

    if(importableByDefault || forceImport)
    {
      // if the type remains the same, and the type supports update
      if(type == newType)
      {
        if(supportsUpdate)
        {
          TF_DEBUG(ALUSDMAYA_TRANSLATORS).Msg(
                    "PrimFilter::PrimFilter %s prim has not changed type and supports updates or inactive.\n", path.GetText());
          // locate the path and delete from the removed set (we do not want to delete this prim!
          // Note that m_removedPrimSet is reverse sorted
          auto iter = std::lower_bound(m_removedPrimSet.begin(), m_removedPrimSet.end(), path, [](const SdfPath& a, const SdfPath& b){ return b < a; } );
          if(iter != removedPrimSet().end() && *iter == path)
          {
            m_removedPrimSet.erase(iter);
            m_updatablePrimSet.push_back(prim);
            // skip creating transforms in this case.
            requiresParent = false;
          }
        }

        //If the prim is still already there with the same type, it isn't new.
        it = m_newPrimSet.erase(lastIt);
      }
      // if we need a transform, make a note of it now
      if(requiresParent)
      {
        m_transformsToCreate.push_back(prim);
      }
    }
    else
    {
      it = m_newPrimSet.erase(lastIt);
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------
} // proxy
} // nodes
} // usdmaya
} // AL
//----------------------------------------------------------------------------------------------------------------------

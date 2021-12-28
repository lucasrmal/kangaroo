/*
 * Kangaroo Lib, the library of Kangaroo PFM
 * Copyright (C) 2015 Lucas Rioux-Maldague <lucasr.mal@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  US
 */

#include "model/institution-manager.h"
// #include "model/icon.h"

namespace kangaroo::model {

InstitutionManager* InstitutionManager::instance_ = new InstitutionManager();

std::vector<std::string> InstitutionManager::CountriesInUse() const {
  std::unordered_set<std::string> countries;
  for (auto it = objects_.iterator(); it != objects_.iterator_end(); ++it) {
    if (it->second->country().empty()) continue;
    countries.insert(it->second->country());
  }
  return std::vector<std::string>(countries.begin(), countries.end());
}

absl::Status InstitutionManager::Merge(const std::vector<int64_t>& from,
                                       int64_t to) {
  // if (_ids.size() < 1)
  // {
  //     ModelException::throwException(tr("The merge set must be at least 2."),
  //     this); return;
  // }
  // else if (!_ids.contains(_idTo))
  // {
  //     ModelException::throwException(tr("The merge set must include the
  //     destination institution id."), this); return;
  // }

  // //Check if all ids are valid
  // for (int id : _ids)
  // {
  //     get(id);
  // }

  // QSet<int> newSet = _ids;
  // newSet.remove(_idTo); //We don't modify accounts that already have this
  // institution...

  // //Do the merge
  // for (Account* a : Account::getTopLevel()->accounts())
  // {
  //     if (newSet.contains(a->idInstitution()))
  //     {
  //         a->setIdInstitution(_idTo);
  //     }
  // }

  // //Delete the institutions & recompute the index
  // for (int i = 0; i < m_institutions.size(); ++i)
  // {
  //     if (newSet.contains(m_institutions[i]->id()))
  //     {
  //         emit institutionRemoved(m_institutions[i]);
  //         m_institutions[i]->deleteLater();
  //         m_institutions.removeAt(i);

  //         emit modified();
  //     }

  //     m_index[m_institutions[i]->id()] = i;
  // }
  return absl::UnimplementedError("Not Yet Implemented!");
}

absl::Status InstitutionManager::ValidateRemove(const Institution& inst) const {
  // TODO(lucasrmal): Update all accounts with this institution id and set to
  // none.
  return absl::UnimplementedError("Not Yet Implemented!");
}

}  // namespace kangaroo::model

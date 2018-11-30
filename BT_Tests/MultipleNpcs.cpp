#include "MultipleNpcs.h"
#include "../BehaviorTree/BT_Noeud.h"

BT_Noeud::ETAT_ELEMENT MultipleNpcs::execute() noexcept {
   GameManager::Log("MultipleNpcs");
   if (gm.getNpcs().size() > 1) {
      return ETAT_ELEMENT::REUSSI;
   }
   else {
      return ETAT_ELEMENT::ECHEC;
   }
}

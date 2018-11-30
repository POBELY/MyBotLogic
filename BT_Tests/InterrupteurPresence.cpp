#include "InterrupteurPresence.h"
#include "../BehaviorTree/BT_Noeud.h"

BT_Noeud::ETAT_ELEMENT InterrupteurPresence::execute() noexcept {
   GameManager::Log("InterrupteurPresence");
   if (gm.m.getActivateurs().empty()) {
      return ETAT_ELEMENT::ECHEC;
   }
   else {
      return ETAT_ELEMENT::REUSSI;
   }
}

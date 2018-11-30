#include "NoCrossNpc.h"
#include "../BehaviorTree/BT_Noeud.h"

BT_Noeud::ETAT_ELEMENT NoCrossNpc::execute() noexcept {
    GameManager::Log("NoCrossNpc");
    bool cross;
    for (const Npc& npc : gm.getNpcs()) {
       if (npc.goingToCross())
          return ETAT_ELEMENT::ECHEC;
    }
    return ETAT_ELEMENT::REUSSI;
}

#include "Inspection.h"
#include "../BehaviorTree/BT_Noeud.h"
#include <chrono>

BT_Noeud::ETAT_ELEMENT Inspection::execute() noexcept {
   auto pre = std::chrono::high_resolution_clock::now();

   GameManager::Log("Inspection");

   for (auto& npc : gm.getNpcs()) {
      if (gm.m.getTile(npc.getTileId()).inspectable()) {
         gm.m.getInteractObjects().push_back(gm.m.getTile(npc.getTileId()).inspecter());
      }

      else {
         // A FAIRE : Se déplacer vers la case inspectable la plus proche !!!
         npc.getChemin().addFirst(3);
      }

   }


   // Temps d'execution
   auto post = std::chrono::high_resolution_clock::now();
   GameManager::Log("Durée Inspection = " + to_string(std::chrono::duration_cast<std::chrono::microseconds>(post - pre).count() / 1000.f) + "ms");

   return ETAT_ELEMENT::REUSSI;
}

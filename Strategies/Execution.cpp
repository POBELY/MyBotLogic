#include "Execution.h"
#include "../BehaviorTree/BT_Noeud.h"
#include <chrono>

BT_Noeud::ETAT_ELEMENT Execution::execute() noexcept {
   //auto pre = std::chrono::high_resolution_clock::now();

   //GameManager::Log("Execution");

   // Initialiser les attributs au début du tour d'exéction
   gm.tilesAVisiter = {};
   gm.isolatedClosedDoorsToOpen = gm.m.getIsolatedClosedDoorsCopy();
   // Executer tous les arbres des NPCS
   for (Npc& npc : gm.getNpcs()) {
      npc.execute();
   }

   // Temps d'execution
   //auto post = std::chrono::high_resolution_clock::now();
   //GameManager::Log("Durée Execution = " + to_string(std::chrono::duration_cast<std::chrono::microseconds>(post - pre).count() / 1000.f) + "ms");

   return ETAT_ELEMENT::REUSSI;
}

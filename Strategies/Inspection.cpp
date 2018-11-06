#include "Inspection.h"
#include "../BehaviorTree/BT_Noeud.h"
#include <chrono>

BT_Noeud::ETAT_ELEMENT Inspection::execute() noexcept {
   auto pre = std::chrono::high_resolution_clock::now();

   GameManager::Log("Inspection");

   vector<int> isolatedClosedDoors = gm.m.getIsolatedClosedDoors();

   for (auto& npc : gm.getNpcs()) {
      // Si on a pas de porte isolé à ouvrir
      if (isolatedClosedDoors.empty()) {
         // On inspecte les murs
         if (gm.m.getTile(npc.getTileId()).inspectable()) {
            // Inspecter l'objet et l'ajouter aux objets
            int wall2InteractID = gm.m.getTile(npc.getTileId()).inspecter();
            npc.inspectWall(wall2InteractID);
            gm.m.addInteractObject(wall2InteractID);
            GameManager::Log("npc " + to_string(npc.getId()) + " interact with object ");
         }

         else {
            // A FAIRE : Se déplacer vers la case inspectable la plus proche !!!
         }
      } else {
         vector<int> ensembleAccessible = npc.getEnsembleAccessible();
         // Prendre une porte isolée
         // Attention : elle n'est pas garantie accessible !!!
         int objectID = isolatedClosedDoors.back();
         isolatedClosedDoors.pop_back();
         ObjectInfo object = gm.m.getPortes()[objectID];
         // On détermine les tuiles adjacente à la porte
         int goal1ID = object.tileID;
         int goal2ID = gm.m.getAdjacentTileAt(goal1ID, object.position);
         // Si on est devant la porte, on interagit
         if (npc.getTileId() == goal1ID || npc.getTileId() == goal2ID) {
            npc.openDoor(objectID);
            gm.m.addInteractObject(objectID);
         } // Sinon on regarde si ces tuiles sont accessibles et on y va
         else {
            if (find(ensembleAccessible.begin(), ensembleAccessible.end(), goal1ID) != ensembleAccessible.end()) {
               npc.setChemin(gm.m.aStar(npc.getTileId(), goal1ID));
            }
            else if (find(ensembleAccessible.begin(), ensembleAccessible.end(), goal2ID) != ensembleAccessible.end()) {
               npc.setChemin(gm.m.aStar(npc.getTileId(), goal2ID));
            }
            else {
               GameManager::Log("Porte isolée non accessible");
            }
         }

      }

   }


   // Temps d'execution
   auto post = std::chrono::high_resolution_clock::now();
   GameManager::Log("Durée Inspection = " + to_string(std::chrono::duration_cast<std::chrono::microseconds>(post - pre).count() / 1000.f) + "ms");

   return ETAT_ELEMENT::REUSSI;
}

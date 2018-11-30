#include "CurrentGOP.h"
#include "../BehaviorTree/BT_Noeud.h"
#include <chrono>

BT_Noeud::ETAT_ELEMENT CurrentGOP::execute() noexcept {
   // précondition : nbNpcs > 1 && interrupteur.existe()
   auto pre = std::chrono::high_resolution_clock::now();

   GameManager::Log("CurrentGOP");
   
   bool npcOnActivateur = false;
   int activateurID;
   int npcID;
   for (Npc& npc : gm.getNpcs()) {
      if (gm.m.getTile(npc.getTileId()).hadActivateur()) {
         activateurID = gm.m.getTile(npc.getTileId()).getActivateur();
         if (find(gm.interrupteurActive.begin(), gm.interrupteurActive.end(), activateurID) == gm.interrupteurActive.end()) {
            npcID = npc.getId();
            npcOnActivateur = true;
            break;
         }
      }
   }

   if (!npcOnActivateur) {
      auto post = std::chrono::high_resolution_clock::now();
      GameManager::Log("Durée CurrentGOP Echec = " + to_string(std::chrono::duration_cast<std::chrono::microseconds>(post - pre).count() / 1000.f) + "ms");

      return ETAT_ELEMENT::ECHEC;
   }

   ObjectInfo activateur = gm.m.getActivateurs()[activateurID];
   for (auto connectedDoorID : activateur.connectedTo) {
      ObjectInfo door = gm.m.getPortes()[connectedDoorID];
      for (Npc& npc : gm.getNpcs()) {
         if (npc.getId() != npcID) {
            // Si on va traverser
            if (npc.getTileId() == door.tileID) {
               if (npc.goingToCross()) {
                  npc.setChemin(gm.m.aStar(npc.getTileId(), gm.m.getAdjacentTileAt(door.tileID, door.position)));
                  //gm.m.getActivateurs().erase(activateur.objectID);
                  gm.interrupteurActive.push_back(activateurID);
                  npc.setCross(false);
                  break;
               }
            }
            else if (npc.getTileId() == gm.m.getAdjacentTileAt(npc.getTileId(), door.position)) {
               if (npc.goingToCross()) {
                  npc.setChemin(gm.m.aStar(npc.getTileId(), door.tileID));
                  // gm.m.getActivateurs().erase(activateur.objectID);
                  gm.interrupteurActive.push_back(activateurID);
                  npc.setCross(false);
                  break;
               }
            }
            // Sinon on va à la porte
            else {
               // La porte est ouverte, on va vers un cote quelconque de la porte
               npc.setChemin(gm.m.aStar(npc.getTileId(), door.tileID));
               break;
            }
         }
      }
   }

   // Temps d'execution
   auto post = std::chrono::high_resolution_clock::now();
   GameManager::Log("Durée CurrentGOP = " + to_string(std::chrono::duration_cast<std::chrono::microseconds>(post - pre).count() / 1000.f) + "ms");

   return ETAT_ELEMENT::REUSSI;
}

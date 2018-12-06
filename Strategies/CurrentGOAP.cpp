#include "CurrentGOAP.h"
#include "../BehaviorTree/BT_Noeud.h"
#include <chrono>

BT_Noeud::ETAT_ELEMENT CurrentGOAP::execute() noexcept {
   // précondition : nbNpcs > 1 && interrupteur.existe()
   auto pre = std::chrono::high_resolution_clock::now();

   GameManager::Log("CurrentGOAP");

   bool npcOnActivateur = false;
   int activateurID;
   int npcID;

   for (Npc& npc : gm.getNpcs()) {
      npc.getChemin().resetChemin();
      npc.getChemin().addFirst(npc.getTileId());
   }

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
      GameManager::Log("Durée CurrentGOAP Echec = " + to_string(std::chrono::duration_cast<std::chrono::microseconds>(post - pre).count() / 1000.f) + "ms");

      return ETAT_ELEMENT::ECHEC;
   }

   bool traverse = false;
   ObjectInfo activateur = gm.m.getActivateurs()[activateurID];
   for (auto connectedDoorID : activateur.connectedTo) {
      ObjectInfo door = gm.m.getPortes()[connectedDoorID];
      for (Npc& npc : gm.getNpcs()) {
         if (npc.getId() != npcID && npc.goingToCross()) {
            // Si on va traverser
            if (npc.getTileId() == door.tileID) {
               npc.setChemin(gm.m.aStar(npc.getTileId(), gm.m.getAdjacentTileAt(door.tileID, door.position)));
               gm.interrupteurActive.push_back(activateurID);
               npc.setCross(false);
               traverse = true;
               break;
            }
            else if (npc.getTileId() == gm.m.getAdjacentTileAt(door.tileID, door.position)) {
               npc.setChemin(gm.m.aStar(npc.getTileId(), door.tileID));
               gm.interrupteurActive.push_back(activateurID);
               npc.setCross(false);
               traverse = true;
               break;
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
   GameManager::Log("Durée CurrentGOAP = " + to_string(std::chrono::duration_cast<std::chrono::microseconds>(post - pre).count() / 1000.f) + "ms");
   if (traverse) {
      return ETAT_ELEMENT::REUSSI;
   }
   else {
      return ETAT_ELEMENT::ECHEC;
   }
}

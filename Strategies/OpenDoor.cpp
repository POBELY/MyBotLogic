#include "OpenDoor.h"
#include "../BehaviorTree/BT_Noeud.h"
#include <chrono>

BT_Noeud::ETAT_ELEMENT OpenDoor::execute() noexcept {

   GameManager::Log("OpenDoor");

   // On regarde si l'on peut ouvrir une porte
   if (!gm.isolatedClosedDoorsToOpen.empty()) {
      const Flood* ensembleAccessible = npc.getEnsembleAccessible();
      // Prendre une porte isolée si elle est accessible
      for (auto doorID : gm.isolatedClosedDoorsToOpen) {
         ObjectInfo object = gm.m.getPortes()[doorID];
         // On détermine les tuiles adjacente à la porte
         int goal1ID = object.tileID;
         int goal2ID = gm.m.getAdjacentTileAt(goal1ID, object.position);
         // Si on est devant la porte, on interagit
         if (npc.getTileId() == goal1ID || npc.getTileId() == goal2ID) {
            npc.openDoor(doorID);
            gm.m.addInteractObject(doorID);
            gm.isolatedClosedDoorsToOpen.erase(find(gm.isolatedClosedDoorsToOpen.begin(), gm.isolatedClosedDoorsToOpen.end(), doorID));
            GameManager::Log("npc " + to_string(npc.getId()) + " interact with door " + to_string(doorID));
            return ETAT_ELEMENT::REUSSI;
         } // Sinon on regarde si ces tuiles sont accessibles et on y va
         else {
            if (ensembleAccessible->is_flooded(goal1ID)) {
               npc.setChemin(gm.m.aStar(npc.getTileId(), goal1ID));
               gm.tilesAVisiter.push_back(npc.getChemin().getFirst()); // Verifier si first est dir ou suiv !!!
               gm.isolatedClosedDoorsToOpen.erase(find(gm.isolatedClosedDoorsToOpen.begin(), gm.isolatedClosedDoorsToOpen.end(), doorID));
               return ETAT_ELEMENT::REUSSI;
            }
            else if (ensembleAccessible->is_flooded(goal2ID)) {
               npc.setChemin(gm.m.aStar(npc.getTileId(), goal2ID));
               gm.tilesAVisiter.push_back(npc.getChemin().getFirst()); // Verifier si first est dir ou suiv !!!
               gm.isolatedClosedDoorsToOpen.erase(find(gm.isolatedClosedDoorsToOpen.begin(), gm.isolatedClosedDoorsToOpen.end(), doorID));
               return ETAT_ELEMENT::REUSSI;
            }
            else {
               GameManager::Log("Porte isolée " + to_string(doorID) + " non accessible");
            }
         }
      }
   }

   return ETAT_ELEMENT::ECHEC;

}
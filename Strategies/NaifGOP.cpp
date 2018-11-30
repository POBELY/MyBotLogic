#include "NaifGOP.h"
#include "../BehaviorTree/BT_Noeud.h"
#include <chrono>
#include <vector>
#include <algorithm>
using namespace std;

BT_Noeud::ETAT_ELEMENT NaifGOP::execute() noexcept {
   // précondition : nbNpcs > 1 && interrupteur.existe()
   auto pre = std::chrono::high_resolution_clock::now();

   GameManager::Log("NaifGOP");

   vector<int> npcsMove = {};

   // Parcours des activateurs
   for (auto activateur : gm.m.getActivateurs()) {

      if (find(gm.interrupteurActive.begin(), gm.interrupteurActive.end(), activateur.second.objectID) == gm.interrupteurActive.end()) {

         int activateurTileID = activateur.second.tileID;
         set<unsigned int> connectedDoorsID = activateur.second.connectedTo;
         ObjectInfo door;
         // Recherche d'un interrupteur accessible et de sa porte associée
         for (auto connectedDoorID : connectedDoorsID) {
            for (Npc& npc : gm.getNpcs()) {
               // Dire a ce Npc d'aller sur cet interrupteur
               if (npc.getEnsembleAccessible()->is_flooded(activateurTileID)) {

                  door = gm.m.getPortes()[connectedDoorID];
                  npc.setChemin(gm.m.aStar(npc.getTileId(), activateurTileID));
                  npcsMove.push_back(npc.getId());

                  break;

               }
            }
            // Recherche d'un autre Npc qui peut traverser la porte
            for (Npc& npc : gm.getNpcs()) {
               // Si les deux cotés de la portes sont accessibles, on va vers le plus proche
               if (find(npcsMove.begin(), npcsMove.end(), npc.getId()) == npcsMove.end()) {
                  if (npc.getEnsembleAccessible()->is_flooded(door.tileID)) {
                     //      if (npc.getEnsembleAccessible()->is_flooded(gm.m.getAdjacentTileAt(door.tileID, door.position))) {
                     //         if (gm.m.getDistance(npc.getTileId(),door.tileID) < gm.m.getDistance(npc.getTileId(), gm.m.getAdjacentTileAt(door.tileID, door.position))) {
                     //            npc.setChemin(gm.m.aStar(npc.getTileId(), gm.m.getAdjacentTileAt(door.tileID, door.position)));
                     //            npc.setCross(true);
                     //            break;
                     //         }
                     //         else {
                     //            npc.setChemin(gm.m.aStar(npc.getTileId(), door.tileID));
                     //            npc.setCross(true);
                     //            break;
                     //         }
                     //      }
                     //// Si un seul cote est accessible, on va vers celui-ci
                     //      else {
                     npc.setChemin(gm.m.aStar(npc.getTileId(), door.tileID));
                     npc.setCross(true);
                     break;
                     //}
                  }
                  else if (npc.getEnsembleAccessible()->is_flooded(gm.m.getAdjacentTileAt(door.tileID, door.position))) {
                     npc.setChemin(gm.m.aStar(npc.getTileId(), gm.m.getAdjacentTileAt(door.tileID, door.position)));
                     npc.setCross(true);
                     break;
                  }

               }
            }

         }
      }
      
   }



   // Temps d'execution
   auto post = std::chrono::high_resolution_clock::now();
   GameManager::Log("Durée NaifGOP = " + to_string(std::chrono::duration_cast<std::chrono::microseconds>(post - pre).count() / 1000.f) + "ms");

   return ETAT_ELEMENT::REUSSI;
}

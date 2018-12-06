#include "NaifGOAP.h"
#include "../BehaviorTree/BT_Noeud.h"
#include <chrono>
#include <vector>
#include <algorithm>
using namespace std;

BT_Noeud::ETAT_ELEMENT NaifGOAP::execute() noexcept {
   // précondition : nbNpcs > 1 && interrupteur.existe()
   auto pre = std::chrono::high_resolution_clock::now();

   GameManager::Log("NaifGOAP");

   vector<int> npcsMove = {};
   vector<int> npcsForActivateurs = {};

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

               map<unsigned int, ObjectInfo> doors = gm.m.getPortes();

               if (npc.getEnsembleAccessible()->is_flooded(activateurTileID) && find(npcsForActivateurs.begin(), npcsForActivateurs.end(), npc.getId()) == npcsForActivateurs.end() && find(npcsMove.begin(), npcsMove.end(), npc.getId()) == npcsMove.end() && doors.find(connectedDoorID) != doors.end()) {
                  door = gm.m.getPortes()[connectedDoorID];
                  npc.setChemin(gm.m.aStar(npc.getTileId(), activateurTileID));
                  npcsForActivateurs.push_back(npc.getId());

                  break;

               }
            }

            if (npcsForActivateurs.empty()) {
               break;
            }

            // Recherche d'un autre Npc qui peut traverser la porte
            for (Npc& npc : gm.getNpcs()) {
               if (find(npcsForActivateurs.begin(), npcsForActivateurs.end(), npc.getId()) == npcsForActivateurs.end() && find(npcsMove.begin(), npcsMove.end(), npc.getId()) == npcsMove.end()) {
                  if (npc.getEnsembleAccessible()->is_flooded(door.tileID)) {
                     npc.setChemin(gm.m.aStar(npc.getTileId(), door.tileID));
                     npc.setCross(true);
                     npcsMove.push_back(npc.getId());
                     break;
                  }
                  else if (npc.getEnsembleAccessible()->is_flooded(gm.m.getAdjacentTileAt(door.tileID, door.position))) {
                     npc.setChemin(gm.m.aStar(npc.getTileId(), gm.m.getAdjacentTileAt(door.tileID, door.position)));
                     npc.setCross(true);
                     npcsMove.push_back(npc.getId());
                     break;
                  }

               }
            }

         }
      }

   }

   //// Execter l'arbre des Npc non considérés
   gm.tilesAVisiter = {};
   for (Npc& npc : gm.getNpcs()) {
      if (find(npcsForActivateurs.begin(), npcsForActivateurs.end(), npc.getId()) == npcsForActivateurs.end() && find(npcsMove.begin(), npcsMove.end(), npc.getId()) == npcsMove.end()) {
         npc.execute();
      }
   }

   // Temps d'execution
   auto post = std::chrono::high_resolution_clock::now();
   GameManager::Log("Durée NaifGOAP = " + to_string(std::chrono::duration_cast<std::chrono::microseconds>(post - pre).count() / 1000.f) + "ms");

   if (npcsMove.empty()) {
      return ETAT_ELEMENT::ECHEC;
   }
   else {
      return ETAT_ELEMENT::REUSSI;
   }

}

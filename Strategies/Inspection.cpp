#include "Inspection.h"
#include "../BehaviorTree/BT_Noeud.h"
#include <chrono>

BT_Noeud::ETAT_ELEMENT Inspection::execute() noexcept {
   auto pre = std::chrono::high_resolution_clock::now();

   GameManager::Log("Inspection");

   vector<int> isolatedClosedDoors = gm.m.getIsolatedClosedDoors();
   vector<int> tilesAVisiter = {};

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
            npc.resetChemins();

            // Calculer le score de chaque tile pour le npc
            // En même temps on calcul le chemin pour aller à cette tile
            // On stocke ces deux informations dans l'attribut cheminsPossibles du Npc
            auto preCalcul = std::chrono::high_resolution_clock::now();
            calculerScoresTilesPourNpc(npc, tilesAVisiter);
            auto postCalcul = std::chrono::high_resolution_clock::now();
            GameManager::Log("Durée calculerScoresEtCheminsTilesPourNpc = " + to_string(std::chrono::duration_cast<std::chrono::microseconds>(postCalcul - preCalcul).count() / 1000.f) + "ms");


            // Choisir la meilleure tile pour ce npc et lui affecter son chemin
            auto preAffect = std::chrono::high_resolution_clock::now();
            int tileChoisi = npc.affecterMeilleurChemin(gm.m);
            auto postAffect = std::chrono::high_resolution_clock::now();
            GameManager::Log("Durée AffectationChemin = " + to_string(std::chrono::duration_cast<std::chrono::microseconds>(postAffect - preAffect).count() / 1000.f) + "ms");


            // Mettre à jour les tilesAVisiter
            tilesAVisiter.push_back(tileChoisi);
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


void Inspection::saveScore(MapTile tile, Npc& npc, vector<int> tilesAVisiter) noexcept {
   // Precondition : tile.statut != inspectable
   float score = 0;


   // On enregistre le cout, cad la distanc npc-tile
   //score += gm.m.getDistance(npc.getTileId(), tile.getId()) * COEF_DISTANCE_NPC_TILE;
   score += gm.m.distanceHex(npc.getTileId(), tile.getId()) * COEF_DISTANCE_NPC_TILE;

   // On regarde l'intêret de cette tile
   float interetTile = tile.getVoisinsMursNonInspecte().size();
   if (interetTile == 0) return; // Si pas d'intêret, la tile ne nous intéresse pas !
   score += interetTile * COEF_INTERET;
   

                                 // On regarde la distance moyenne de cette tuile aux autres tuiles déjà visités
   if (!tilesAVisiter.empty()) {
      float distanceMoyenneTiles = 0;
      for (auto autre : tilesAVisiter) {
         distanceMoyenneTiles += gm.m.distanceHex(tile.getId(), autre);
      }
      distanceMoyenneTiles /= tilesAVisiter.size();
      score += distanceMoyenneTiles * COEF_DISTANCE_TILE_AUTRE_TILES;
   }

   // Il reste à affecter le score et le chemin au npc
   npc.addScore(tile.getId(), score);
}


void Inspection::calculerScore1Tile(int tileID, Map& m, Npc& npc, const vector<int> tilesAVisiter) {
   MapTile tile = m.getTile(tileID);
   // On ne considère la tile que si on ne la visite pas déjà !
   if ((tile.getStatut() != MapTile::INSPECTEE) && find(tilesAVisiter.begin(), tilesAVisiter.end(), tile.getId()) == tilesAVisiter.end()) {
      saveScore(tile, npc, tilesAVisiter);
   }
}
#include "Inspection.h"
#include "../BehaviorTree/BT_Noeud.h"
#include <chrono>

BT_Noeud::ETAT_ELEMENT Inspection::execute() noexcept {
   auto pre = std::chrono::high_resolution_clock::now();

   GameManager::Log("Inspection");

   // On inspecte les murs
   if (gm.m.getTile(npc.getTileId()).inspectable()) {
      // Inspecter l'objet et l'ajouter aux objets
      int wall2InteractID = gm.m.getTile(npc.getTileId()).inspecter();
      npc.inspectWall(wall2InteractID);
      //npc.setWaiting();
      //gm.addWaitingNpc(npc);
      gm.m.addInteractObject(wall2InteractID);
      gm.tilesAVisiter.push_back(npc.getTileId());
      GameManager::Log("npc " + to_string(npc.getId()) + " interact with object " + to_string(wall2InteractID));
   }
   // Sinon on se déplace vers une case inspectable
   else {
      npc.resetChemins();

      // Calculer le score de chaque tile pour le npc
      // En même temps on calcul le chemin pour aller à cette tile
      // On stocke ces deux informations dans l'attribut cheminsPossibles du Npc
      calculerScoresTiles();

      // Choisir la meilleure tile pour ce npc et lui affecter son chemin
      int tileChoisi = npc.affecterMeilleurChemin(gm.m);

      // Mettre à jour les tilesAVisiter
      gm.tilesAVisiter.push_back(tileChoisi);
   }

   // Temps d'execution
   auto post = std::chrono::high_resolution_clock::now();
   GameManager::Log("Durée Inspection = " + to_string(std::chrono::duration_cast<std::chrono::microseconds>(post - pre).count() / 1000.f) + "ms");

   return ETAT_ELEMENT::REUSSI;
}


void Inspection::saveScore(MapTile tile) noexcept {
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
   if (!gm.tilesAVisiter.empty()) {
      float distanceMoyenneTiles = 0;
      for (auto autre : gm.tilesAVisiter) {
         distanceMoyenneTiles += gm.m.distanceHex(tile.getId(), autre);
      }
      distanceMoyenneTiles /= gm.tilesAVisiter.size();
      score += distanceMoyenneTiles * COEF_DISTANCE_TILE_AUTRE_TILES;
   }

   // Il reste à affecter le score et le chemin au npc
   npc.addScore(tile.getId(), score);
}


void Inspection::calculerScore1Tile(int tileID, Map& m) {
   MapTile tile = m.getTile(tileID);
   // On ne considère la tile que si on ne la visite pas déjà !
   if ((tile.getStatut() != MapTile::INSPECTEE) && find(gm.tilesAVisiter.begin(), gm.tilesAVisiter.end(), tile.getId()) == gm.tilesAVisiter.end()) {
      saveScore(tile);
   }
}
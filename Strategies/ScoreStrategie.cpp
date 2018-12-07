
#include "ScoreStrategie.h"
#include "MyBotLogic/BehaviorTree/BT_Noeud.h"
#include "MyBotLogic/GameManager.h"
#include <chrono>

ScoreStrategie::ScoreStrategie(GameManager& gm, Npc& npc, string nom)
   : gm{ gm }, npc{ npc },
   nom{ nom }
{
}

BT_Noeud::ETAT_ELEMENT ScoreStrategie::execute() noexcept {
   GameManager::Log(nom);
   // On ne sait pas où se trouvent les objectifs !
   // On va les chercher !

   // Pour ça chaque npc va visiter en premier les tuiles avec le plus haut score

   // L'ensemble des tiles que l'on va visiter
   bool movesNoStatics = false;

   npc.resetChemins();

   // Calculer le score de chaque tile pour le npc
   // En même temps on calcul le chemin pour aller à cette tile
   // On stocke ces deux informations dans l'attribut cheminsPossibles du Npc
   calculerScoresTiles();

   // Choisir la meilleure tile pour ce npc et lui affecter son chemin
   int tileChoisi = npc.affecterMeilleurChemin(gm.m);

   // Mettre à jour les tilesAVisiter
   if (tileChoisi != npc.getTileId()) {
      gm.tilesAVisiter.push_back(tileChoisi);
      return ETAT_ELEMENT::REUSSI;
   }

   //GameManager::Log(nom + " Echec");
   return ETAT_ELEMENT::ECHEC;

}

void ScoreStrategie::calculerScore1Tile(int tileID, Map& m) {
   MapTile tile = m.getTile(tileID);
   // On ne considère la tile que si on ne la visite pas déjà !
   if ((tile.getStatut() == MapTile::Statut::CONNU || tile.getStatut() == MapTile::Statut::VISITABLE) && find(gm.tilesAVisiter.begin(), gm.tilesAVisiter.end(), tile.getId()) == gm.tilesAVisiter.end()) {
      saveScore(tile);
   }
}

// Calcul le score de chaque tiles et son chemin pour un npc
// On prend en compte les tilesAVisiter des autres npcs pour que les tiles soient loins les unes des autres
void ScoreStrategie::calculerScoresTiles() noexcept {
   //GameManager::Log("Taille ensemble : " + to_string(npc.getEnsembleAccessible().size()));
   for (auto tileID : npc.getEnsembleAccessible()->tiles()) { // parcours toutes les tiles découvertes par l'ensemble des npcs et qui sont accessibles
      calculerScore1Tile(tileID, gm.m);
   }
}


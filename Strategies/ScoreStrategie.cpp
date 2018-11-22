
#include "ScoreStrategie.h"
#include "MyBotLogic/BehaviorTree/BT_Noeud.h"
#include "MyBotLogic/GameManager.h"
#include <chrono>

ScoreStrategie::ScoreStrategie(GameManager& gm, string nom)
   : gm{ gm },
   nom{ nom }
{
}

BT_Noeud::ETAT_ELEMENT ScoreStrategie::execute() noexcept {
   GameManager::Log(nom);
   // On ne sait pas o� se trouvent les objectifs !
   // On va les chercher !

   // Pour �a chaque npc va visiter en premier les tuiles avec le plus haut score

   // L'ensemble des tiles que l'on va visiter
   vector<int> tilesAVisiter;
   bool movesNoStatics = false;

   for (auto& npc : gm.getNpcs()) {
      npc.resetChemins();

      // Calculer le score de chaque tile pour le npc
      // En m�me temps on calcul le chemin pour aller � cette tile
      // On stocke ces deux informations dans l'attribut cheminsPossibles du Npc
      calculerScoresTilesPourNpc(npc, tilesAVisiter);

      // Choisir la meilleure tile pour ce npc et lui affecter son chemin
      int tileChoisi = npc.affecterMeilleurChemin(gm.m);

      // Mettre � jour les tilesAVisiter
      tilesAVisiter.push_back(tileChoisi);
      if (tileChoisi != npc.getTileId()) {
         movesNoStatics = true;
      }
   }

   if (movesNoStatics) {
      return ETAT_ELEMENT::REUSSI;
   }
   else {
      GameManager::Log(nom + " Echec");
      return ETAT_ELEMENT::ECHEC;
   }
}

void ScoreStrategie::calculerScore1Tile(int tileID, Map& m, Npc& npc, const vector<int> tilesAVisiter) {
   MapTile tile = m.getTile(tileID);
   // On ne consid�re la tile que si on ne la visite pas d�j� !
   if ((tile.getStatut() == MapTile::Statut::CONNU || tile.getStatut() == MapTile::Statut::VISITABLE) && find(tilesAVisiter.begin(), tilesAVisiter.end(), tile.getId()) == tilesAVisiter.end()) {
      saveScore(tile, npc, tilesAVisiter);
   }
}

// Calcul le score de chaque tiles et son chemin pour un npc
// On prend en compte les tilesAVisiter des autres npcs pour que les tiles soient loins les unes des autres
void ScoreStrategie::calculerScoresTilesPourNpc(Npc& npc, vector<int> tilesAVisiter) noexcept {
   //GameManager::Log("Taille ensemble : " + to_string(npc.getEnsembleAccessible().size()));
   for (auto tileID : npc.getEnsembleAccessible()->tiles()) { // parcours toutes les tiles d�couvertes par l'ensemble des npcs et qui sont accessibles
      calculerScore1Tile(tileID, gm.m, npc, tilesAVisiter);
   }
}


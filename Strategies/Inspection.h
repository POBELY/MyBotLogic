#ifndef INSPECTION_H
#define INSPECTION_H

#include "../BehaviorTree/BT_Feuille.h"
#include "../GameManager.h"
#include "ScoreStrategie.h"

class Inspection : public ScoreStrategie {;

public:
   Inspection(GameManager& gm) : ScoreStrategie(gm,"Inspection") {}
   void saveScore(MapTile tile, Npc& npc, vector<int> tilesAVisiter) noexcept;
   void calculerScore1Tile(int tileID, Map& m, Npc& npc, const vector<int> tilesAVisiter);
   ETAT_ELEMENT execute() noexcept override;

   enum { COEF_DISTANCE_NPC_TILE = -12 }; // Il faut que ce soit négatif
   enum { COEF_DISTANCE_TILE_AUTRE_TILES = 12 };
   enum { COEF_INTERET = 1 };
};

#endif
#ifndef INSPECTION_H
#define INSPECTION_H

#include "../BehaviorTree/BT_Feuille.h"
#include "../GameManager.h"
#include "ScoreStrategie.h"

class Inspection : public ScoreStrategie {
public:
   Inspection(GameManager& gm, Npc& npc) : ScoreStrategie(gm, npc, "Inspection") {};
   void saveScore(MapTile tile) noexcept;
   void calculerScore1Tile(int tileID, Map& m);
   ETAT_ELEMENT execute() noexcept override;

   enum { COEF_DISTANCE_NPC_TILE = -12 }; // Il faut que ce soit négatif
   enum { COEF_DISTANCE_TILE_AUTRE_TILES = 1 };
   enum { COEF_INTERET = 1 };
};

#endif
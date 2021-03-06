#ifndef EXPEDITION_H
#define EXPEDITION_H

#include "../BehaviorTree/BT_Feuille.h"
#include "../GameManager.h"
#include "../Strategies/ScoreStrategie.h"

class Expedition : public ScoreStrategie {
public:
   void saveScore(MapTile tile) noexcept;
   float interet(MapTile tile) noexcept;

   enum { COEF_DISTANCE_NPC_TILE = -12 }; // Il faut que ce soit n�gatif
   enum { COEF_DISTANCE_OBJECTIFS_TILE = -12 }; // Il faut que ce soit n�gatif
   enum { COEF_DISTANCE_TILE_AUTRE_TILES = 12 };
   enum { COEF_INTERET = 1 };
   enum { COEF_INTERET_ACCESSIBLE = 2 };
   enum { COEF_INTERET_INACCESSIBLE_MAIS_VISIBLE = 1 };
   enum { COEF_ACTIVATEUR = 1 };
   Expedition(GameManager&, Npc&);
};

#endif
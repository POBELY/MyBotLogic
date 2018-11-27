#ifndef SCORE_STRATEGIE_H
#define SCORE_STRATEGIE_H

#include "MyBotLogic/BehaviorTree/BT_Feuille.h"
#include "MyBotLogic/GameManager.h"

class ScoreStrategie : public BT_Feuille {
protected:
   GameManager & gm;
   Npc& npc;
   string nom;
   virtual void calculerScoresTiles() noexcept;
   virtual void calculerScore1Tile(int tileID, Map& m);

public:
   ScoreStrategie(GameManager&, Npc& npc, string nom);
   virtual ETAT_ELEMENT execute() noexcept override;
   virtual void saveScore(MapTile tile) noexcept = 0;
};

#endif
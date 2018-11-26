#ifndef DEF_HAS_PLAN_TO_FOLLOW_H
#define DEF_HAS_PLAN_TO_FOLLOW_H

#include "MyBotLogic/BehaviorTree/BT_Feuille.h"
#include "../GameManager.h"

class HasPlanToFollow : public BT_Feuille {
   GameManager& gm;
public:
   HasPlanToFollow(GameManager& gm) : gm{ gm } {}


   // On v�rifie si tous les npcs peuvent acc�der � un objectif diff�rent
   ETAT_ELEMENT execute() noexcept override;
};


#endif
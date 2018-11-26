#ifndef DEF_HAS_PLAN_TO_FOLLOW_H
#define DEF_HAS_PLAN_TO_FOLLOW_H

#include "MyBotLogic/BehaviorTree/BT_Feuille.h"
#include "../GameManager.h"

class HasPlanToFollow : public BT_Feuille {
   GameManager& gm;
public:
   HasPlanToFollow(GameManager& gm) : gm{ gm } {}


   // On vérifie si tous les npcs peuvent accéder à un objectif différent
   ETAT_ELEMENT execute() noexcept override;
};


#endif
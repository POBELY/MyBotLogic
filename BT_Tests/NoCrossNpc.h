#ifndef NO_CROSS_NPC_H
#define NO_CROSS_NPC_H

#include "MyBotLogic/BehaviorTree/BT_Feuille.h"
#include "../GameManager.h"

class NoCrossNpc : public BT_Feuille {
   GameManager& gm;
public:
   NoCrossNpc(GameManager& gm) : gm{ gm } {}

   ETAT_ELEMENT execute() noexcept override;
};

#endif
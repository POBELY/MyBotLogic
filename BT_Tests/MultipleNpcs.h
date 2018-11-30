#ifndef MULTIPLE_NPCS_H
#define MULTIPLE_NPCS_H

#include "MyBotLogic/BehaviorTree/BT_Feuille.h"
#include "../GameManager.h"

class MultipleNpcs : public BT_Feuille {
   GameManager& gm;
public:
   MultipleNpcs(GameManager& gm) : gm{ gm } {}

   ETAT_ELEMENT execute() noexcept override;
};

#endif
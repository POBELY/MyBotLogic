#ifndef OBJECTIFS_FOR_ALL_NPCS_H
#define OBJECTIFS_FOR_ALL_NPCS_H

#include "MyBotLogic/BehaviorTree/BT_Feuille.h"
#include "../GameManager.h"

class ObjectifsForAllNpcs : public BT_Feuille {
   GameManager& gm;
public:
   ObjectifsForAllNpcs(GameManager& gm) : gm{ gm } {}

   ETAT_ELEMENT execute() noexcept override;
};

#endif
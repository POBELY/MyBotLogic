#ifndef INTERRUPTEUR_PRESENCE_H
#define INTERRUPTEUR_PRESENCE_H

#include "MyBotLogic/BehaviorTree/BT_Feuille.h"
#include "../GameManager.h"

class InterrupteurPresence : public BT_Feuille {
   GameManager& gm;
public:
   InterrupteurPresence(GameManager& gm) : gm{ gm } {}

   ETAT_ELEMENT execute() noexcept override;
};

#endif
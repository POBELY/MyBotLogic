#ifndef CURRENT_GOAP_H
#define CURRENT_GOAP_H

#include "../BehaviorTree/BT_Feuille.h"
#include "../GameManager.h"

class CurrentGOAP : public BT_Feuille {
   GameManager& gm;

public:
   CurrentGOAP(GameManager& gm) : gm{ gm } {}
    ETAT_ELEMENT execute() noexcept override;
};

#endif
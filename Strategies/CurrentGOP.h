#ifndef CURRENT_GOP_H
#define CURRENT_GOP_H

#include "../BehaviorTree/BT_Feuille.h"
#include "../GameManager.h"

class CurrentGOP : public BT_Feuille {
   GameManager& gm;

public:
   CurrentGOP(GameManager& gm) : gm{ gm } {}
    ETAT_ELEMENT execute() noexcept override;
};

#endif
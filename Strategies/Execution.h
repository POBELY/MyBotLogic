#ifndef EXECUTION_H
#define EXECUTION_H

#include "../BehaviorTree/BT_Feuille.h"
#include "../GameManager.h"

class Execution : public BT_Feuille {
   GameManager& gm;

public:
    Execution(GameManager& gm) : gm{ gm } {}
    ETAT_ELEMENT execute() noexcept override;
};

#endif
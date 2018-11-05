#ifndef INSPECTION_H
#define INSPECTION_H

#include "../BehaviorTree/BT_Feuille.h"
#include "../GameManager.h"

class Inspection : public BT_Feuille {
   GameManager& gm;

public:
   Inspection(GameManager& gm) : gm{ gm } {}
   ETAT_ELEMENT execute() noexcept override;
};

#endif
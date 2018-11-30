#ifndef NAIF_GOP_H
#define NAIF_GOP_H

#include "../BehaviorTree/BT_Feuille.h"
#include "../GameManager.h"

class NaifGOP : public BT_Feuille {
   GameManager& gm;

public:
    NaifGOP(GameManager& gm) : gm{ gm } {}
    ETAT_ELEMENT execute() noexcept override;
};

#endif
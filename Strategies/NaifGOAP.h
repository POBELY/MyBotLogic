#ifndef NAIF_GOAP_H
#define NAIF_GOAP_H

#include "../BehaviorTree/BT_Feuille.h"
#include "../GameManager.h"

class NaifGOAP : public BT_Feuille {
   GameManager& gm;

public:
    NaifGOAP(GameManager& gm) : gm{ gm } {}
    ETAT_ELEMENT execute() noexcept override;
};

#endif
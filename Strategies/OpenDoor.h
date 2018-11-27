#ifndef OPENDOOR_H
#define OPENDOOR_H

#include "../BehaviorTree/BT_Feuille.h"
#include "../GameManager.h"
#include "ScoreStrategie.h"

class OpenDoor : public BT_Feuille {
    GameManager& gm;
public:
    OpenDoor(GameManager& gm) : gm{ gm } {};
    ETAT_ELEMENT execute() noexcept override;
};

#endif
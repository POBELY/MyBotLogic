#ifndef OPENDOOR_H
#define OPENDOOR_H

#include "../BehaviorTree/BT_Feuille.h"
#include "../GameManager.h"
#include "ScoreStrategie.h"

class OpenDoor : public BT_Feuille {
   GameManager& gm;
   Npc& npc;
public:
   OpenDoor(GameManager& gm, Npc& npc) : gm{ gm }, npc{ npc } {};
   ETAT_ELEMENT execute() noexcept override;
};

#endif
#include "HasPlanToFollow.h"

BT_Noeud::ETAT_ELEMENT HasPlanToFollow::execute() noexcept {
    return gm.has_plan() ? ETAT_ELEMENT::REUSSI : ETAT_ELEMENT::ECHEC;
}
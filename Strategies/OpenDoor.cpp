#include "OpenDoor.h"
#include "../BehaviorTree/BT_Noeud.h"
#include <chrono>

BT_Noeud::ETAT_ELEMENT OpenDoor::execute() noexcept {
    auto pre = std::chrono::high_resolution_clock::now();

    GameManager::Log("OpenDoor");

    vector<int> isolatedClosedDoors = gm.m.getIsolatedClosedDoors();
    bool echecOpenDoor = true;

    for (auto& npc : gm.getNpcs()) {
        // On regarde si l'on peut ouvrir une porte
        if (!isolatedClosedDoors.empty()) {
            vector<int> ensembleAccessible = npc.getEnsembleAccessible();
            // Prendre une porte isolée si elle est accessible
            for (auto doorID : isolatedClosedDoors) {
                ObjectInfo object = gm.m.getPortes()[doorID];
                // On détermine les tuiles adjacente à la porte
                int goal1ID = object.tileID;
                int goal2ID = gm.m.getAdjacentTileAt(goal1ID, object.position);
                // Si on est devant la porte, on interagit
                if (npc.getTileId() == goal1ID || npc.getTileId() == goal2ID) {
                    npc.openDoor(doorID);
                    gm.m.addInteractObject(doorID);
                    isolatedClosedDoors.erase(find(isolatedClosedDoors.begin(), isolatedClosedDoors.end(), doorID));
                    GameManager::Log("npc " + to_string(npc.getId()) + " interact with door " + to_string(doorID));
                    echecOpenDoor = false;
                    break;
                } // Sinon on regarde si ces tuiles sont accessibles et on y va
                else {
                    if (find(ensembleAccessible.begin(), ensembleAccessible.end(), goal1ID) != ensembleAccessible.end()) {
                        npc.setChemin(gm.m.aStar(npc.getTileId(), goal1ID));
                        isolatedClosedDoors.erase(find(isolatedClosedDoors.begin(), isolatedClosedDoors.end(), doorID));
                        echecOpenDoor = false;
                        break;
                    }
                    else if (find(ensembleAccessible.begin(), ensembleAccessible.end(), goal2ID) != ensembleAccessible.end()) {
                        npc.setChemin(gm.m.aStar(npc.getTileId(), goal2ID));
                        isolatedClosedDoors.erase(find(isolatedClosedDoors.begin(), isolatedClosedDoors.end(), doorID));
                        echecOpenDoor = false;
                        break;
                    }
                    else {
                        GameManager::Log("Porte isolée " + to_string(doorID) + " non accessible");
                    }
                }
            }
        }
    }


    // Temps d'execution
    auto post = std::chrono::high_resolution_clock::now();
    GameManager::Log("Durée Inspection = " + to_string(std::chrono::duration_cast<std::chrono::microseconds>(post - pre).count() / 1000.f) + "ms");
    if(!echecOpenDoor)
        return ETAT_ELEMENT::REUSSI;
    else return ETAT_ELEMENT::ECHEC;
}
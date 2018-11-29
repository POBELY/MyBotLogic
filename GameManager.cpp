
#include "GameManager.h"
#include "Mouvement.h"

#include "BehaviorTree/BT_Noeud.h"
#include "BehaviorTree/BT_Composite.h"
#include "BehaviorTree/Composite/Sequenceur.h"
#include "BehaviorTree/Composite/Selecteur.h"
#include "BT_Tests/ObjectifsForAllNpcs.h"
#include "BT_Tests/CheminsForAllNpcs.h"
#include "Strategies/Exploitation.h"
#include "Strategies/Execution.h"

#include <algorithm>
#include <tuple>
#include<chrono>
using namespace std;
using namespace std::chrono;

// On initialise notre attribut statique ...
Logger GameManager::logger{};
Logger GameManager::loggerRelease{};

GameManager::GameManager(LevelInfo info) :
	m{Map(info)} ,
    objectifPris{ vector<int>{} }
{
    floods.reserve(info.npcs.size());
    shared_floods_mapping.reserve(info.npcs.size());
    // On récupère l'ensemble des npcs !
    for (auto pair_npc : info.npcs) {
        NPCInfo npc = pair_npc.second;
        npcs.emplace_back(npc);
        floods.push_back(std::make_unique<Flood>(m, npc.tileID));
        shared_floods_mapping.push_back({npc.npcID});
        npcs.back().setEnsembleAccessible(floods.back().get());
    }

	//initialisation des tileGoals
	for (unsigned int tileId : m.getObjectifs()) {
		setNpcsGoalTile(tileId);
	}
    updateFlux();
}

void GameManager::InitializeBehaviorTree() noexcept {
   //  Création du behaviorTree Manager
   ObjectifsForAllNpcs *objectifs = new ObjectifsForAllNpcs(*this);
   CheminsForAllNpcs *chemins = new CheminsForAllNpcs(*this);
   Exploitation *exploitation = new Exploitation(*this);
   Execution *execution = new Execution(*this);

   Sequenceur *sequenceur = new Sequenceur({ objectifs, chemins, exploitation });

   behaviorTreeManager = Selecteur({ sequenceur, execution });

   //Initialisation des BehaviorTree des Npcs
   for (Npc& npc : npcs) {
      npc.initializeBehaviorTree(*this);
   }

   }


vector<Mouvement> GameManager::getAllMouvements() {
   // On va récupérer la liste des mouvements
   vector<Mouvement> mouvements;

   // Pour tous les NPCs, s'il n'y a aucun autre Npc devant eux
   for (auto& npc : npcs) {
      GameManager::Log("NPC = " + to_string(npc.getId()));
      GameManager::Log("chemin = " + npc.getChemin().toString());
      GameManager::Log("case actuelle = " + to_string(npc.getTileId()));

      // Si le npc doit aller quelquepart !!!
      if (!npc.getChemin().empty()) {
         // On récupère la case où il doit aller
         int caseCible = npc.getChemin().getFirst();
         GameManager::Log("case cible = " + to_string(caseCible));


         Tile::ETilePosition direction = m.getDirection(npc.getTileId(), caseCible);
         GameManager::Log(std::string("direction = ") + to_string(direction));

         // On enregistre le mouvement
         mouvements.push_back(Mouvement(npc.getId(), npc.getTileId(), caseCible, direction));

         npc.getChemin().removeFirst(); // On peut supprimer le chemin
      }
      else {
         GameManager::Log("case cible = Ne Bouge Pas");
         // Même si le Npc ne bouge pas, il a quand même un mouvement statique !
         mouvements.push_back(Mouvement(npc.getId(), npc.getTileId(), npc.getTileId(), Tile::ETilePosition::CENTER));
      }
   }
   return mouvements;
}

void GameManager::moveNpcs(vector<Action*>& actionList) noexcept {
   PROFILE_SCOPE("GameManager::moveNpcs");
   // TODO !
   // Il faut réordonner les chemins entre les npcs !
   // Cad que si deux Npcs peuvent échanger leurs objectifs et que cela diminue leurs chemins respectifs, alors il faut le faire !
   reafecterObjectifsSelonDistance();

   openDoors();

   // On récupère tous les mouvements
   vector<Mouvement> mouvements = getAllMouvements();

   // Puis on va l'ordonner pour laisser la priorité à celui qui va le plus loin !
   ordonnerMouvements(mouvements);

   // Puis pour chaque mouvement
   for (auto mouvement : mouvements) {
      Npc& npc = getNpcById(mouvement.getNpcId());
      // On ne prend en compte notre mouvement que s'il compte
      if (mouvement.isNotStopped()) {
         // ET ENFIN ON FAIT BOUGER NOTRE NPC !!!!! <3
         actionList.push_back(new Move(mouvement.getNpcId(), mouvement.getDirection()));
         // ET ON LE FAIT AUSSI BOUGER DANS NOTRE MODELE !!!

         npc.move(mouvement.getDirection(), m);
         // TEST : pour chaque npc qui se déplace sur son objectif à ce tour, alors mettre estArrive à vrai
         if (mouvement.getDirection() != Tile::ETilePosition::CENTER && npc.getTileObjectif() == mouvement.getTileDestination())
            // il faut aussi vérifier si tous les NPC ont un objectif atteignable, donc si on est en mode Exploitation
         {
            npc.setArrived(true);
         }
         else {
            npc.setArrived(false);
         }
      }
      else if (npc.hadInspection()) {
         actionList.push_back(new Interact(npc.getId(), npc.getInteractWall(), Interact::EInteraction::Interaction_SearchHiddenDoor));
      }
      else if (npc.hadOpenDoor()) {
         actionList.push_back(new Interact(npc.getId(), npc.getInteractDoor(), Interact::EInteraction::Interaction_OpenDoor));
      }
   }
}

vector<int> getIndicesMouvementsSurMemeCaseCible(vector<Mouvement>& mouvements, int caseCible) {
   vector<int> indices;
   for (int i = 0; i < mouvements.size(); ++i) {
      if (mouvements[i].getTileDestination() == caseCible) indices.push_back(i);
   }
   return indices;
}

int GameManager::getIndiceMouvementPrioritaire(vector<Mouvement>& mouvements, vector<int> indicesAConsiderer) {
   int indiceMax = indicesAConsiderer[0];
   int distanceMax = getNpcById(mouvements[indicesAConsiderer[0]].getNpcId()).getChemin().distance();
   for (int i = 0; i < indicesAConsiderer.size(); ++i) {
      // Si un mouvement est stationnaire, alors personne n'est autorisé à passer !
      if (!mouvements[indicesAConsiderer[i]].isNotStopped())
         return -1;
      int dist = getNpcById(mouvements[indicesAConsiderer[i]].getNpcId()).getChemin().distance();
      if (dist > distanceMax) {
         indiceMax = i;
         distanceMax = dist;
      }
   }
   return indiceMax;
}

void GameManager::stopNonPrioritaireMouvements(vector<Mouvement>& mouvements, vector<int> indicesMouvementsSurMemeCaseCible, int indiceMouvementPrioritaire, bool& continuer) {
   for (int i = 0; i < indicesMouvementsSurMemeCaseCible.size(); ++i) {
      if (indicesMouvementsSurMemeCaseCible[i] != indiceMouvementPrioritaire) {
         int indice = indicesMouvementsSurMemeCaseCible[i];
         // Si le mouvement n'était pas déjà à l'arrêt alors on a réellement effectué un changement !
         if (mouvements[indice].isNotStopped())
            continuer = true;
         mouvements[indice].stop();
         Npc& npc = getNpcById(mouvements[indice].getNpcId());
         npc.getChemin().resetChemin();
         if (indiceMouvementPrioritaire != -1)
            GameManager::Log("Npc " + to_string(mouvements[indice].getNpcId()) + " a stoppé son mouvement pour laisser la place à Npc " + to_string(mouvements[indiceMouvementPrioritaire].getNpcId()));
         else
            GameManager::Log("Npc " + to_string(mouvements[indice].getNpcId()) + " a stoppé son mouvement car quelqu'un est immobile.");
      }
   }
}

void GameManager::gererCollisionsMemeCaseCible(vector<Mouvement>& mouvements) {
   // Tant que l'on a fait une modification
   bool continuer = true;
   // Pour toutes les cases cibles
   while (continuer) {
      continuer = false;
      for (auto& mouvement : mouvements) {
         // On récupère tous les indices des mouvements qui vont sur cette case
         vector<int> indicesMouvementsSurMemeCaseCible = getIndicesMouvementsSurMemeCaseCible(mouvements, mouvement.getTileDestination());

         // Si ils sont plusieurs à vouloir aller sur cette case
         if (indicesMouvementsSurMemeCaseCible.size() >= 2) {
            // On récupère le mouvement associé au Npc ayant le plus de chemin à faire
            int indiceMouvementPrioritaire = getIndiceMouvementPrioritaire(mouvements, indicesMouvementsSurMemeCaseCible);

            // On passe tous les autres mouvements en Center !
            stopNonPrioritaireMouvements(mouvements, indicesMouvementsSurMemeCaseCible, indiceMouvementPrioritaire, continuer);
         }
      }
   }
}

void GameManager::ordonnerMouvements(vector<Mouvement>& mouvements) noexcept {
   PROFILE_SCOPE("GameManager::ordonnerMouvements");

   // Si deux npcs veulent aller sur la même case, alors celui qui a le plus de chemin à faire passe, et tous les autres restent sur place !
   gererCollisionsMemeCaseCible(mouvements);
}

void GameManager::addNewTiles(TurnInfo ti) noexcept {
   if (m.getNbTilesDecouvertes() < m.getNbTiles()) {
      // pour tous les npcs
      for (auto& npc : ti.npcs) {
         // On regarde les tuiles qu'ils voyent
         for (auto& tileId : npc.second.visibleTiles) {
            // Si ces tuiles n'ont pas été découvertes
            if (m.getTile(tileId).getStatut() == MapTile::INCONNU) {
               // On les setDecouverte
               m.addTile(ti.tiles[tileId],*this);
            }
         }
      }
   }
}

void GameManager::addNewObjects(TurnInfo ti) noexcept {
   // Tous les objets visibles par tous les npcs ...
   for (auto npc : ti.npcs) {
      for (auto objet : npc.second.visibleObjects) {
         // Si on ne connaît pas cet objet on l'ajoute
         if (!m.objectExist(objet) || m.hadInteract(objet)) {
            m.addObject(ti.objects[objet]);
         }
      }
      // Mettre à jour l'état de la tuile du npc
      if (m.getTile(npc.second.tileID).getVoisinsMursNonInspecte().empty()) {
         m.getTile(npc.second.tileID).setStatut(MapTile::INSPECTEE);
      }
      // Mettre a visitable les voisins accessibles connu d'une case visitée
      for (auto voisinID : m.getTile(npc.second.tileID).getVoisinsAccessibles()) {
         if (m.getTile(voisinID).getStatut() == MapTile::Statut::CONNU) {
            m.getTile(voisinID).setStatut(MapTile::Statut::VISITABLE);
         }
      }

   }
   m.viderInteractObjects();
}

void GameManager::updateModel(const TurnInfo &ti) noexcept {
   PROFILE_SCOPE("GM Update Model");

   // On essaye de rajouter les nouvelles tiles !
   addNewTiles(ti);

   // On essaye de rajouter les nouvelles tiles !
   addNewObjects(ti);

   // Mettre à jour les flux de nos NPCs
   updateFlux();
}


Npc& GameManager::getNpcById(int id) {
   auto it = std::find_if(npcs.begin(), npcs.end(), [id](const Npc& npc) {
      return npc.getId() == id;
   });

   if (it == npcs.end()) {
      throw npc_inexistant{};
   }

   return *it;
}
std::vector<Npc>& GameManager::getNpcs() {
   return npcs;
}
void GameManager::addNpc(Npc npc) {
   auto it = std::find_if(npcs.begin(), npcs.end(), [searched = npc](const Npc& npc) {
      return npc.getId() == searched.getId();
   });
   if (it != npcs.end())
      throw npc_deja_existant{};

   npcs.push_back(npc);
}

void GameManager::reafecterObjectifsSelonDistance() {
   PROFILE_SCOPE("GameManager::reafecterObjectifsSelonDistance");
   // Tant que l'on fait des modifications on continue ...
   bool continuer = true;

   while (continuer && npcs.size() > 1) {
      continuer = false;

      // Pour tous les npcs ...
      for (std::size_t npc_i = 0; npc_i < npcs.size(); ++npc_i) {
         Npc& npc = npcs[npc_i];
         for (std::size_t npc_j = npc_i + 1; npc_j < npcs.size(); ++npc_j) {
            Npc& autreNpc = npcs[npc_j];

            const int objectifNpc = npc.getShortTermObjectif();
            const int objectifAutreNpc = autreNpc.getShortTermObjectif();

            //const int tempsMaxChemins = std::max(npc.getChemin().distance(), autreNpc.getChemin().distance());
            const int tempsMaxChemins = std::max(m.getDistance(npc.getTileId(), objectifNpc), m.getDistance(autreNpc.getTileId(), objectifAutreNpc));

            // Si l'interversion des objectifs est bénéfique pour l'un deux et ne coûte rien à l'autre (ou lui est aussi bénéfique)
            if (npc.isAccessibleTile(objectifAutreNpc)
               && autreNpc.isAccessibleTile(objectifNpc)) {
               if (std::max(m.getDistance(npc.getTileId(), objectifAutreNpc),
                  m.getDistance(autreNpc.getTileId(), objectifNpc)) < tempsMaxChemins) {// Ensuite que c'est rentable
                  EventProfiler::instance().register_instant_event("swap objectives");
                  GameManager::Log("Npc " + to_string(npc.getId()) + " et Npc " + to_string(autreNpc.getId()) + " échangent leurs objectifs !");
                  npc.setChemin(m.aStar(npc.getTileId(), objectifAutreNpc));
                  autreNpc.setChemin(m.aStar(autreNpc.getTileId(), objectifNpc));
                  continuer = true; // Et on devra continuer pour vérifier que cette intervertion n'en a pas entrainé de nouvelles !
               }
            }
         }
      }
   }
}

void GameManager::unmerge_floods() {
    PROFILE_SCOPE("unmerge floods");
    
    for (std::vector<unsigned int>& npc_sharing_flood : shared_floods_mapping) {
        vector<int> toDemerge;
        int ind = 0;

        Npc& owning_npc = getNpcById(npc_sharing_flood.front());

        // Pour tous les NPC qui partagent ce même flood
        for (unsigned int sharing_npc_id : npc_sharing_flood) {
            Npc& sharing_npc = getNpcById(sharing_npc_id);

            const bool is_npc_owning_flood = sharing_npc.getId() == owning_npc.getId();
            const bool is_npc_sharing_flood = !is_npc_owning_flood;
            const bool does_path_exists_between_owning_npc_and_shared = m.aStar(owning_npc.getTileId(),
                                                                                sharing_npc.getTileId()).isAccessible();
            const bool no_path_to_owner = !does_path_exists_between_owning_npc_and_shared;
            const bool should_unmerge = is_npc_sharing_flood && no_path_to_owner;
            
            // Ce NPC ne fait plus parti du flood qu'il partageait
            if (should_unmerge) {
                // Crée un nouveau flood pour cet individu
                shared_floods_mapping.push_back({ sharing_npc_id });
                floods.push_back(std::make_unique<Flood>(m, sharing_npc.getTileId()));
                sharing_npc.setEnsembleAccessible(floods.back().get());

                // Marque l'individu pour qu'il soit retiré plus tard
                toDemerge.push_back(ind);
            }
            ++ind;
        }
        // On inverse toDemerge pour supprimer nos éléments depuis la fin
        std::reverse(toDemerge.begin(), toDemerge.end());

        // On retire les npc qui ne partagent plus ce flood
        for (auto pos : toDemerge) {
            npc_sharing_flood.erase(npc_sharing_flood.begin() + pos);
        }

        // Réinitialse le flood original
        if (!toDemerge.empty()) {
            for (unsigned int ncp_id : npc_sharing_flood) {
                owning_npc.getEnsembleAccessible()->reset(owning_npc.getTileId());
            }
        }
    }
}

void GameManager::merge_floods() {
    PROFILE_SCOPE("merge floods");
    vector<int> toErase = {};

    for (int i = 0; i < shared_floods_mapping.size() - 1; ++i) {
        const auto to_erase_iterator = find(toErase.begin(), toErase.end(), i);

        // Si i n'est pas à effacer
        if (to_erase_iterator == toErase.end()) {
            for (int j = i + 1; j < shared_floods_mapping.size(); ++j) {
                Npc& owner_flood_i = getNpcById(shared_floods_mapping[i].front());
                Npc& owner_flood_j = getNpcById(shared_floods_mapping[j].front());

                // Si les 2 floods se touchent
                if(owner_flood_i.getEnsembleAccessible()->intersects(*owner_flood_j.getEnsembleAccessible())) {
                    toErase.push_back(j);

                    // Tous les NPC dans j utilisent le flood i dorénavant
                    for (unsigned int npc_id : shared_floods_mapping[j]) {
                        Npc& sharing_npc = getNpcById(npc_id);
                        sharing_npc.setEnsembleAccessible(owner_flood_i.getEnsembleAccessible());
                        shared_floods_mapping[i].push_back(npc_id);
                    }
                }
            }
        }
    }

    // Supprimer les flux n'existant plus
    std::reverse(toErase.begin(), toErase.end());
    for (auto i : toErase) {
         shared_floods_mapping.erase(shared_floods_mapping.begin() + i);
         floods.erase(floods.begin() + i);
    }
}

void GameManager::grow_floods() {
    PROFILE_SCOPE("grow floods");

    for (const std::vector<unsigned int>& shared_flood_group : shared_floods_mapping) {
        Npc& flood_owner = getNpcById(shared_flood_group.front());

        flood_owner.floodfill(m);
    }
}

void GameManager::init_floods() {
   PROFILE_SCOPE("init floods");

   for (auto& npc : npcs) {
      npc.getEnsembleAccessible()->reset(npc.getTileId());
   }

   m.quitButton = false;
}

void GameManager::updateFlux() noexcept {
    PROFILE_SCOPE("updateFlux");

    if (m.quitButton) init_floods();
    unmerge_floods();
    grow_floods();
    merge_floods();
}

bool GameManager::isDoorAdjacente(int interrupteurID) {
   ObjectInfo interrupteur = m.getActivateurs()[interrupteurID];
   set<unsigned int> doorsID = interrupteur.connectedTo;
   for (auto doorID : doorsID) {
      // Si une porte est adjacente à notre interrupteur
      if (m.getPortes()[doorID].tileID == interrupteur.tileID || m.getAdjacentTileAt(m.getPortes()[doorID].tileID, m.getPortes()[doorID].position) == interrupteur.tileID) {
         return true;
      }
   }
   return false;
}

void GameManager::setNpcsGoalTile(unsigned int goalTileId)
{
	auto it = std::min_element(getNpcs().begin(), getNpcs().end(), [this, goalTileId](const Npc& a, const Npc& b) {
		if (a.getTileGoal() != -1) return false;
		if (b.getTileGoal() != -1) return true;

		return m.distanceHex(a.getTileId(), goalTileId) < m.distanceHex(b.getTileId(), goalTileId);
	});

	it->setTileGoal(goalTileId);
}
void GameManager::openDoors() {
   for (Npc& npc : npcs) {
      MapTile& npcTile = m.getTile(npc.getTileId());
      if (npcTile.hadisolatedClosedDoors()) {
         int doorID = npcTile.getIsolatedClosedDoor();
         npc.openDoor(doorID);
         m.addInteractObject(doorID);
         // Suprime la porte de la tile courante
         npcTile.removeIsolatedClosedDoor(doorID);
         // Suprimer la porte de la seconde tile à laquelle elle est voisine
         ObjectInfo door = m.getPortes()[doorID];
         int tiledoorID = door.tileID;
         if (door.tileID == npc.getTileId()) {
            tiledoorID = m.getAdjacentTileAt(door.tileID, door.position);
         }
         m.getTile(tiledoorID).removeIsolatedClosedDoor(doorID);
         // Supprime la porte du modèle
         m.getIsolatedClosedDoors().erase(find(m.getIsolatedClosedDoors().begin(), m.getIsolatedClosedDoors().end(), doorID)); // Supprimer la porte des portes isolés du modèle
         // Suprimer les mouvements du Npc
         npc.getChemin().resetChemin();
         npc.getChemin().addFirst(npc.getTileId());

         GameManager::Log("npc " + to_string(npc.getId()) + " interact with door " + to_string(doorID));

      }
   }
}

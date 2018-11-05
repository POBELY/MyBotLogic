
#include "GameManager.h"
#include "Mouvement.h"

#include "BehaviorTree/BT_Noeud.h"
#include "BehaviorTree/BT_Composite.h"
#include "BehaviorTree/Composite/Sequenceur.h"
#include "BehaviorTree/Composite/Selecteur.h"
#include "BT_Tests/ObjectifsForAllNpcs.h"
#include "BT_Tests/CheminsForAllNpcs.h"
#include "Strategies/Expedition.h"
#include "Strategies/Exploration.h"
#include "Strategies/Exploitation.h"
#include "Strategies/Inspection.h"

#include <algorithm>
#include <tuple>
#include<chrono>
using namespace std;
using namespace std::chrono;

// On initialise notre attribut statique ...
Logger GameManager::logger{};
Logger GameManager::loggerRelease{};

GameManager::GameManager(LevelInfo info) :
   m{ Map(info) },
   objectifPris{ vector<int>{} }
{
   // On récupère l'ensemble des npcs !
   for (auto pair_npc : info.npcs) {
      NPCInfo npc = pair_npc.second;
      npcs.emplace_back(npc);
      flux.push_back({ npc.npcID }); // Initialisation flux
   }

   updateFlux();

}

void GameManager::InitializeBehaviorTree() noexcept {
   //  Création du behaviorTree Manager
   ObjectifsForAllNpcs *objectifs = new ObjectifsForAllNpcs(*this);
   CheminsForAllNpcs *chemins = new CheminsForAllNpcs(*this);
   Exploitation *exploitation = new Exploitation(*this);
   ScoreStrategie *expedition = new Expedition(*this, "Expedition");
   ScoreStrategie *exploration = new Exploration(*this, "Exploration");
   Inspection *inspection = new Inspection(*this);

   Sequenceur *sequenceur1 = new Sequenceur({ chemins, exploitation });

   Selecteur *selecteur = new Selecteur({ sequenceur1, expedition, inspection });

   Sequenceur *sequenceur2 = new Sequenceur({ objectifs, selecteur });

   behaviorTreeManager = Selecteur({ sequenceur2, exploration, inspection });
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
      else if (m.getTile(npc.getTileId()).hadInspection()) {
         actionList.push_back(new Interact(npc.getId(), m.getTile(npc.getTileId()).getInspection(), Interact::EInteraction::Interaction_SearchHiddenDoor));
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
               m.addTile(ti.tiles[tileId]);
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
         if (!m.objectExist(objet)) {
            m.addObject(ti.objects[objet]);
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

void GameManager::updateFlux() noexcept {
   PROFILE_SCOPE("updateFlux");

   // Demerger nos Flux : A TESTER
   vector<int> toDemerge;
   int ind;
   for (auto &npcsID : flux) {
      toDemerge = {};
      ind = 0;
      for (auto &npcID : npcsID) {
         if (npcsID[0] != npcID && !m.aStar(getNpcById(npcsID[0]).getTileId(), getNpcById(npcID).getTileId()).isAccessible()) {
            flux.push_back({ npcID });
            // ReDef ensembleAccess
            getNpcById(npcID).setEnsembleAccessible({ getNpcById(npcID).getTileId() });
            toDemerge.push_back(ind);
         }
         ++ind;
      }
      std::reverse(toDemerge.begin(), toDemerge.end());
      for (auto pos : toDemerge) {
         npcsID.erase(npcsID.begin() + pos);
      }
   }

   // Mettre à jour nos Flux
   for (auto npcsID : flux) {
      vector<int> flood = getNpcById(npcsID[0]).floodfill(m);
      // On copie ce flux pour tous ceux partageant le même flux
      for (auto npcID : npcsID) {
         getNpcById(npcID).setEnsembleAccessible(flood);
      }
   }

   // Trouver les flux communs
   vector<int> toErase = {};
   for (int i = 0; i < flux.size() - 1; ++i) {
      if (find(toErase.begin(), toErase.end(), i) == toErase.end()) {
         for (int j = i + 1; j < flux.size(); ++j) {
            // non 0 mais indice de tuile visite ou visitable !!! A FAIRE

            vector<int> ensembleAccessible_i = getNpcById(flux[i][0]).getEnsembleAccessible();
            vector<int> ensembleAccessible_j = getNpcById(flux[j][0]).getEnsembleAccessible();
            int indice_tuile = 0;
            // Recherché une tuile visite ou visitable de notre flux
            for (auto tileID : ensembleAccessible_j) {
               if (m.getTile(tileID).getStatut() == MapTile::VISITE || m.getTile(tileID).getStatut() == MapTile::VISITABLE) {
                  indice_tuile = tileID;
                  break;
               }
            }
            // Si cette tuile appartient à un autre flux, alors ces flux sont identiques
            if (find(ensembleAccessible_i.begin(), ensembleAccessible_i.end(), indice_tuile) != ensembleAccessible_i.end()) {
               toErase.push_back(j);
               // Rejoindre un flux partagé
               for (auto f : flux[j]) {
                  flux[i].push_back(f);
               }
            }
         }
      }
   }
   // Supprimer les flux n'existant plus
   std::reverse(toErase.begin(), toErase.end());
   for (auto i : toErase) {
      flux.erase(flux.begin() + i);
   }
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

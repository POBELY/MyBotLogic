
#include "Npc.h"
#include "Globals.h"
#include "GameManager.h"
#include "Strategies/Inspection.h"
#include "Strategies/Exploration.h"
#include "BehaviorTree/Composite/Sequenceur.h"
#include <chrono>
#include <algorithm>
#include <cassert>

Npc::Npc(const NPCInfo info) :
   id{ static_cast<int>(info.npcID) },
   tileId{ static_cast<int>(info.tileID) },
   tileObjectif{ -1 },
   chemin{ Chemin{} },
   associated_flood{nullptr},
   estArrive{ false }
{
}

void Npc::initializeBehaviorTree(GameManager& gm) noexcept {
   //  Création du behaviorTree Manager 
   ScoreStrategie *exploration = new Exploration(gm, *this);
   ScoreStrategie *inspection = new Inspection(gm, *this);
   behaviorTreeNpc = Selecteur({ exploration, inspection });
}

void Npc::move(Tile::ETilePosition direction, Map &m) noexcept {
   if (m.getTile(tileId).hadActivateur() && direction != Tile::ETilePosition::CENTER) m.quitButton = true;
   tileId = m.getAdjacentTileAt(tileId, direction);
   m.getTile(tileId).setStatut(MapTile::Statut::VISITE);
}

void Npc::resetChemins() noexcept {
   scoresAssocies.clear();
}

void Npc::addScore(int tileIndice, float score) noexcept {
   scoresAssocies[tileIndice] = score;
}

void testBestScore(pair<const int, float> pair, float& bestScore, int& bestScoreIndice) {
   int tileId = pair.first;
   float score = pair.second;
   //GameManager::Log("Case potentielle à explorer : " + to_string(tileId) + " de score " + to_string(score));
   if (score > bestScore) {
      bestScore = score;
      bestScoreIndice = tileId;
   }
}

int Npc::affecterMeilleurChemin(Map &m) noexcept {
   if (scoresAssocies.empty()) {
      // Dans ce cas-là on reste sur place !
      chemin = Chemin{};
      //GameManager::Log("Le Npc " + to_string(id) + " n'a rien a rechercher et reste sur place !");
      return tileId;
   }

   // On cherche le meilleur score
   //auto preScore = std::chrono::high_resolution_clock::now();
   float bestScore = scoresAssocies.begin()->second;
   int bestScoreIndice = scoresAssocies.begin()->first;
   for (auto pair : scoresAssocies) {
      testBestScore(pair, bestScore, bestScoreIndice);
   }
   //auto postScore = std::chrono::high_resolution_clock::now();
   //GameManager::Log("Durée chercher meilleur score = " + to_string(std::chrono::duration_cast<std::chrono::microseconds>(postScore - preScore).count() / 1000.f) + "ms");

   // On affecte son chemin, mais il nous faut le calculer ! =)
   //auto preAStar = std::chrono::high_resolution_clock::now();
   chemin = m.aStar(tileId, bestScoreIndice);
   //auto postAStar = std::chrono::high_resolution_clock::now();
   //GameManager::Log("Le Npc " + to_string(id) + " va rechercher la tile " + to_string(chemin.destination()));
   //GameManager::Log("Durée a* = " + to_string(std::chrono::duration_cast<std::chrono::microseconds>(postAStar - preAStar).count() / 1000.f) + "ms");

   // On renvoie la destination
   return chemin.destination();
}

void ajoutIfUnkown(Map &m, int voisin, const vector<int>& oldOpen, const vector<int>& Open, vector<int>& newOpen) {
   // Si elle est connu
   if (m.getTile(voisin).existe()) {
      // Si elle n'est pas déjà ajouté
      if (find(oldOpen.begin(), oldOpen.end(), voisin) == oldOpen.end() 
       && find(Open.begin(), Open.end(), voisin) == Open.end()) {
         // On l'ajoute comme nouvelle tuile ouverte
         newOpen.push_back(voisin);
      }
   }
}

void addNewVoisins(Map &m, int tileID, const vector<int>& oldOpen, vector<int>& Open, vector<int>& newOpen) {
   //PROFILE_SCOPE("addNewVoisins");
   for (int voisin : m.getTile(tileID).getVoisinsAccessibles()) {
      ajoutIfUnkown(m, voisin, oldOpen, Open, newOpen);
   }
   // On définit les dernières tuiles ajoutés avec leur coût courant
   if (find(Open.begin(), Open.end(), tileID) == Open.end()) {
      Open.push_back(tileID);
   }
}

void parcourirNewVoisins(Map &m, int tileID, vector<int>& oldOpen, vector<int>& Open, vector<int>& newOpen) {
   //PROFILE_SCOPE("parcourirNewVoisins");
   oldOpen.swap(newOpen);
   newOpen.clear();
   // On regarde les voisins des dernieres tuiles ajoutées
   for (int tileID : oldOpen) {
      addNewVoisins(m, tileID, oldOpen, Open, newOpen);
   }
}

void Npc::floodfill(Map &m) {
    //PROFILE_SCOPE("floodfill");
#if 1
    associated_flood->reduce_to_accessibles(m);
    associated_flood->grow(m);
#else
    vector<int> Open;
    vector<int> oldOpen;
    vector<int> newOpen;

    Open.reserve(m.getTailleTotal());
    oldOpen.reserve(m.getTailleTotal());
    newOpen.reserve(m.getTailleTotal());

    // Initialisation newOpen aux cases accessibles
    std::copy_if(ensembleAccessible.begin(),
        ensembleAccessible.end(),
        std::back_inserter(newOpen),
        [&m](const int tile_id) { return m.getTile(tile_id).isAccessible(); });

    // Tant qu'il reste des noeuds à traiter ...
    while (!newOpen.empty()) {
        parcourirNewVoisins(m, tileId, oldOpen, Open, newOpen);
    }

    // On met à jour l'ensemble et les distances accessible d'un NPC
    ensembleAccessible = std::move(Open);
    return ensembleAccessible;
#endif
}

void Npc::inspectWall(int wallID) {
   interactWall = wallID;
}

void Npc::openDoor(int doorID) {
   interactDoor = doorID;
}

bool Npc::hadInspection() const noexcept {
   return interactWall != -1;
}

bool Npc::hadOpenDoor() const noexcept {
   return interactDoor != -1;
}

int Npc::getId() const noexcept {
   return id;
}

int Npc::getTileId() const noexcept {
   return tileId;
}

int Npc::getShortTermObjectif() const noexcept {
   return getChemin().empty() ? getTileId() : getChemin().destination();
}

int Npc::getTileObjectif() const noexcept {
   return tileObjectif;
}

int Npc::getInteractWall() noexcept {
   int res = interactWall;
   interactWall = -1;
   return res;
}

int Npc::getInteractDoor() noexcept {
   int res = interactDoor;
   interactDoor = -1;
   return res;
}

void Npc::setTileObjectif(int idTile) {
   tileObjectif = idTile;
}

void Npc::setTileGoal(int idTile) {
	tileGoal = idTile;
}

int Npc::getTileGoal() const
{
	return tileGoal;
}

const Chemin& Npc::getChemin() const noexcept {
   return chemin;
}

Chemin& Npc::getChemin() noexcept {
   return chemin;
}

void Npc::setChemin(const Chemin& chemin) {
   this->chemin = chemin;
}

void Npc::setChemin(Chemin&& chemin) {
   this->chemin = std::move(chemin);
}

const Flood* Npc::getEnsembleAccessible() const noexcept {
    return associated_flood;
}

Flood* Npc::getEnsembleAccessible() noexcept {
    return associated_flood;
}

bool Npc::isAccessibleTile(int tileId) const {
    assert(associated_flood);
    return associated_flood->is_flooded(tileId);
}

bool Npc::isArrived() const {
   return estArrive;
}

void Npc::setArrived(bool etat) {
   estArrive = etat;
}

void Npc::setEnsembleAccessible(Flood* associated_flood) {
   this->associated_flood = associated_flood;
}

void Npc::setMoving() noexcept {
    currentState = moving;
}

void Npc::setWaiting() noexcept {
    currentState = waiting;
}

bool Npc::isWaiting() const noexcept {
    return currentState == waiting;
}

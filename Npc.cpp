
#include "Npc.h"
#include "Globals.h"
#include "GameManager.h"
#include <chrono>

Npc::Npc(const NPCInfo info) :
   id{ static_cast<int>(info.npcID) },
   tileId{ static_cast<int>(info.tileID) },
   tileObjectif{ -1 },
   chemin{ Chemin{} },
   ensembleAccessible{ static_cast<int>(info.tileID) },
   estArrive{ false }
{
}


void Npc::move(Tile::ETilePosition direction, Map &m) noexcept {
   tileId = m.getAdjacentTileAt(tileId, direction);
   if (m.getTile(tileId).getVoisinsMursNonInspecte().empty()) {
      m.getTile(tileId).setStatut(MapTile::Statut::INSPECTEE);
   }
   else {
      m.getTile(tileId).setStatut(MapTile::Statut::VISITE);
   }
   // Mettre a visitable les voisins accesible connu d'une case visité
   for (auto voisinID : m.getTile(tileId).getVoisinsAccessibles()) {
      if (m.getTile(voisinID).getStatut() == MapTile::Statut::CONNU) {
         m.getTile(voisinID).setStatut(MapTile::Statut::VISITABLE);
      }
   }
}

void Npc::resetChemins() noexcept {
   cheminsPossibles.clear();
   scoresAssocies.clear();
}

void Npc::addChemin(Chemin& chemin) noexcept {
   cheminsPossibles.push_back(chemin);
}

void Npc::addScore(int tileIndice, float score) noexcept {
   scoresAssocies[tileIndice] = score;
}

Chemin Npc::getCheminMinNonPris(vector<int> objectifsPris, int tailleCheminMax) const noexcept {
   Chemin cheminMin;
   cheminMin.setInaccessible();
   int distMin = tailleCheminMax;

   for (int i = 0; i < cheminsPossibles.size(); ++i) {
      Chemin chemin = cheminsPossibles[i];
      // Si le chemin n'est pas déjà pris et qu'il est plus court !
      int destination = (chemin.empty()) ? tileId : chemin.destination(); // si le npc est déjà arrivé il reste là
      if (chemin.isAccessible()
         && chemin.distance() < distMin
         && (objectifsPris.empty() || find(objectifsPris.begin(), objectifsPris.end(), destination) == objectifsPris.end())) {
         cheminMin = chemin;
         distMin = chemin.distance();
      }
   }

   return cheminMin;
}

void testBestScore(pair<const int, float> pair, float& bestScore, int& bestScoreIndice) {
   int tileId = pair.first;
   float score = pair.second;
   GameManager::Log("Case potentielle à explorer : " + to_string(tileId) + " de score " + to_string(score));
   if (score > bestScore) {
      bestScore = score;
      bestScoreIndice = tileId;
   }
}

int Npc::affecterMeilleurChemin(Map &m) noexcept {
   if (scoresAssocies.empty()) {
      // Dans ce cas-là on reste sur place !
      chemin = Chemin{};
      GameManager::Log("Le Npc " + to_string(id) + " n'a rien a rechercher et reste sur place !");
      return tileId;
   }

   // On cherche le meilleur score
   auto preScore = std::chrono::high_resolution_clock::now();
   float bestScore = scoresAssocies.begin()->second;
   int bestScoreIndice = scoresAssocies.begin()->first;
   for (auto pair : scoresAssocies) {
      testBestScore(pair, bestScore, bestScoreIndice);
   }
   auto postScore = std::chrono::high_resolution_clock::now();
   GameManager::Log("Durée chercher meilleur score = " + to_string(std::chrono::duration_cast<std::chrono::microseconds>(postScore - preScore).count() / 1000.f) + "ms");

   // On affecte son chemin, mais il nous faut le calculer ! =)
   auto preAStar = std::chrono::high_resolution_clock::now();
   chemin = m.aStar(tileId, bestScoreIndice);
   auto postAStar = std::chrono::high_resolution_clock::now();
   GameManager::Log("Le Npc " + to_string(id) + " va rechercher la tile " + to_string(chemin.destination()));
   GameManager::Log("Durée a* = " + to_string(std::chrono::duration_cast<std::chrono::microseconds>(postAStar - preAStar).count() / 1000.f) + "ms");

   // On renvoie la destination
   return chemin.destination();
}

void ajoutIfUnkown(Map &m, int voisin, const vector<int>& oldOpen, const vector<int>& Open, vector<int>& newOpen) {
   // Si elle est connu
   if (m.getTile(voisin).existe()) {
      // Si elle n'est pas déjà ajouté
      if (find(oldOpen.begin(), oldOpen.end(), voisin) == oldOpen.end() && find(Open.begin(), Open.end(), voisin) == Open.end()) {
         // On l'ajoute comme nouvelle tuile ouverte
         newOpen.push_back(voisin);
      }
   }
}

void addNewVoisins(Map &m, int tileID, const vector<int>& oldOpen, vector<int>& Open, vector<int>& newOpen) {
   for (auto voisin : m.getTile(tileID).getVoisinsAccessibles()) {
      ajoutIfUnkown(m, voisin, oldOpen, Open, newOpen);
   }
   // On définit les dernières tuiles ajoutés avec leur coût courant
   if (find(Open.begin(), Open.end(), tileID) == Open.end()) {
      Open.push_back(tileID);
   }
}

void parcourirNewVoisins(Map &m, int tileID, vector<int>& oldOpen, vector<int>& Open, vector<int>& newOpen) {
   oldOpen = newOpen;
   newOpen = vector<int>();
   // On regarde les voisins des dernieres tuiles ajoutées
   for (int tileID : oldOpen) {
      addNewVoisins(m, tileID, oldOpen, Open, newOpen);
   }
}

vector<int> Npc::floodfill(Map &m) {
   vector<int> Open;
   vector<int> oldOpen;
   vector<int> newOpen;

   // Initialisation newOpen aux cases Visite et Visitable
   for (auto tileID : ensembleAccessible) {
      if (m.getTile(tileID).getStatut() == MapTile::INSPECTEE || m.getTile(tileID).getStatut() == MapTile::VISITE || m.getTile(tileID).getStatut() == MapTile::VISITABLE) {
         newOpen.push_back(tileID);
      }
   }

   // Tant qu'il reste des noeuds à traiter ...
   while (!newOpen.empty()) {
      parcourirNewVoisins(m, tileId, oldOpen, Open, newOpen);
   }

   // On met à jour l'ensemble et les distances accessible d'un NPC
   ensembleAccessible = Open;
   return Open;
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

const vector<int>& Npc::getEnsembleAccessible() const noexcept {
   return ensembleAccessible;
}

bool Npc::isAccessibleTile(int tileId) const {
   PROFILE_SCOPE("Npc::isAccessibleTile");
   return find(ensembleAccessible.begin(), ensembleAccessible.end(), tileId) != ensembleAccessible.end();
}

bool Npc::isArrived() const {
   return estArrive;
}

void Npc::setArrived(bool etat) {
   estArrive = etat;
}

void Npc::setEnsembleAccessible(vector<int> newEnsembleAccessible) {
   ensembleAccessible = newEnsembleAccessible;
}

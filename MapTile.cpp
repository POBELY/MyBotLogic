
#include "MapTile.h"
#include "Map.h"
#include "TileInfo.h"
#include "GameManager.h"

MapTile::MapTile(unsigned int id, Map &m) :
   id{ static_cast<int>(id) },
   x{ static_cast<int>(id) % m.getColCount() },
   y{ static_cast<int>(id) / m.getColCount() },
   voisins{ vector<int>{} },
   type{ Tile::ETileType::TileAttribute_Default },
   statut{ INCONNU }
{
   voisins.reserve(6);

   int indice{ 0 };
   int i{ 0 };

   int indices[]{ id - m.getColCount() ,id + 1 ,id + m.getColCount() ,id + m.getColCount() - 1,id - 1,id - m.getColCount() - 1,
      id - m.getColCount() + 1 ,id + 1 ,id + m.getColCount() + 1 ,id + m.getColCount(),id - 1,id - m.getColCount() };

   bool conditionDirections[]{ (m.isInMap(indice) && y > 0) ,
      (m.isInMap(indice) && x < m.getColCount() - 1),
      (m.isInMap(indice) && y < m.getRowCount() - 1),
      (m.isInMap(indice) && y < m.getRowCount() - 1 && x > 0),
      (m.isInMap(indice) && x > 0),
      (m.isInMap(indice) && y > 0 && x > 0),
      (m.isInMap(indice) && x < m.getColCount() - 1),
      (m.isInMap(indice) && x < m.getColCount() - 1) ,
      (m.isInMap(indice) && x < m.getColCount() - 1 && y < m.getRowCount() - 1) ,
      (m.isInMap(indice) && y < m.getRowCount() - 1) ,
      (m.isInMap(indice) && x > 0) ,
      (m.isInMap(indice)) };

   int directions[]{ Tile::NE,Tile::E,Tile::SE,Tile::SW,Tile::W,Tile::NW };


   // On regarde sur quelle ligne on est, car ça change les indices
   if (y % 2 == 0) { // Ligne paire
                     // NE
      for (i; i < 6; ++i)
         if (conditionDirections[i])
         {
            voisinsDirection[directions[i]] = indices[i];
            voisins.push_back(voisinsDirection[directions[i]]);
         }
   }
   else { // Ligne impaire !
          // NE
      for (i; i < 6; ++i)
         if (conditionDirections[6 + i])
         {
            voisinsDirection[directions[i]] = indices[6 + i];
            voisins.push_back(voisinsDirection[directions[i]]);
         }

   }

   voisinsVisibles = voisins;
   voisinsAccessibles = voisins;
   voisinsMysterious = voisins;
}

void MapTile::setTileDecouverte(const TileInfo tile) {
   type = tile.tileType;
   statut = CONNU;
}

bool MapTile::isVoisinAccessible(int id) const noexcept {
   return std::find(voisinsAccessibles.begin(), voisinsAccessibles.end(), id) != voisinsAccessibles.end();
}

bool MapTile::isVoisinVisible(int id) const noexcept {
   return std::find(voisinsVisibles.begin(), voisinsVisibles.end(), id) != voisinsVisibles.end();
}

bool MapTile::isVoisinMysterious(int id) const noexcept {
   return std::find(voisinsMysterious.begin(), voisinsMysterious.end(), id) != voisinsMysterious.end();
}

bool MapTile::hadActivateur() const noexcept
{
   return activateur != -1;
}

bool MapTile::hadisolatedClosedDoors() const noexcept
{
   return !voisinsIsolatedClosedDoors.empty();
}

int MapTile::getVoisinByDirection(Tile::ETilePosition direction) const noexcept {
   return voisinsDirection[direction];
}

void MapTile::removeMysterieux(int id) {
   auto it = find(voisinsMysterious.begin(), voisinsMysterious.end(), id);
   if (it != voisinsMysterious.end()) {
      voisinsMysterious.erase(it);
   }
}

void MapTile::removeAccessible(int id) {
   auto it = find(voisinsAccessibles.begin(), voisinsAccessibles.end(), id);
   if (it != voisinsAccessibles.end()) {
      voisinsAccessibles.erase(it);
   }
}

void MapTile::removeVisible(int id) {
   auto it = find(voisinsVisibles.begin(), voisinsVisibles.end(), id);
   if (it != voisinsVisibles.end()) {
      voisinsVisibles.erase(it);
   }
}

void MapTile::removeMurNonInspectee(int id) {
   auto it = find(voisinsMursNonInspectee.begin(), voisinsMursNonInspectee.end(), id);
   if (it != voisinsMursNonInspectee.end()) {
      voisinsMursNonInspectee.erase(it);
   }
   if (statut == VISITE && voisinsMursNonInspectee.empty()) {
      statut = INSPECTEE;
   }
}

void MapTile::removeMur(int id) {
   auto it = find(voisinsMurs.begin(), voisinsMurs.end(), id);
   if (it != voisinsMurs.end()) {
      voisinsMurs.erase(it);
   }
}

void MapTile::removeIsolatedClosedDoor(int id) {
   auto it = find(voisinsIsolatedClosedDoors.begin(), voisinsIsolatedClosedDoors.end(), id);
   if (it != voisinsIsolatedClosedDoors.end()) {
      voisinsIsolatedClosedDoors.erase(it);
   }
}

void MapTile::addMur(int id)
{
   auto it = find(voisinsMurs.begin(), voisinsMurs.end(), id);
   if (it == voisinsMurs.end()) {
      voisinsMurs.push_back(id);
      voisinsMursNonInspectee.push_back(id);
   }
}

void MapTile::addIsolatedClosedDoor(int id)
{
   auto it = find(voisinsIsolatedClosedDoors.begin(), voisinsIsolatedClosedDoors.end(), id);
   if (it == voisinsIsolatedClosedDoors.end()) {
      voisinsIsolatedClosedDoors.push_back(id);
   }
}

void MapTile::setActivateur(int id)
{
   activateur = id;
}

int MapTile::inspecter() {
   return voisinsMursNonInspectee.back();;
}


bool MapTile::existe() const noexcept {
   return statut != MapTile::Statut::INCONNU;
}

bool MapTile::inspectable() const noexcept {
   return statut != MapTile::Statut::INSPECTEE;
}

int MapTile::getId() const noexcept {
   return id;
}

int MapTile::getX() const noexcept {
   return x;
}

int MapTile::getY() const noexcept {
   return y;
}

Tile::ETileType MapTile::getType() const noexcept {
   return type;
}

const vector<int>& MapTile::getVoisins() const noexcept {
   return voisins;
}

const vector<int>& MapTile::getVoisinsAccessibles() const noexcept {
   return voisinsAccessibles;
}

void MapTile::addVoisinAccessible(int voisinID) noexcept {
   voisinsAccessibles.push_back(voisinID);
}

void MapTile::addVoisinVisible(int voisinID) noexcept {
   voisinsVisibles.push_back(voisinID);
}


const vector<int>& MapTile::getVoisinsVisibles() const noexcept {
   return voisinsVisibles;
}

const vector<int>& MapTile::getVoisinsMysterieux() const noexcept {
   return voisinsMysterious;
}

int MapTile::getActivateur() const noexcept {
   return activateur;
}

const vector<int>& MapTile::getVoisinsMursNonInspecte() {
   return voisinsMursNonInspectee;
}

bool MapTile::isInVoisins(int id) const noexcept {
   return find(voisins.begin(), voisins.end(), id) != voisins.end();
}

bool MapTile::isInVoisinsAccessibles(int id) const noexcept {
   return find(voisinsAccessibles.begin(), voisinsAccessibles.end(), id) != voisinsAccessibles.end();
}

bool MapTile::isInVoisinsVisibles(int id) const noexcept {
   return find(voisinsVisibles.begin(), voisinsVisibles.end(), id) != voisinsVisibles.end();
}

bool MapTile::isInVoisinsMysterieux(int id) const noexcept {
   return find(voisinsMysterious.begin(), voisinsMysterious.end(), id) != voisinsMysterious.end();
}

MapTile::Statut MapTile::getStatut() const noexcept {
   return statut;
}

bool MapTile::isAccessible() const noexcept {
    return getStatut() == MapTile::INSPECTEE 
       ||  getStatut() == MapTile::VISITE 
       ||  getStatut() == MapTile::VISITABLE;
}

void MapTile::setStatut(MapTile::Statut new_statut) {
   statut = new_statut;
}

int MapTile::getIsolatedClosedDoor() {
   return voisinsIsolatedClosedDoors.back();
}

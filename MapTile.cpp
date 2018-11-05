
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

bool MapTile::hadInspection() const noexcept
{
   return inspection != -1;
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

void MapTile::addMur(int id)
{
   voisinsMurs.push_back(id);
   voisinsMursNonInspectee.push_back(id);
}

void MapTile::setActivateur(int id)
{
   activateur = id;
}

int MapTile::inspecter() {
   inspection = voisinsMursNonInspectee.back();
   voisinsMursNonInspectee.pop_back();
   if (voisinsMursNonInspectee.empty()) {
      statut = INSPECTEE;
   }
   return inspection;
}

bool MapTile::existe() {
   return statut != MapTile::Statut::INCONNU;
}

bool MapTile::inspectable() {
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

vector<int> MapTile::getVoisins() const noexcept {
   return voisins;
}

vector<int> MapTile::getVoisinsAccessibles() const noexcept {
   return voisinsAccessibles;
}

void MapTile::addVoisinAccessible(int voisinID) noexcept {
   voisinsAccessibles.push_back(voisinID);
}

void MapTile::addVoisinVisible(int voisinID) noexcept {
   voisinsVisibles.push_back(voisinID);
}


vector<int> MapTile::getVoisinsVisibles() const noexcept {
   return voisinsVisibles;
}

vector<int> MapTile::getVoisinsMysterieux() const noexcept {
   return voisinsMysterious;
}

int MapTile::getActivateur() const noexcept {
   return activateur;
}

int MapTile::getInspection() noexcept {
   int res = inspection;
   inspection = -1;
   return res;
}

vector<int> MapTile::getVoisinsMursNonInspecte() {
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

void MapTile::setStatut(MapTile::Statut new_statut) {
   statut = new_statut;
}

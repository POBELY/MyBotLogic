#ifndef MAP_TILE_H
#define MAP_TILE_H

#include "TileInfo.h"
#include "MyBotLogic/Map.h"
#include <vector>
using namespace std;

class Map;

class MapTile {
public:
   enum Statut { INCONNU, CONNU, VISITABLE, VISITE, INSPECTEE };

private:
   int id;
   int x, y; // La position de la tile. x est l'indice de colonne, y est l'indice de ligne.
   Tile::ETileType type;
   int voisinsDirection[6] = { -1,-1,-1,-1,-1,-1 };
   vector<int> voisins; // les identifiants des voisins de la tuile
   vector<int> voisinsAccessibles; // les voisins connus et accessible (pas de murs ni de fenêtres) y compris les voisinsMysterious
   vector<int> voisinsVisibles; // les voisins visibles (contient les voisins accessibles et les voisins fenetres)
   vector<int> voisinsMysterious; // les voisins sur lequel on a pas encore d'information
   vector<int> voisinsMurs = {}; //ce sont les murs entourant la Tile
   vector<int> voisinsMursNonInspectee = {}; //ce sont les murs entourant la Tile
   vector<int> voisinsIsolatedClosedDoors = {}; //ce sont les portes isolées entourant la Tile
   int activateur = -1;
   Statut statut;

public:
   MapTile() = default; // Constructeur par défaut obligatoire pour pouvoir utiliser tuple ...
   MapTile(unsigned int id, Map &m); // Appelé dès le début et uniquement là !

   void setTileDecouverte(const TileInfo ti);
   void removeMysterieux(int id);
   void removeAccessible(int id);
   void removeVisible(int id);
   void removeMurNonInspectee(int id);
   void removeMur(int id);
   void removeIsolatedClosedDoor(int id);
   void addMur(int id);
   void addIsolatedClosedDoor(int id);
   void setActivateur(int id);
   int inspecter();

   int getVoisinByDirection(Tile::ETilePosition direction) const noexcept; // Permet de récupérer le voisin dans une certaine direction d'une tile
   bool isVoisinAccessible(int id) const noexcept;
   bool isVoisinVisible(int id) const noexcept;
   bool isVoisinMysterious(int id) const noexcept;
   bool hadActivateur() const noexcept;
   bool hadisolatedClosedDoors() const noexcept;

   bool existe() const noexcept;
   bool inspectable() const noexcept;

   int getId() const noexcept;
   int getX() const noexcept;
   int getY() const noexcept;
   Tile::ETileType getType() const noexcept;
   const vector<int>& getVoisins() const noexcept;
   const vector<int>& getVoisinsAccessibles() const noexcept;
   void addVoisinAccessible(int voisinID) noexcept;
   void addVoisinVisible(int voisinID) noexcept;
   const vector<int>& getVoisinsVisibles() const noexcept;
   const vector<int>& getVoisinsMysterieux() const noexcept;
   int getActivateur() const noexcept;
   const vector<int>& getVoisinsMursNonInspecte();
   bool isInVoisins(int id) const noexcept;
   bool isInVoisinsAccessibles(int id) const noexcept;
   bool isInVoisinsVisibles(int id) const noexcept;
   bool isInVoisinsMysterieux(int id) const noexcept;
   Statut getStatut() const noexcept;
   bool isAccessible() const noexcept;
   void setStatut(Statut new_statut);
   int getIsolatedClosedDoor();
};

#endif

#ifndef MAP_MANAGER_H
#define MAP_MANAGER_H

#include "LevelInfo.h"
#include "TileInfo.h"
#include "ObjectInfo.h"
#include "Chemin.h"
#include "Npc.h"
#include "MyBotLogic/MapTile.h"
#include <map>
#include <functional>

using namespace std;

class tile_inexistante {};

class MapTile;
class Npc;
class Map {
    int rowCount;
    int colCount;
    int nbTilesDecouvertes;
    vector<MapTile> tiles;
    vector<unsigned int> objectifs;
    map<unsigned int, ObjectInfo> murs;
    map<unsigned int, ObjectInfo> portes;
    map<unsigned int, ObjectInfo> fenetres;
    map<unsigned int, ObjectInfo> activateurs;
    mutable vector<vector<int>> distances; // ensemble des distances cases � cases
    mutable vector<vector<int>> distancesAStar; // ensemble des distances cases � case

    int total_size() const noexcept { return rowCount * colCount; }

public:

    Map() = default;
    Map(const LevelInfo);
    bool isInMap(int idTile) const noexcept;

    Chemin aStar(int depart, int arrivee, float coefEvaluation = 1) const noexcept; // Renvoie le chemin � parcourir pour aller du d�part � l'arriv�e

    float distanceL2(int depart, int arrivee) const noexcept; // Renvoie la distance L2 � vol d'oiseau !
    int distanceHex(int depart, int arrivee) const noexcept;
    Tile::ETilePosition getDirection(int tile1, int tile2) const noexcept; // Permet de savoir dans quel sens se d�placer pour aller d'une tile � l'autre si celles-ci sont adjacentes ! =)
    int getAdjacentTileAt(int tileSource, Tile::ETilePosition direction) const noexcept; // Permet de r�cup�rer l'indice d'une tuile adjacente � une autre
    int tailleCheminMax() const noexcept; // Permet de savoir la taille maximum d'un chemin

    void addTile(TileInfo) noexcept; // Permet de rajouter une tile � la map
    void addObject(ObjectInfo) noexcept; // Permet de rajouter un object � la map

    int getX(int id) const noexcept; // Permet de r�cup�rer x et y � partir d'un indice
    int getY(int id) const noexcept;
    vector<int> getVoisins(int id) const noexcept;

    int getRowCount() const noexcept;
    int getColCount() const noexcept;
    int getNbTiles() const noexcept;
    int getNbTilesDecouvertes() const noexcept;
    MapTile& getTile(int id);
    int getDistance(int tile1,int tile2) const;

    const vector<unsigned int>& getObjectifs() const noexcept;
    const map<unsigned int, ObjectInfo>& getMurs() const noexcept;
    const map<unsigned int, ObjectInfo>& getPortes() const noexcept;
    const map<unsigned int, ObjectInfo>& getFenetres() const noexcept;
    const map<unsigned int, ObjectInfo>& getActivateurs() const noexcept;

    bool objectExist(int id); // Permet de savoir si un objet existe d�j� ou pas
};


#endif

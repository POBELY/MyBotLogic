#ifndef MAP_MANAGER_H
#define MAP_MANAGER_H

#include "LevelInfo.h"
#include "TileInfo.h"
#include "ObjectInfo.h"
#include "Chemin.h"
#include "Npc.h"
#include "MyBotLogic/MapTile.h"
#include <map>
using namespace std;

class tile_inexistante {};

class MapTile;
class Npc;
class GameManager;
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
    vector<vector<int>> distancesAStar; // ensemble des distances cases � case
    vector<int> interactObjects; // ensemble des objets avec lesquels on a interagit au tour pr�c�dent
    vector<int> isolatedClosedDoors; // ensemble des portes sans interrupteurs

    int total_size() const noexcept { return rowCount * colCount; }
public:
   bool quitButton = false;
   bool presenceForbiddenTiles = false;

    Map() = default;
    Map(const LevelInfo);
    bool isInMap(int idTile) const noexcept;

    Chemin aStar(int depart, int arrivee, float coefEvaluation = 1) noexcept; // Renvoie le chemin � parcourir pour aller du d�part � l'arriv�e

    float distanceL2(int depart, int arrivee) const noexcept; // Renvoie la distance L2 � vol d'oiseau !
    int distanceHex(int depart, int arrivee) const noexcept;
    Tile::ETilePosition getDirection(int tile1, int tile2) const noexcept; // Permet de savoir dans quel sens se d�placer pour aller d'une tile � l'autre si celles-ci sont adjacentes ! =)
    int Map::getAdjacentTileAt(int tileSource, Tile::ETilePosition direction) const noexcept; // Permet de r�cup�rer l'indice d'une tuile adjacente � une autre
    int tailleCheminMax() const noexcept; // Permet de savoir la taille maximum d'un chemin

    void addTile(TileInfo) noexcept; // Permet de rajouter une tile � la map
	void addTile(TileInfo, GameManager& gm) noexcept;
    void addObject(ObjectInfo) noexcept; // Permet de rajouter un object � la map
    void addInteractObject(int objectID);
    void viderInteractObjects();

    int getX(int id) const noexcept; // Permet de r�cup�rer x et y � partir d'un indice
    int getY(int id) const noexcept;
    vector<int> getVoisins(int id) const noexcept;

    int getRowCount() const noexcept;
    int getColCount() const noexcept;
    int getTailleTotal() const noexcept { return getRowCount() * getColCount(); }
    int getNbTiles() const noexcept;
    int getNbTilesDecouvertes() const noexcept;
    MapTile& getTile(int id);
    vector<MapTile>& getTiles();
    const MapTile& getTile(int id) const;
    int getDistance(int tile1,int tile2);
    int getDistanceAStar(int tile1, int tile2);

    const vector<unsigned int>& getObjectifs() const;
    map<unsigned int, ObjectInfo> getMurs();
    map<unsigned int, ObjectInfo> getPortes();
    map<unsigned int, ObjectInfo> getFenetres();
    map<unsigned int, ObjectInfo> getActivateurs();
    vector<int> getInteractObjects();
    vector<int> getIsolatedClosedDoorsCopy();
    vector<int>& getIsolatedClosedDoors();

    bool objectExist(int id); // Permet de savoir si un objet existe d�j� ou pas
    bool hadInteract(int id); // Permet de savoir si on � intertagit avec cet objet au tour pr�c�dent
};


#endif

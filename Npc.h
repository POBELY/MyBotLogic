#ifndef NPC_H
#define NPC_H

#include "Globals.h"
#include "NPCInfo.h"
#include "Chemin.h"
#include "Map.h"
#include <vector>
using namespace std;

class tile_inaccessible {};

class Map;
class Npc {
private:
   vector<Chemin> cheminsPossibles; // Ceci est une variable temporaire permettant de stocker les chemins parmis lesquelles choisirs un objectif
   map<int, float> scoresAssocies; // Les scores associés aux tiles !

   int id;
   int tileId; // Sa position sur la carte
   int tileObjectif; // Là où il doit aller !
   Chemin chemin; // Utilisé pour savoir quel chemin suivre pour se rendre à l'objectif
   vector<int> ensembleAccessible; // ensemble des tuiles auquel un npc à accès
   bool estArrive; // indique si le npc a atteind son objectif
   int interactWall = -1;
   int interactDoor = -1;

public:

   Npc() = default;
   Npc(const NPCInfo);

   void move(Tile::ETilePosition, Map&) noexcept; // Permet de faire bouger notre npc dans notre modèle =)

   void resetChemins() noexcept;
   void addChemin(Chemin& chemin) noexcept;
   void addScore(int tileIndice, float score) noexcept;
   Chemin getCheminMinNonPris(vector<int> objectifsPris, int tailleCheminMax) const noexcept; // Permet de trouver le chemin le plus court qui ne soit pas déjà pris
   int affecterMeilleurChemin(Map &m) noexcept; // Affecte au npc le chemin avec le meilleur score et renvoie la destination de ce chemin !
   const vector<int>& floodfill(Map &m); // Calcule le coût et l'ensemble des tiles accessibles pour un npcs, et MAJ ses attributs.
   void inspectWall(int wallID);
   void openDoor(int doorID);

   bool hadInspection() const noexcept;
   bool hadOpenDoor() const noexcept;

   int getId() const noexcept;
   int getTileId() const noexcept;
   int getShortTermObjectif() const noexcept;
   int getTileObjectif() const noexcept;
   int getInteractWall() noexcept;
   int getInteractDoor() noexcept;
   void setTileObjectif(int idTile);
   const Chemin& getChemin() const noexcept;
   Chemin& getChemin() noexcept;
   void setChemin(const Chemin& chemin);
   void setChemin(Chemin&& chemin);
   const vector<int>& getEnsembleAccessible() const noexcept;
   bool isAccessibleTile(int tileId) const;
   bool isArrived() const;
   void setArrived(bool etat);
   void setEnsembleAccessible(const vector<int>& newEnsembleAccessible);
   void setEnsembleAccessible(vector<int>&& newEnsembleAccessible);
};

#endif
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
   map<int, float> scoresAssocies; // Les scores associ�s aux tiles !

   int id;
   int tileId; // Sa position sur la carte
   int tileObjectif; // L� o� il doit aller !
   Chemin chemin; // Utilis� pour savoir quel chemin suivre pour se rendre � l'objectif
   vector<int> ensembleAccessible; // ensemble des tuiles auquel un npc � acc�s
   bool estArrive; // indique si le npc a atteind son objectif

public:

   Npc() = default;
   Npc(const NPCInfo);

   void move(Tile::ETilePosition, Map&) noexcept; // Permet de faire bouger notre npc dans notre mod�le =)

   void resetChemins() noexcept;
   void addChemin(Chemin& chemin) noexcept;
   void addScore(int tileIndice, float score) noexcept;
   Chemin getCheminMinNonPris(vector<int> objectifsPris, int tailleCheminMax) const noexcept; // Permet de trouver le chemin le plus court qui ne soit pas d�j� pris
   int affecterMeilleurChemin(Map &m) noexcept; // Affecte au npc le chemin avec le meilleur score et renvoie la destination de ce chemin !
   vector<int> floodfill(Map &m); // Calcule le co�t et l'ensemble des tiles accessibles pour un npcs, et MAJ ses attributs.

   int getId() const noexcept;
   int getTileId() const noexcept;
   int getShortTermObjectif() const noexcept;
   int getTileObjectif() const noexcept;
   void setTileObjectif(int idTile);
   const Chemin& getChemin() const noexcept;
   Chemin& getChemin() noexcept;
   void setChemin(const Chemin& chemin);
   void setChemin(Chemin&& chemin);
   const vector<int>& getEnsembleAccessible() const noexcept;
   bool isAccessibleTile(int tileId) const;
   bool isArrived() const;
   void setArrived(bool etat);
   void setEnsembleAccessible(vector<int> newEnsembleAccessible);
};

#endif
#ifndef NPC_H
#define NPC_H

#include "Globals.h"
#include "NPCInfo.h"
#include "Chemin.h"
#include "Map.h"
#include <vector>
#include "BehaviorTree/Composite/Selecteur.h"
class GameManager;
using namespace std;

class tile_inaccessible {};

class Flood;

class Map;
class Npc {
private:
   map<int, float> scoresAssocies; // Les scores associ�s aux tiles !

   int id;
   int tileId; // Sa position sur la carte
   int tileObjectif; // L� o� il doit aller !
   int tileGoal = -1; //la tile goal � atteindre pour terminer l'execution
   Chemin chemin; // Utilis� pour savoir quel chemin suivre pour se rendre � l'objectif
   //vector<int> ensembleAccessible; // ensemble des tuiles auquel un npc � acc�s
   Flood* associated_flood;
   bool estArrive; // indique si le npc a atteind son objectif
   int interactWall = -1;
   int interactDoor = -1;
   bool cross = false;
   Selecteur behaviorTreeNpc;
   
   enum CollisionState { waiting, moving };
   CollisionState currentState = moving;

public:

   Npc() = default;
   Npc(const NPCInfo);
   void initializeBehaviorTree(GameManager& gm) noexcept; // Permet d'initialiser le BT

   void execute() noexcept {
      //ScopedProfiler p("NPC Execute");
      behaviorTreeNpc.execute();
   };

   void move(Tile::ETilePosition, Map&) noexcept; // Permet de faire bouger notre npc dans notre mod�le =)

   void resetChemins() noexcept;
   void addScore(int tileIndice, float score) noexcept;
   int affecterMeilleurChemin(Map &m) noexcept; // Affecte au npc le chemin avec le meilleur score et renvoie la destination de ce chemin !
   void floodfill(Map &m); // Calcule le co�t et l'ensemble des tiles accessibles pour un npcs, et MAJ ses attributs.
   void inspectWall(int wallID);
   void openDoor(int doorID);

   bool hadInspection() const noexcept;
   bool hadOpenDoor() const noexcept;
   bool goingToCross() const noexcept { return cross; };

   int getId() const noexcept;
   int getTileId() const noexcept;
   int getShortTermObjectif() const noexcept;
   int getTileObjectif() const noexcept;
   int getInteractWall() noexcept;
   int getInteractDoor() noexcept;
   void setTileObjectif(int idTile);
   void setTileGoal(int idTile);
   int getTileGoal() const;
   const Chemin& getChemin() const noexcept;
   Chemin& getChemin() noexcept;
   void setChemin(const Chemin& chemin);
   void setChemin(Chemin&& chemin);
   const Flood* getEnsembleAccessible() const noexcept;
   Flood* getEnsembleAccessible() noexcept;
   bool isAccessibleTile(int tileId) const;
   bool isArrived() const;
   void setArrived(bool etat);
   void setEnsembleAccessible(Flood* associated_flooding);
   void setCross(bool newCross) { cross = newCross; };
   
   void setMoving() noexcept;
   void setWaiting() noexcept;
   bool isWaiting() const noexcept;
   
};

#endif
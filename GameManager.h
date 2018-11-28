#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include "Map.h"
#include "Npc.h"
#include "Logger.h"
#include "Mouvement.h"
#include "TurnInfo.h"
#include "Profileur.h"
#include "Flood.h"

#include "BehaviorTree/Composite/Selecteur.h"

#include <map>

class npc_inexistant {};
class npc_deja_existant {};

class GameManager {
   static Logger logger, loggerRelease;
   std::vector<Npc> npcs;
public:
   Map m;
   Selecteur behaviorTreeManager; // Arbre de comportement du GameManager pour d�terminer la strat�gie � suivre
   vector<int> objectifPris; // Permet de savoir quels sont les objectifs actuellement assign�s � des npcs
   //vector<vector<unsigned int>> flux;
   vector<std::unique_ptr<Flood>> floods;
   vector<vector<unsigned int>> shared_floods_mapping;
   vector<int> tilesAVisiter = {}; // ensemble des tuiles visit�es par nos Npc au cours d'un tour
   vector<int> isolatedClosedDoorsToOpen = {}; // ensemble des portes dont on ne cherche pas d'interaction sur le tour courant

   GameManager() = default;
   GameManager(LevelInfo);
   void moveNpcs(vector<Action*>& actionList) noexcept; // Remplie l'action liste !
   void reafecterObjectifsSelonDistance(); // R�affecte les objectifs des Npcs entre
   void ordonnerMouvements(vector<Mouvement>& mouvements) noexcept; // Permet d'ordonner les mouvements pour �viter les collisions et g�rer les politesses de priorit�s =)
   void updateModel(const TurnInfo&) noexcept; // Met � jour le mod�le avec les informations que d�couvrent les NPCS
   void unmerge_floods();
   void merge_floods();
   void grow_floods();
   void init_floods();
   void updateFlux() noexcept;
   void InitializeBehaviorTree() noexcept; // Permet d'initialiser le BT
   void execute() noexcept {
      ScopedProfiler p("GM Execute");
      behaviorTreeManager.execute();
   };

   Npc& getNpcById(int id);
   std::vector<Npc>& getNpcs();
   void addNpc(Npc npc);
   bool isDoorAdjacente(int interrupteurID);
   void setNpcsGoalTile(unsigned int tileId);

   static void Log(string str) noexcept { // Permet de d�bugger ! :D
#ifndef _DEBUG
      return;
#endif
#ifdef _DEBUG
      logger.Log(str);
#endif
   }
   static void LogRelease(string str) noexcept { // Permet de d�bugger ! :D
      loggerRelease.Log(str);
   }
   static void SetLog(string path, string fileName) noexcept { // Permet d'initialiser le logger =)
#ifndef _DEBUG
      return;
#endif
#ifdef _DEBUG
      logger.Init(path, fileName);
#endif
   }
   static void SetLogRelease(string path, string fileName) noexcept { // Permet d'initialiser le logger =)
      loggerRelease.Init(path, fileName);
   }

private:
   void addNewTiles(TurnInfo ti) noexcept;
   void addNewObjects(TurnInfo ti) noexcept;
   vector<Mouvement> getAllMouvements();
   int getIndiceMouvementPrioritaire(vector<Mouvement>& mouvements, vector<int> indicesAConsiderer);
   void gererCollisionsMemeCaseCible(vector<Mouvement>& mouvements);
   void stopNonPrioritaireMouvements(vector<Mouvement>& mouvements, vector<int> indicesMouvementsSurMemeCaseCible, int indiceMouvementPrioritaire, bool& continuer);
};

#endif
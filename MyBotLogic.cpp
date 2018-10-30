#include "MyBotLogic.h"

#include "TurnInfo.h"
#include "NPCInfo.h"
#include "LevelInfo.h"

#include "windows.h"
#include "MyBotLogic/Profileur.h"
#include <chrono>
#include <fstream>
using namespace std::chrono;


MyBotLogic::MyBotLogic() :
    logpath{""}
{
	//Write Code Here
}

/*virtual*/ MyBotLogic::~MyBotLogic()
{
	//Write Code Here
}

/*virtual*/ void MyBotLogic::Configure(int argc, char *argv[], const std::string& _logpath)
{
#ifdef BOT_LOGIC_DEBUG
	mLogger.Init(_logpath, "MyBotLogic.log");
#endif

	BOT_LOGIC_LOG(mLogger, "Configure", true);
    /* _logpath =
    C:\Users\dusa2404\Documents\IA\IABootCamp\AIBot_v0.59\\LocalMatchResults\aibotlog
    */
    logpath = _logpath;
	
	//Write Code Here
}

/*virtual*/ void MyBotLogic::Start()
{
	//Write Code Here
}

/*virtual*/ void MyBotLogic::Init(LevelInfo& _levelInfo)
{
    auto pre = high_resolution_clock::now();
    // Le logger
	GameManager::SetLog(logpath, "MyLog.log");
	GameManager::SetLogRelease(logpath, "MyLogRelease.log");
    // On cr�e notre mod�le du jeu en cours !
    gm = GameManager(_levelInfo);
    gm.InitializeBehaviorTree();

    // On associe � chaque npc son objectif !
    //gm.associateNpcsWithObjectiv();
    auto post = high_resolution_clock::now();
    GameManager::Log("Dur�e Initialisation = " + to_string(duration_cast<microseconds>(post - pre).count() / 1000.f) + "ms");
}

/*virtual*/ void MyBotLogic::OnGameStarted()
{
	//Write Code Here
}

/*virtual*/ void MyBotLogic::FillActionList(TurnInfo& _turnInfo, std::vector<Action*>& _actionList)
{
    GameManager::Log("TURN =========================== " + to_string(_turnInfo.turnNb));
    ScopedProfiler p("Turn");

    // On compl�te notre mod�le avec l'information qu'on vient de d�couvrir !
    gm.updateModel(_turnInfo);

    // On d�finit notre strat�gie en ex�cutant notre arbre de comportement
    gm.execute();

    // On fait se d�placer chaque Npc vers son objectif associ� =)
    gm.moveNpcs(_actionList);
    GameManager::Log("Compteur 1 : " + to_string(gm.m.compteur1));
    GameManager::Log("Compteur 2 : " + to_string(gm.m.compteur2));
    gm.m.compteur1 = 0;
    gm.m.compteur2 = 0;
}

/*virtual*/ void MyBotLogic::Exit()
{
    GameManager::Log("exit");
    GameManager::LogRelease("exit");

    std::ofstream profile_json("LocalMatchResults\\aibotlog\\profile.json");
    if(profile_json) {
        profile_json << EventProfiler::instance() << std::endl;
        GameManager::Log("save profile.json");
        GameManager::LogRelease("save profile.json");
    }
    else {
        GameManager::Log("failed to save profile.json");
        GameManager::LogRelease("failed to save profile.json");
    }
}
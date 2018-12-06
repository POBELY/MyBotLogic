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
    logpath = _logpath;
	
	//Write Code Here
}

/*virtual*/ void MyBotLogic::Start()
{
	//Write Code Here
}

/*virtual*/ void MyBotLogic::Init(LevelInfo& _levelInfo)
{
    PROFILE_SCOPE("Init");
    // Le logger
	GameManager::SetLog(logpath, "MyLog.log");
	GameManager::SetLogRelease(logpath, "MyLogRelease.log");
    // On crée notre modèle du jeu en cours !
    gm = GameManager(_levelInfo);
    gm.InitializeBehaviorTree();

}

/*virtual*/ void MyBotLogic::OnGameStarted()
{
	//Write Code Here
}

/*virtual*/ void MyBotLogic::FillActionList(TurnInfo& _turnInfo, std::vector<Action*>& _actionList)
{
    //PROFILE_SCOPE("Turn");
    GameManager::Log("TURN =========================== " + to_string(_turnInfo.turnNb));
    auto pre = std::chrono::high_resolution_clock::now();
    // On complète notre modèle avec l'information qu'on vient de découvrir !
    gm.updateModel(_turnInfo);

    // On définit notre stratégie en exécutant notre arbre de comportement
    gm.execute();

    // On fait se déplacer chaque Npc vers son objectif associé =)
    gm.moveNpcs(_actionList);
    auto post = std::chrono::high_resolution_clock::now();
    GameManager::LogRelease("Durée Tour = " + to_string(std::chrono::duration_cast<std::chrono::microseconds>(post - pre).count() / 1000.f) + "ms");
}

/*virtual*/ void MyBotLogic::Exit()
{
    GameManager::Log("exit");
    GameManager::LogRelease("exit");

#if ENABLE_PROFILING
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
#endif
}

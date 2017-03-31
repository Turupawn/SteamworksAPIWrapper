#ifndef STEAM_INTEGRATION_H
#define STEAM_INTEGRATION_H

#include <iostream>
#include <fstream>
#include "include/steam_api.h"
#include "include/isteamuserstats.h"
#include "include/isteamremotestorage.h"
#include "LeaderboardHandler.h"

using namespace std;

class LeaderboardHandler;

void steamInit();
bool steamIsUsingApi();
void steamUnlockAchievement(string achievement_name);
void steamSetScore(string leaderboard_name, double score);
Leaderboard* steamGetLeaderboard(string leaderboard_name);
void steamFindLeaderboard(string leaderboard_name);
void steamDownloadEntryAttachment(LeaderboardEntry* leaderboard_entry);
void steamStoreLeaderboardAttachment(string leaderboard_name, char* attachment, int attachment_size);
void steamSetStat(string stat_name, double stat);
double steamGetStat(string stat_name);
void steamStoreData(string key, string data);
string steamGetData(string key);
void steamUpdateCallbacks();
string steamGetState();

void steamSetCurrentControllerActionSet(string action_set_name);
bool steamIsControllerActionDown(string action_name);
bool steamIsUsingSteamController();
void steamShowControllerBindingPanel();


#endif

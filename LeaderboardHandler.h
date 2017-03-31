#ifndef LEADERBOARD_HANDLER_H
#define LEADERBOARD_HANDLER_H

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include "include/steam_api.h"
#include "include/isteamuserstats.h"
#include "../Leaderboard.h"
#include "../LeaderboardEntry.h"
#include "../../RosalilaUtility/RosalilaUtility.h"

using namespace std;

class LeaderboardHandler
{
public:
    map<string,SteamLeaderboard_t> steam_leaderboards;
    int leaderboards_to_download_count;
    string state;
    map<string, Leaderboard* >leaderboards;
    map<LeaderboardEntry*, UGCHandle_t> entries_attachments;

    LeaderboardHandler();
    void findLeaderboard( const char *pchLeaderboardName );
    void downloadEntryAttachment(LeaderboardEntry* leaderboard_entry);
    void uploadScore(string leaderboard_name, double score);
    void storeLeaderboardUGC(string leaderboard_name, char* attachment, int attachment_size);

    void onFindLeaderboard( LeaderboardFindResult_t *pCallback,	bool bIOFailure );
    void onDownloadTopScores(LeaderboardScoresDownloaded_t *pCallback, bool bIOFailure);
    void onDownloadNearScores(LeaderboardScoresDownloaded_t *pCallback, bool bIOFailure);
    void onDownloadFriendsScores(LeaderboardScoresDownloaded_t *pCallback, bool bIOFailure);
    void onDownloadUGCComplete(RemoteStorageDownloadUGCResult_t *pCallback, bool bIOFailure);
    void onStoreLeaderboardUGCComplete(RemoteStorageFileShareResult_t *pCallback,	bool bIOFailure );
    void onStoreLeaderboardUGCComplete2(RemoteStorageFileWriteAsyncComplete_t *pCallback,	bool bIOFailure );
    void onUploadScore(LeaderboardScoreUploaded_t *pCallback, bool bIOFailure);
};

#endif

#include "LeaderboardHandler.h"

LeaderboardHandler::LeaderboardHandler()
{
    leaderboards_to_download_count=0;
    this->state="finished";
}

void LeaderboardHandler::findLeaderboard( const char *pchLeaderboardName )
{
    rosalila()->utility->writeLogLine("Finding Leaderboard");
    this->state = "loading";
    leaderboards[pchLeaderboardName]=new Leaderboard();
    leaderboards_to_download_count+=3;
    CCallResult<LeaderboardHandler, LeaderboardFindResult_t> *find_leaderboard_callback = new CCallResult<LeaderboardHandler, LeaderboardFindResult_t>();
    SteamAPICall_t hSteamAPICall = SteamUserStats()->FindLeaderboard(pchLeaderboardName);
    find_leaderboard_callback->Set(hSteamAPICall, this, &LeaderboardHandler::onFindLeaderboard);
}

void LeaderboardHandler::downloadEntryAttachment(LeaderboardEntry* leaderboard_entry)
{
    rosalila()->utility->writeLogLine("Downloading entry attachment");
    this->state="loading";
    leaderboard_entry->attachment_state = "loading";
    SteamAPICall_t steam_api_call = SteamRemoteStorage()->UGCDownload(entries_attachments[leaderboard_entry],1);

    CCallResult<LeaderboardHandler, RemoteStorageDownloadUGCResult_t> *download_ugc_callback = new CCallResult<LeaderboardHandler, RemoteStorageDownloadUGCResult_t>();

    download_ugc_callback->Set(steam_api_call, this,
        &LeaderboardHandler::onDownloadUGCComplete);
}

void LeaderboardHandler::uploadScore(string leaderboard_name, double score)
{
    rosalila()->utility->writeLogLine("Uploading score");
    state = "loading";
    SteamAPICall_t hSteamAPICall = SteamUserStats()->UploadLeaderboardScore(steam_leaderboards[leaderboard_name],k_ELeaderboardUploadScoreMethodKeepBest, score, NULL, 0);

    CCallResult<LeaderboardHandler, LeaderboardScoreUploaded_t> *upload_score_callback = new CCallResult<LeaderboardHandler, LeaderboardScoreUploaded_t>();
    upload_score_callback->Set(hSteamAPICall, this, &LeaderboardHandler::onUploadScore);
}

void LeaderboardHandler::storeLeaderboardUGC(string leaderboard_name, char* attachment, int attachment_size)
{
    rosalila()->utility->writeLogLine("Storing leaderboard UGC");
    this->state = "loading";
    UGCFileWriteStreamHandle_t stream_handle = SteamRemoteStorage()->FileWriteStreamOpen(leaderboard_name.c_str());
    SteamRemoteStorage()->FileWriteStreamWriteChunk(stream_handle, attachment, attachment_size);
    SteamRemoteStorage()->FileWriteStreamClose(stream_handle);

    SteamAPICall_t hSteamAPICall = SteamRemoteStorage()->FileShare(leaderboard_name.c_str());
    CCallResult<LeaderboardHandler, RemoteStorageFileShareResult_t> *file_write_callback = new CCallResult<LeaderboardHandler, RemoteStorageFileShareResult_t>();
    file_write_callback->Set(hSteamAPICall, this,
        &LeaderboardHandler::onStoreLeaderboardUGCComplete);
}

void LeaderboardHandler::onFindLeaderboard( LeaderboardFindResult_t *pCallback,	bool bIOFailure )
{
    if (bIOFailure)
    {
        rosalila()->utility->writeLogLine("Leaderboard could not be found");
        this->state = "error";
        return;
    }
    rosalila()->utility->writeLogLine("Leaderboard found");

    steam_leaderboards[SteamUserStats()->GetLeaderboardName(pCallback->m_hSteamLeaderboard)] = pCallback->m_hSteamLeaderboard;

    SteamAPICall_t hSteamAPICall_top = SteamUserStats()->DownloadLeaderboardEntries(
        steam_leaderboards[SteamUserStats()->GetLeaderboardName(pCallback->m_hSteamLeaderboard)], k_ELeaderboardDataRequestGlobal, 1, 3);
    SteamAPICall_t hSteamAPICall_near = SteamUserStats()->DownloadLeaderboardEntries(
        steam_leaderboards[SteamUserStats()->GetLeaderboardName(pCallback->m_hSteamLeaderboard)], k_ELeaderboardDataRequestGlobalAroundUser, -1, 1);
    SteamAPICall_t hSteamAPICall_friends = SteamUserStats()->DownloadLeaderboardEntries(
        steam_leaderboards[SteamUserStats()->GetLeaderboardName(pCallback->m_hSteamLeaderboard)], k_ELeaderboardDataRequestFriends, -1, -1);

    CCallResult<LeaderboardHandler, LeaderboardScoresDownloaded_t> *download_top_scores_callback = new CCallResult<LeaderboardHandler, LeaderboardScoresDownloaded_t>();
    CCallResult<LeaderboardHandler, LeaderboardScoresDownloaded_t> *download_near_scores_callback = new CCallResult<LeaderboardHandler, LeaderboardScoresDownloaded_t>();
    CCallResult<LeaderboardHandler, LeaderboardScoresDownloaded_t> *download_friends_scores_callback = new CCallResult<LeaderboardHandler, LeaderboardScoresDownloaded_t>();

    download_top_scores_callback->Set(hSteamAPICall_top, this,
        &LeaderboardHandler::onDownloadTopScores);
    download_near_scores_callback->Set(hSteamAPICall_near, this,
        &LeaderboardHandler::onDownloadNearScores);
    download_friends_scores_callback->Set(hSteamAPICall_friends, this,
        &LeaderboardHandler::onDownloadFriendsScores);
}

void LeaderboardHandler::onDownloadTopScores(LeaderboardScoresDownloaded_t *pCallback, bool bIOFailure)
{
    leaderboards_to_download_count--;

    if(bIOFailure)
    {
        rosalila()->utility->writeLogLine("Error downloading top scores");
        this->state="error";
        return;
    }

    rosalila()->utility->writeLogLine("Top scores downloaded");

    //int m_nLeaderboardEntries = std::min(pCallback->m_cEntryCount, 1000);
    int m_nLeaderboardEntries = pCallback->m_cEntryCount;
    LeaderboardEntry_t *m_leaderboardEntries = new LeaderboardEntry_t[m_nLeaderboardEntries];

    vector<LeaderboardEntry*> leaderboard_top_entries;

    for (int index = 0; index < m_nLeaderboardEntries; index++)
    {
        SteamUserStats()->GetDownloadedLeaderboardEntry(
            pCallback->m_hSteamLeaderboardEntries,index,
            &m_leaderboardEntries[index],
            NULL,0);

        LeaderboardEntry* leaderboard_entry = new LeaderboardEntry(
                              (string)SteamFriends()->GetFriendPersonaName( (&m_leaderboardEntries[index])->m_steamIDUser),
                              (&m_leaderboardEntries[index])->m_nGlobalRank,
                              (&m_leaderboardEntries[index])->m_nScore,
                              (&m_leaderboardEntries[index])->m_cDetails);

        leaderboard_top_entries.push_back(leaderboard_entry);

        entries_attachments[leaderboard_entry]=(&m_leaderboardEntries[index])->m_hUGC;
    }

    leaderboards[SteamUserStats()->GetLeaderboardName(pCallback->m_hSteamLeaderboard)]->top_entries = leaderboard_top_entries;

    if(state != "error" && leaderboards_to_download_count == 0)
        state = "finished";
}

void LeaderboardHandler::onDownloadNearScores(LeaderboardScoresDownloaded_t *pCallback, bool bIOFailure)
{
    leaderboards_to_download_count--;

    if(bIOFailure)
    {
        rosalila()->utility->writeLogLine("Error downloading near scores");
        this->state="error";
        return;
    }

    rosalila()->utility->writeLogLine("Near scores downloaded");

    //int m_nLeaderboardEntries = std::min(pCallback->m_cEntryCount, 1000);
    int m_nLeaderboardEntries = pCallback->m_cEntryCount;
    LeaderboardEntry_t *m_leaderboardEntries = new LeaderboardEntry_t[m_nLeaderboardEntries];

    vector<LeaderboardEntry*> leaderboard_near_entries;
    LeaderboardEntry* current_user_global_rank = NULL;

    for (int index = 0; index < m_nLeaderboardEntries; index++)
    {
        SteamUserStats()->GetDownloadedLeaderboardEntry(
            pCallback->m_hSteamLeaderboardEntries,index,
            &m_leaderboardEntries[index],
            NULL,0);

        LeaderboardEntry* leaderboard_entry = new LeaderboardEntry(
                              (string)SteamFriends()->GetFriendPersonaName( (&m_leaderboardEntries[index])->m_steamIDUser),
                              (&m_leaderboardEntries[index])->m_nGlobalRank,
                              (&m_leaderboardEntries[index])->m_nScore,
                              (&m_leaderboardEntries[index])->m_cDetails);

        leaderboard_near_entries.push_back(leaderboard_entry);

        if((&m_leaderboardEntries[index])->m_steamIDUser==SteamUser()->GetSteamID())
        {
            current_user_global_rank = leaderboard_entry;
        }

        entries_attachments[leaderboard_entry]=(&m_leaderboardEntries[index])->m_hUGC;
    }

    leaderboards[SteamUserStats()->GetLeaderboardName(pCallback->m_hSteamLeaderboard)]->near_entries = leaderboard_near_entries;
    leaderboards[SteamUserStats()->GetLeaderboardName(pCallback->m_hSteamLeaderboard)]->leaderboard_self_entry = current_user_global_rank;

    if(state != "error" && leaderboards_to_download_count == 0)
        state = "finished";
}

void LeaderboardHandler::onDownloadFriendsScores(LeaderboardScoresDownloaded_t *pCallback, bool bIOFailure)
{
    leaderboards_to_download_count--;

    if(bIOFailure)
    {
        rosalila()->utility->writeLogLine("Error downloading friends scores");
        this->state="error";
        return;
    }

    rosalila()->utility->writeLogLine("Friends scores downloaded");

    //int m_nLeaderboardEntries = std::min(pCallback->m_cEntryCount, 1000);
    int m_nLeaderboardEntries = pCallback->m_cEntryCount;
    LeaderboardEntry_t *m_leaderboardEntries = new LeaderboardEntry_t[m_nLeaderboardEntries];

    vector<LeaderboardEntry*> leaderboard_friends_entries;

    for (int index = 0; index < m_nLeaderboardEntries; index++)
    {
        SteamUserStats()->GetDownloadedLeaderboardEntry(
            pCallback->m_hSteamLeaderboardEntries,index,
            &m_leaderboardEntries[index],
            NULL,0);

        LeaderboardEntry* leaderboard_entry = new LeaderboardEntry(
                              (string)SteamFriends()->GetFriendPersonaName( (&m_leaderboardEntries[index])->m_steamIDUser),
                              (&m_leaderboardEntries[index])->m_nGlobalRank,
                              (&m_leaderboardEntries[index])->m_nScore,
                              (&m_leaderboardEntries[index])->m_cDetails);

        leaderboard_friends_entries.push_back(leaderboard_entry);

        entries_attachments[leaderboard_entry]=(&m_leaderboardEntries[index])->m_hUGC;
    }

    leaderboards[SteamUserStats()->GetLeaderboardName(pCallback->m_hSteamLeaderboard)]->friends_entries = leaderboard_friends_entries;

    if(state != "error" && leaderboards_to_download_count == 0)
        state = "finished";
}

void LeaderboardHandler::onDownloadUGCComplete(RemoteStorageDownloadUGCResult_t *pCallback, bool bIOFailure)
{
    if(pCallback->m_eResult != k_EResultOK || bIOFailure)
    {
        rosalila()->utility->writeLogLine("Error downloading UGC");
        this->state="error";
        return;
    }

    rosalila()->utility->writeLogLine("Download UGC complete");

    char *retreived_data = new char[pCallback->m_nSizeInBytes];
    SteamRemoteStorage()->UGCRead( pCallback->m_hFile, retreived_data, pCallback->m_nSizeInBytes, 0, EUGCReadAction() );

    for(int i=0;i<(int)leaderboards[pCallback->m_pchFileName]->top_entries.size();i++)
    {
        if(leaderboards[pCallback->m_pchFileName]->top_entries[i]->name==(string)SteamFriends()->GetFriendPersonaName(pCallback->m_ulSteamIDOwner))
        {
            leaderboards[pCallback->m_pchFileName]->top_entries[i]->attachment = retreived_data;
            leaderboards[pCallback->m_pchFileName]->top_entries[i]->attachment_size = pCallback->m_nSizeInBytes;
            leaderboards[pCallback->m_pchFileName]->top_entries[i]->attachment_state = "loaded";
        }
    }

    for(int i=0;i<(int)leaderboards[pCallback->m_pchFileName]->near_entries.size();i++)
    {
        if(leaderboards[pCallback->m_pchFileName]->near_entries[i]->name==(string)SteamFriends()->GetFriendPersonaName(pCallback->m_ulSteamIDOwner))
        {
            leaderboards[pCallback->m_pchFileName]->near_entries[i]->attachment = retreived_data;
            leaderboards[pCallback->m_pchFileName]->near_entries[i]->attachment_size = pCallback->m_nSizeInBytes;
            leaderboards[pCallback->m_pchFileName]->near_entries[i]->attachment_state = "loaded";
        }
    }

    for(int i=0;i<(int)leaderboards[pCallback->m_pchFileName]->friends_entries.size();i++)
    {
        if(leaderboards[pCallback->m_pchFileName]->friends_entries[i]->name==(string)SteamFriends()->GetFriendPersonaName(pCallback->m_ulSteamIDOwner))
        {
            leaderboards[pCallback->m_pchFileName]->friends_entries[i]->attachment = retreived_data;
            leaderboards[pCallback->m_pchFileName]->friends_entries[i]->attachment_size = pCallback->m_nSizeInBytes;
            leaderboards[pCallback->m_pchFileName]->friends_entries[i]->attachment_state = "loaded";
        }
    }

    this->state="finished";
}

void LeaderboardHandler::onStoreLeaderboardUGCComplete(RemoteStorageFileShareResult_t *pCallback, bool bIOFailure)
{
    if(pCallback->m_eResult != k_EResultOK || bIOFailure)
    {
        rosalila()->utility->writeLogLine("Error storing leaderboard UGC");
        this->state="error";
        return;
    }

    rosalila()->utility->writeLogLine("Leaderboard UGC writing complete");
    SteamUserStats()->AttachLeaderboardUGC(
        steam_leaderboards[pCallback->m_rgchFilename],
        pCallback->m_hFile);

    this->state="finished";
}

void LeaderboardHandler::onUploadScore(LeaderboardScoreUploaded_t *pCallback, bool bIOFailure)
{
    if(!pCallback->m_bSuccess || bIOFailure)
    {
        rosalila()->utility->writeLogLine("Downloading entry attachment");
        this->state="error";
        return;
    }

    rosalila()->utility->writeLogLine("Score upload complete");

    this->state="finished";
}

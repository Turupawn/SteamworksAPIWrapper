#include "SteamIntegration.h"

LeaderboardHandler *leaderboard_handler;
string steam_folder = "SavesDir/";

bool is_using_steam = false;
bool is_using_steam_controller = false;
ControllerHandle_t steam_controller_handle;
map<string, ControllerActionSetHandle_t> steam_controller_action_sets;
map<string,ControllerDigitalActionHandle_t> steam_controller_actions;

void steamInit()
{
    if(!SteamAPI_Init())
    {
        rosalila()->utility->writeLogLine("Steam API could not be initialized");
        return;
    }

    //SteamAPICall_t x = SteamUGC()->CreateItem( 517510, k_EWorkshopFileTypeCommunity);

    is_using_steam = true;

    leaderboard_handler=new LeaderboardHandler();


    is_using_steam_controller = true;
    if(!SteamController()->Init())
    {
        is_using_steam_controller = false;
        rosalila()->utility->writeLogLine("Steam controller API could not be initialized");
    }

    //init
    SteamController()->RunFrame();

    if(steamIsUsingSteamController())
    {
        //Controller handle
        ControllerHandle_t controller_handles[STEAM_CONTROLLER_MAX_COUNT];
        SteamController()->GetConnectedControllers(controller_handles);
        steam_controller_handle = controller_handles[0];

        if(steam_controller_handle == 0)
        {
            is_using_steam_controller = false;
        }
    }

    if(steamIsUsingSteamController())
    {
        //Action sets
        steam_controller_action_sets["InGameControls"] = SteamController()->GetActionSetHandle("InGameControls");
        steam_controller_action_sets["MenuControls"] = SteamController()->GetActionSetHandle("MenuControls");

        //Actions
        steam_controller_actions["menu_up"] = SteamController()->GetDigitalActionHandle("menu_up");
        steam_controller_actions["menu_down"] = SteamController()->GetDigitalActionHandle("menu_down");
        steam_controller_actions["menu_left"] = SteamController()->GetDigitalActionHandle("menu_left");
        steam_controller_actions["menu_right"] = SteamController()->GetDigitalActionHandle("menu_right");
        steam_controller_actions["menu_select"] = SteamController()->GetDigitalActionHandle("menu_select");
        steam_controller_actions["menu_cancel"] = SteamController()->GetDigitalActionHandle("menu_cancel");

        steam_controller_actions["up"] = SteamController()->GetDigitalActionHandle("up");
        steam_controller_actions["down"] = SteamController()->GetDigitalActionHandle("down");
        steam_controller_actions["left"] = SteamController()->GetDigitalActionHandle("left");
        steam_controller_actions["right"] = SteamController()->GetDigitalActionHandle("right");
        steam_controller_actions["action"] = SteamController()->GetDigitalActionHandle("action");
        steam_controller_actions["back"] = SteamController()->GetDigitalActionHandle("back");
    }

    if(SteamUser())
    {
        SteamUserStats()->RequestCurrentStats();
        leaderboard_handler->findLeaderboard("Training1");
        leaderboard_handler->findLeaderboard("Training2");
        leaderboard_handler->findLeaderboard("Training3");
        leaderboard_handler->findLeaderboard("Training4");
        leaderboard_handler->findLeaderboard("Training5");
        leaderboard_handler->findLeaderboard("Training6");
        leaderboard_handler->findLeaderboard("A");
        leaderboard_handler->findLeaderboard("B");
        leaderboard_handler->findLeaderboard("C");
        leaderboard_handler->findLeaderboard("D");
        leaderboard_handler->findLeaderboard("E");
        leaderboard_handler->findLeaderboard("F");
        leaderboard_handler->findLeaderboard("A1");
        leaderboard_handler->findLeaderboard("A2");
        leaderboard_handler->findLeaderboard("A3");
        leaderboard_handler->findLeaderboard("B1");
        leaderboard_handler->findLeaderboard("B2");
        leaderboard_handler->findLeaderboard("B3");
        leaderboard_handler->findLeaderboard("C1");
        leaderboard_handler->findLeaderboard("C2");
        leaderboard_handler->findLeaderboard("C3");
        leaderboard_handler->findLeaderboard("C1");
        leaderboard_handler->findLeaderboard("C2");
        leaderboard_handler->findLeaderboard("C3");
        leaderboard_handler->findLeaderboard("D1");
        leaderboard_handler->findLeaderboard("D2");
        leaderboard_handler->findLeaderboard("D3");
        leaderboard_handler->findLeaderboard("E1");
        leaderboard_handler->findLeaderboard("E2");
        leaderboard_handler->findLeaderboard("E3");
        leaderboard_handler->findLeaderboard("F1");
        leaderboard_handler->findLeaderboard("F2");
        leaderboard_handler->findLeaderboard("F3");
        for(int i=1;i<=24;i++)
        {
            string leaderboard_name = "X"+rosalila()->utility->toString(i);
            leaderboard_handler->findLeaderboard(leaderboard_name.c_str());
        }
        for(int i=1;i<=5;i++)
        {
            string leaderboard_name = "Z"+rosalila()->utility->toString(i);
            leaderboard_handler->findLeaderboard(leaderboard_name.c_str());
        }

    }

//    for(int i=1; i<=4; i++)
//        SteamUserStats()->ClearAchievement(string(string("Charge")+rosalila()->utility->toString(i)).c_str());
//    for(int i=1; i<=4; i++)
//        SteamUserStats()->ClearAchievement(string(string("ChargeDestroy")+rosalila()->utility->toString(i)).c_str());
//    for(int i=1; i<=4; i++)
//        SteamUserStats()->ClearAchievement(string(string("Parry")+rosalila()->utility->toString(i)).c_str());
//    for(int i=1; i<=4; i++)
//        SteamUserStats()->ClearAchievement(string(string("ParryDash")+rosalila()->utility->toString(i)).c_str());
//    for(int i=1; i<=4; i++)
//        SteamUserStats()->ClearAchievement(string(string("Clear")+rosalila()->utility->toString(i)).c_str());
//    for(int i=1; i<=4; i++)
//        SteamUserStats()->ClearAchievement(string(string("Perfect")+rosalila()->utility->toString(i)).c_str());
//
//    SteamUserStats()->ResetAllStats(true);
}

bool steamIsUsingApi()
{
    return is_using_steam && SteamUser();
}

void steamSetCurrentControllerActionSet(string action_set_name)
{
    if(is_using_steam)
        SteamController()->ActivateActionSet(steam_controller_handle, steam_controller_action_sets[action_set_name]);
}

bool steamIsControllerActionDown(string action_name)
{
    if(is_using_steam)
    {
        ControllerDigitalActionData_t digital_action_data = SteamController()->GetDigitalActionData(steam_controller_handle, steam_controller_actions[action_name]);
        return digital_action_data.bActive && digital_action_data.bState;
    }
    return false;
}

void steamUnlockAchievement(string achievement_name)
{
    if(is_using_steam && SteamUser())
    {
        SteamUserStats()->SetAchievement(achievement_name.c_str());
        SteamUserStats()->StoreStats();
    }
}

void steamSetScore(string leaderboard_name, double score)
{
    if(is_using_steam && SteamUser())
    {
        leaderboard_handler->uploadScore(leaderboard_name, score);
    }
}

void steamStoreLeaderboardAttachment(string leaderboard_name, char* attachment, int attachment_size)
{
    if(is_using_steam && SteamUser())
    {
        leaderboard_handler->storeLeaderboardUGC(leaderboard_name, attachment, attachment_size);
    }
}

Leaderboard* steamGetLeaderboard(string leaderboard_name)
{
    if(is_using_steam && SteamUser())
    {
        return leaderboard_handler->leaderboards[leaderboard_name];
    }else
    {
        return NULL;
    }
}

void steamFindLeaderboard(string leaderboard_name)
{
    if(is_using_steam && SteamUser())
    {
        leaderboard_handler->findLeaderboard(leaderboard_name.c_str());
    }
}

void steamDownloadEntryAttachment(LeaderboardEntry* leaderboard_entry)
{
    if(is_using_steam && SteamUser())
    {
        leaderboard_handler->downloadEntryAttachment(leaderboard_entry);
    }
}

void steamSetStat(string stat_name, double stat)
{
    if(is_using_steam && SteamUser())
    {
        SteamUserStats()->SetStat(stat_name.c_str(), (int32)stat);
        SteamUserStats()->StoreStats();
    }
}

double steamGetStat(string stat_name)
{
    if(is_using_steam && SteamUser())
    {
        int32* stat_fetch = new int32(0);
        SteamUserStats()->GetStat(stat_name.c_str(), stat_fetch);
        double return_value = *stat_fetch;
        delete stat_fetch;
        return return_value;
    }else
    {
        return 0;
    }
}

void steamStoreData(string key, string data)
{
    if(is_using_steam && SteamUser())
    {
        string file_name = steam_folder+key+".sav";
        ofstream out(file_name.c_str());
        out<<data<<endl;
        out.close();
    }
}

string steamGetData(string key)
{
    if(is_using_steam && SteamUser())
    {
        string file_name = steam_folder+key+".sav";
        ifstream in(file_name.c_str());
        string retreived_data="";
        string retrived_stream="";
        while(in>>retrived_stream)
        {
            retreived_data+=retrived_stream;
        }
        in.close();
        return retreived_data;
    }else
    {
        return "";
    }
}


void steamUpdateCallbacks()
{
    if(is_using_steam && SteamUser())
    {
        SteamAPI_RunCallbacks();
    }
    if(is_using_steam_controller)
        SteamController()->RunFrame();
}

string steamGetState()
{
    if(is_using_steam && SteamUser())
        return leaderboard_handler->state;
    return "finished";
}

bool steamIsUsingSteamController()
{
    return is_using_steam_controller;
}

void steamShowControllerBindingPanel()
{
    if(is_using_steam)
        SteamController()->ShowBindingPanel(steam_controller_handle);
}

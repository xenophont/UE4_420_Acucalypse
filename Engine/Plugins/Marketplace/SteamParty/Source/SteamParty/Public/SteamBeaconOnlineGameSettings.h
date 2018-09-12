// Copyright 2013-2017 W3 Studios, LLC. All Rights Reserved.

#pragma once
#include "Online.h"

//General session settings for a Steam Party Beacon Plugin
class FSteamBeaconOnlineSessionSettings : public FOnlineSessionSettings
{
public:

	FSteamBeaconOnlineSessionSettings(bool bIsLAN = false, bool bIsPresence = false, int32 MaxNumPlayers = 4);
	virtual ~FSteamBeaconOnlineSessionSettings() {}

	void CopyFromBase(FOnlineSessionSettings* BaseSource)
	{
		NumPublicConnections = BaseSource->NumPublicConnections;
		NumPrivateConnections = BaseSource->NumPrivateConnections;
		bShouldAdvertise = BaseSource->bShouldAdvertise;
		bAllowJoinInProgress = BaseSource->bAllowJoinInProgress;
		bIsLANMatch = BaseSource->bIsLANMatch;
		bIsDedicated = BaseSource->bIsDedicated;
		bUsesStats = BaseSource->bUsesStats;
		bAllowInvites = BaseSource->bAllowInvites;
		bUsesPresence = BaseSource->bUsesPresence;
		bAllowJoinViaPresence = BaseSource->bAllowJoinViaPresence;
		bAllowJoinViaPresenceFriendsOnly = BaseSource->bAllowJoinViaPresenceFriendsOnly;
		bAntiCheatProtected = BaseSource->bAntiCheatProtected;
		BuildUniqueId = BaseSource->BuildUniqueId;
		Settings = BaseSource->Settings;
	}
};


//General search setting for the Steam Beacon Plugin
class FSteamBeaconOnlineSearchSettings : public FOnlineSessionSearch
{
public:
	FSteamBeaconOnlineSearchSettings(bool bSearchingLAN = false, bool bSearchingPresence = false);
	virtual ~FSteamBeaconOnlineSearchSettings() {}
};


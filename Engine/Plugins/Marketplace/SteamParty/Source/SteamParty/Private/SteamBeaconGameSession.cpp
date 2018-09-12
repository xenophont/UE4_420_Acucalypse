// Copyright 2013-2017 W3 Studios, LLC. All Rights Reserved.

#include "SteamBeaconGameSession.h"
#include "SteamParty.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemTypes.h"
#include "SteamBeaconGameInstance.h"
#include "SteamSessionKeys.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/CommandLine.h"


//Session Handling for Steam Party Beacons
ASteamBeaconGameSession::ASteamBeaconGameSession(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		//Session Delegates
		OnFindFriendSessionCompletedDelegate = FOnFindFriendSessionCompleteDelegate::CreateUObject(this, &ASteamBeaconGameSession::OnFindFriendSessionsCompletedCallback);
		OnFindSessionsCompletedDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &ASteamBeaconGameSession::OnFindSessionsCompletedCallback);

		
		//Party Delegates
		OnCreatePartySessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &ASteamBeaconGameSession::OnCreatePartySessionComplete);
		OnDestroyPartySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &ASteamBeaconGameSession::OnDestroyPartySessionComplete);

		//Game Delegates
		OnCreateGameSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &ASteamBeaconGameSession::OnCreateGameSessionComplete);
		OnJoinGameSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &ASteamBeaconGameSession::OnJoinGameSessionComplete);
	}
}

//Retrieve the search results
TArray<FOnlineSessionSearchResult>& ASteamBeaconGameSession::GetSessionSearchResults()
{
	return SteamBeaconSearchSettings->SearchResults;
};

//Find session from steam friend
void ASteamBeaconGameSession::FindFriendSession(const FUniqueNetId& Friend)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		//Make sure delegate isn't already doing a search, causes crash if not checked
		if (Sessions.IsValid())
		{
			//Bind the Delegate and Search
			if (!OnFindFriendSessionsCompletedDelegateHandle.IsValid())
			{
				OnFindFriendSessionsCompletedDelegateHandle = Sessions->AddOnFindFriendSessionCompleteDelegate_Handle(0, OnFindFriendSessionCompletedDelegate);
				Sessions->FindFriendSession(0, Friend);
			}
			else
			{
				UE_LOG(LogSteamBeacon, Warning, TEXT("ASteamBeaconGameSession::FindSessionFromSessionId OnFindFriendSessionsCompletedDelegateHandle AlreadyBound!, Search Already started"));
			}
		}
	}
}

//Called after steam finished "FindFriendSession"
void ASteamBeaconGameSession::OnFindFriendSessionsCompletedCallback(int32 LocalPlayerNum, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SearchResult)
{
	UE_LOG(LogSteamBeacon, Verbose, TEXT("OnFindFriendSessionsCompletedCallback bSuccess: %d"), bWasSuccessful);

	//Remove Delegate binding
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			if (OnFindFriendSessionsCompletedDelegateHandle.IsValid())
			{
				Sessions->ClearOnFindFriendSessionCompleteDelegate_Handle(LocalPlayerNum, OnFindFriendSessionsCompletedDelegateHandle);
				OnFindFriendSessionsCompletedDelegateHandle.Reset();
			}
		}
	}

	//Manually trigger callback on gameinstance to join the party beacon, avoiding parallel code paths!
	if (bWasSuccessful)
	{
		// Can't just use SearchResult.IsValid() here - it's possible the SessionInfo pointer is valid, but not the data until we actually join the session
		if (SearchResult.Num() > 0 && SearchResult[0].Session.OwningUserId.IsValid() && SearchResult[0].Session.SessionInfo.IsValid())
		{
			USteamBeaconGameInstance* pGI = Cast<USteamBeaconGameInstance>(GetGameInstance());
			if (pGI)
			{
				//Only the first result matters as we are searching 1 friend
				pGI->OnSessionUserInviteAccepted(bWasSuccessful, LocalPlayerNum, SearchResult[0].Session.OwningUserId, SearchResult[0]);
			}
		}
		else
		{
			UE_LOG(LogSteamBeacon, Warning, TEXT("OnFindFriendSessionsCompletedCallback: Join friend returned no search result."));
		}
	}
}



//Find Session from the ID
void ASteamBeaconGameSession::FindSessionFromSessionId(const FSteamBeaconSessionResult& InSearchResult)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		//Make sure delegate isn't already doing a search, causes crash if not checked
		if (Sessions.IsValid())
		{
			SteamBeaconSearchSettings = MakeShareable(new FSteamBeaconOnlineSearchSettings(InSearchResult.bIsLan, InSearchResult.bIsPresence));
									
			//Presence, some one is hosting a listen server, use their name to find query against the OwningID
			if (InSearchResult.bIsPresence)
			{
				//Only Search for LobbyId or GameServerId, STEAMKEY_OWNINGUSERNAME, "NM", the queryparser will auto add "_s" so its "NM_s" , hacky for now
				//SteamBeaconSearchSettings->QuerySettings.Set("NM", InSearchResult.SessionId, EOnlineComparisonOp::Equals);
				//SteamBeaconSearchSettings->QuerySettings.Set(STEAMKEY_OWNINGUSERID, InSearchResult.SessionId, EOnlineComparisonOp::Equals);
				SteamBeaconSearchSettings->QuerySettings.Set(FName(TEXT("SESSIONID")), InSearchResult.SessionId, EOnlineComparisonOp::Equals);

				//Exclude Party Sessions! important as it will hang on the search callback for 15 seconds and timeout
				SteamBeaconSearchSettings->QuerySettings.Set(SETTING_GAMEMODE, FString(TEXT("PARTY")), EOnlineComparisonOp::NotEquals);
			}
			else
			{
				//Dedicated
				//SEARCH_STEAM_HOSTIP = TEXT("SteamHostIp"), define is buried
				SteamBeaconSearchSettings->QuerySettings.Set(FName(TEXT("SteamHostIp")), InSearchResult.HostAddr, EOnlineComparisonOp::Equals);
			}

			//Bind the Delegate and Search
			if (!OnFindSessionsCompletedDelegateHandle.IsValid())
			{
				OnFindSessionsCompletedDelegateHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompletedDelegate);
				Sessions->FindSessions(0, SteamBeaconSearchSettings.ToSharedRef());
			}
			else
			{
				UE_LOG(LogSteamBeacon, Warning, TEXT("ASteamBeaconGameSession::FindSessionFromSessionId OnFindSessionsCompletedDelegateHandle AlreadyBound!, Search Already started"));
			}
		}
	}
	else
	{
		OnFindSessionsCompletedCallback(false);
	}
}

//Callback from steam after "FindSessions" is complete
void ASteamBeaconGameSession::OnFindSessionsCompletedCallback(bool bWasSuccessful)
{
	UE_LOG(LogSteamBeacon, Verbose, TEXT("OnFindSessionsCompletedCallback bSuccess: %d"), bWasSuccessful);

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			if (OnFindSessionsCompletedDelegateHandle.IsValid())
			{
				Sessions->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompletedDelegateHandle);
				OnFindSessionsCompletedDelegateHandle.Reset();
			}

			UE_LOG(LogSteamBeacon, Verbose, TEXT("Num Search Results: %d"), SteamBeaconSearchSettings->SearchResults.Num());
			if (SteamBeaconSearchSettings->SearchResults.Num())
			{
				for (int32 SearchIdx = 0; SearchIdx < SteamBeaconSearchSettings->SearchResults.Num(); SearchIdx++)
				{
					const FOnlineSessionSearchResult& SearchResult = SteamBeaconSearchSettings->SearchResults[SearchIdx];
					DumpSession(&SearchResult.Session);
				}
			}
		}
	}

	//Broadcast Event to any attached delegates
	FindSessionsCompletedEvent.Broadcast(bWasSuccessful);
}

//Begin the Party Session
bool ASteamBeaconGameSession::HostPartySession(const FUniqueNetIdRepl& UserId, FName InSessionName, int32 MaxNumPlayers)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid() && UserId.IsValid())
		{
			//Get current default beacon listen port
			int32 BeaconListenPort;
			GConfig->GetInt(TEXT("/Script/OnlineSubsystemUtils.OnlineBeaconHost"), TEXT("ListenPort"), BeaconListenPort, GEngineIni);
			// Allow the command line to override the default port
			int32 PortOverride;
			if (FParse::Value(FCommandLine::Get(), TEXT("BeaconPort="), PortOverride) && PortOverride != 0)
			{
				BeaconListenPort = PortOverride;
			}
			
			SteamBeaconPartyHostSettings = MakeShareable(new FSteamBeaconOnlineSessionSettings(false, true, MaxNumPlayers));
			SteamBeaconPartyHostSettings->Set(SETTING_BEACONPORT, BeaconListenPort, EOnlineDataAdvertisementType::ViaOnlineService);
			
			//Name the GameMode "PARTY" for easier query filtering for Party Lobbies
			SteamBeaconPartyHostSettings->Set(SETTING_GAMEMODE, FString(TEXT("PARTY")), EOnlineDataAdvertisementType::ViaOnlineService);
			
			if (!OnCreatePartySessionCompleteDelegateHandle.IsValid())
			{
				OnCreatePartySessionCompleteDelegateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(OnCreatePartySessionCompleteDelegate);
				return Sessions->CreateSession(*UserId, InSessionName, *SteamBeaconPartyHostSettings);
			}
			else
			{
				UE_LOG(LogSteamBeacon, Warning, TEXT("OnCreatePartySessionCompleteDelegateHandle AlreadyBound!, CreateSession Already in Process"));
			}
		}
	}
	return false;
}

//Callback from Steam when the Party Session is created
void ASteamBeaconGameSession::OnCreatePartySessionComplete(FName InSessionName, bool bWasSuccessful)
{
	UE_LOG(LogSteamBeacon, Verbose, TEXT("OnCreatePartySessionComplete %s bSuccess: %d"), *InSessionName.ToString(), bWasSuccessful);

	//Remove our Delegate Registered Online
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub && OnCreatePartySessionCompleteDelegateHandle.IsValid())
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->ClearOnCreateSessionCompleteDelegate_Handle(OnCreatePartySessionCompleteDelegateHandle);
		}
		OnCreatePartySessionCompleteDelegateHandle.Reset();
	}

	//Send to all our Registered Event Delegates
	CreatePresencePartySessionCompleteEvent.Broadcast(InSessionName, bWasSuccessful);
}

//Destroy the Party Session
bool ASteamBeaconGameSession::DestroyPartySession(FName InSessionName)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			if (!OnDestroyPartySessionCompleteDelegateHandle.IsValid())
			{
				OnDestroyPartySessionCompleteDelegateHandle = Sessions->AddOnDestroySessionCompleteDelegate_Handle(OnDestroyPartySessionCompleteDelegate);
				return Sessions->DestroySession(InSessionName);
			}
			else
			{
				UE_LOG(LogSteamBeacon, Warning, TEXT("OnDestroyPartySessionCompleteDelegateHandle AlreadyBound!, DestroySession Already in Process"));
			}
		}
	}
	return false;
}

//Steam callback when the Party Session has been destroyed
void ASteamBeaconGameSession::OnDestroyPartySessionComplete(FName InSessionName, bool bWasSuccessful)
{
	UE_LOG(LogSteamBeacon, Verbose, TEXT("OnDestroyPartySessionComplete %s bSuccess: %d"), *InSessionName.ToString(), bWasSuccessful);

	//Remove our Delegate Registered Online
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub && OnDestroyPartySessionCompleteDelegateHandle.IsValid())
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroyPartySessionCompleteDelegateHandle);
		}
		OnDestroyPartySessionCompleteDelegateHandle.Reset();
	}
}

//Game Sessions Not Party Sessions
bool ASteamBeaconGameSession::HostGameSession(const FUniqueNetIdRepl& UserId, int32 MaxNumPlayers)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid() && UserId.IsValid())
		{
			TSharedPtr<FSteamBeaconOnlineSessionSettings> SteamBeaconGameHostSettings = MakeShareable(new FSteamBeaconOnlineSessionSettings(false, true, MaxNumPlayers));
			//Add our custom search key since the default engine steam implementation is broken between the query parameter and created names
			SteamBeaconGameHostSettings->Set(FName(TEXT("SESSIONID")), UserId->ToString(), EOnlineDataAdvertisementType::ViaOnlineService);
			
			//Name the GameMode "TYPE" for easier query filtering for Party Lobbies
			SteamBeaconGameHostSettings->Set(SETTING_GAMEMODE, FString(TEXT("DeathMatch")), EOnlineDataAdvertisementType::ViaOnlineService);

			if (!OnCreateGameSessionCompleteDelegateHandle.IsValid())
			{
				OnCreateGameSessionCompleteDelegateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(OnCreateGameSessionCompleteDelegate);
				return Sessions->CreateSession(*UserId, GameSessionName, *SteamBeaconGameHostSettings);
			}
			else
			{
				UE_LOG(LogSteamBeacon, Warning, TEXT("OnCreatePartySessionCompleteDelegateHandle AlreadyBound!, CreateSession Already in Process"));
			}
		}
	}
	return false;
}

//Steam callback when Game Sesssion has been created
void ASteamBeaconGameSession::OnCreateGameSessionComplete(FName InSessionName, bool bWasSuccessful)
{
	UE_LOG(LogSteamBeacon, Verbose, TEXT("OnCreateGameSessionComplete %s bSuccess: %d"), *InSessionName.ToString(), bWasSuccessful);

	//Remove our Delegate Registered Online
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub && OnCreateGameSessionCompleteDelegateHandle.IsValid())
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateGameSessionCompleteDelegateHandle);
		}
		OnCreateGameSessionCompleteDelegateHandle.Reset();
	}

	//Send to all our Registered Event Delegates
	CreatePresenceGameSessionCompleteEvent.Broadcast(InSessionName, bWasSuccessful);
}

//Joining Sessions
bool ASteamBeaconGameSession::JoinGameSession(const FUniqueNetIdRepl& UserId, const FOnlineSessionSearchResult& SearchResult)
{
	bool bResult = false;

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid() && UserId.IsValid())
		{
			if (!OnJoinGameSessionCompleteDelegateHandle.IsValid())
			{
				OnJoinGameSessionCompleteDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(OnJoinGameSessionCompleteDelegate);
				bResult = Sessions->JoinSession(*UserId, GameSessionName, SearchResult);
			}
			else
			{
				UE_LOG(LogSteamBeacon, Warning, TEXT("OnJoinGameSessionCompleteDelegateHandle AlreadyBound!"));
			}
		}
	}

	return bResult;
}

//Delegate fired when the joining process for an online session has completed
void ASteamBeaconGameSession::OnJoinGameSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(LogSteamBeacon, Verbose, TEXT("OnJoinGameSessionComplete %s bSuccess: %d"), *InSessionName.ToString(), static_cast<int32>(Result));
	
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	IOnlineSessionPtr Sessions = nullptr;
	if (OnlineSub)
	{
		Sessions = OnlineSub->GetSessionInterface();
		if (OnJoinGameSessionCompleteDelegateHandle.IsValid())
		{
			Sessions->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinGameSessionCompleteDelegateHandle);
			OnJoinGameSessionCompleteDelegateHandle.Reset();
		}
	}

	JoinGameSessionCompleteEvent.Broadcast(Result);
}
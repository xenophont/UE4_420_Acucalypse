// Copyright 2013-2017 W3 Studios, LLC. All Rights Reserved.

#pragma once

#include "GameFramework/GameSession.h"
#include "SteamBeaconOnlineGameSettings.h"
#include "SteamBeaconTypes.h"
#include "SteamBeaconGameSession.generated.h"

//Session Handling for Steam Party Beacons
UCLASS()
class STEAMPARTY_API ASteamBeaconGameSession : public AGameSession
{
	GENERATED_UCLASS_BODY()
	
public:

	//Get Session info from friend id, used to connectin info from Party Invite
	virtual void FindFriendSession(const FUniqueNetId& Friend);
	
	//Search Sessions to find specific SessionId
	virtual void FindSessionFromSessionId(const FSteamBeaconSessionResult& InSearchResult);
	
	//Handle for OnFindFriendSessionsComplete Delegate
	FDelegateHandle OnFindFriendSessionsCompletedDelegateHandle;
	//Handle for OnFindSessionsComplete Delegate
	FDelegateHandle OnFindSessionsCompletedDelegateHandle;

	//Search Setting and Container, Will be filled in from Async Steam Task
	TSharedPtr<FSteamBeaconOnlineSearchSettings> SteamBeaconSearchSettings;
	// Transient properties of a Party Session during game creation/matchmaking */
	//FAbatronGameSessionParams SteamBeaconCurrentPartySessionParams;
	// Current Party Host settings 
	TSharedPtr<FSteamBeaconOnlineSessionSettings> SteamBeaconPartyHostSettings;


	//Event triggered after session search completes
	DECLARE_EVENT_OneParam(ASteamBeaconGameSession, FOnFindSessionsComplete, bool /*bWasSuccessful*/);
	FOnFindSessionsComplete FindSessionsCompletedEvent;

	//Retrieve Search Results
	TArray<FOnlineSessionSearchResult>& GetSessionSearchResults();

protected:
		
	// Delegate for searching for FRIEND sessions 
	FOnFindFriendSessionCompleteDelegate OnFindFriendSessionCompletedDelegate;

	// Delegate for searching for sessions 
	FOnFindSessionsCompleteDelegate OnFindSessionsCompletedDelegate;

	//Callback fired when a session search query has completed
	///@param bWasSuccessful true if the async action completed without error, false if there was an error
	virtual void OnFindFriendSessionsCompletedCallback(int32 LocalPlayerNum, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SearchResult);

	//Callback fired when a session search query has completed
	///@param bWasSuccessful true if the async action completed without error, false if there was an error
	virtual void OnFindSessionsCompletedCallback(bool bWasSuccessful);


	//~~~~~~~~~~ Party Sessions
protected:

	//Delegate fired when a session create request has completed
	///@param SessionName the name of the session this callback is for
	///@param bWasSuccessful true if the async action completed without error, false if there was an error
	virtual void OnCreatePartySessionComplete(FName InSessionName, bool bWasSuccessful);

	// Delegate for creating a new session 
	FOnCreateSessionCompleteDelegate OnCreatePartySessionCompleteDelegate;
	FDelegateHandle OnCreatePartySessionCompleteDelegateHandle;

	//Delegate fired when a session destroy request has completed
	///@param SessionName the name of the session this callback is for
	///@param bWasSuccessful true if the async action completed without error, false if there was an error
	virtual void OnDestroyPartySessionComplete(FName InSessionName, bool bWasSuccessful);

	// Delegate for destroying a session 
	FOnDestroySessionCompleteDelegate OnDestroyPartySessionCompleteDelegate;
	FDelegateHandle OnDestroyPartySessionCompleteDelegateHandle;

public:
	//Event triggered when a presence PARTY session is created
	///@param SessionName name of session that was created
	///@param bWasSuccessful was the create successful
	DECLARE_EVENT_TwoParams(ASteamBeaconGameSession, FOnCreatePresencePartySessionComplete, FName /*SessionName*/, bool /*bWasSuccessful*/);
	FOnCreatePresencePartySessionComplete CreatePresencePartySessionCompleteEvent;
	
	//Host a new online party session
	///@param UserId user that initiated the request
	///@param SessionName name of session
	///@param MaxNumPlayers Maximum number of players to allow in the session
	///@return bool true if successful, false otherwise
	bool HostPartySession(const FUniqueNetIdRepl& UserId, FName InSessionName, int32 MaxNumPlayers);

	//Destroy online PARTY session
	///@param SessionName name of session
	///@return bool true if successful, false otherwise
	bool DestroyPartySession(FName InSessionName);

	//~~~~~~~~~~ Game Sessions
public:

	//Host a new online GAME session
	///@param UserId user that initiated the request
	///@param MaxNumPlayers Maximum number of players to allow in the session
	///@return bool true if successful, false otherwise
	bool HostGameSession(const FUniqueNetIdRepl& UserId, int32 MaxNumPlayers);

	//Event triggered when a presence GAME session is created
	///@param SessionName name of session that was created
	///@param bWasSuccessful was the create successful
	DECLARE_EVENT_TwoParams(ASteamBeaconGameSession, FOnCreatePresenceGameSessionComplete, FName /*SessionName*/, bool /*bWasSuccessful*/);
	FOnCreatePresenceGameSessionComplete CreatePresenceGameSessionCompleteEvent;


	//Joins a GAME session from a search result
	///@param Id of player to join
	///@param SearchResult Session to join
	///@return bool true if successful, false otherwise
	bool JoinGameSession(const FUniqueNetIdRepl& UserId, const FOnlineSessionSearchResult& SearchResult);

	//Event triggered when a presence GAME session is created
	///@param SessionName name of session that was joined
	///@param Result was the create successful
	DECLARE_EVENT_OneParam(ASteamBeaconGameSession, FOnJoinGameSessionComplete, EOnJoinSessionCompleteResult::Type /*Result*/);
	FOnJoinGameSessionComplete JoinGameSessionCompleteEvent;

protected:
	
	//Delegate fired when a session create request has completed
	///@param SessionName the name of the session this callback is for
	///@param bWasSuccessful true if the async action completed without error, false if there was an error
	virtual void OnCreateGameSessionComplete(FName InSessionName, bool bWasSuccessful);

	//Delegate for creating a new GAME session 
	FOnCreateSessionCompleteDelegate OnCreateGameSessionCompleteDelegate;
	FDelegateHandle OnCreateGameSessionCompleteDelegateHandle;

	//Delegate fired when a session join request has completed
	///@param SessionName the name of the session this callback is for
	///@param Result true if the async action completed without error, false if there was an error
	void OnJoinGameSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type Result);

	//Delegate after joining a session 
	FOnJoinSessionCompleteDelegate OnJoinGameSessionCompleteDelegate;
	FDelegateHandle OnJoinGameSessionCompleteDelegateHandle;
};

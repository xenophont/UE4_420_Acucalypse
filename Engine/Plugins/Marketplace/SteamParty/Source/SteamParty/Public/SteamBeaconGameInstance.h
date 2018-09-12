// Copyright 2013-2017 W3 Studios, LLC. All Rights Reserved.

#pragma once

#include "Engine/GameInstance.h"
#include "SteamBeaconTypes.h"
#include "SteamBeaconHost.h"
#include "SteamBeaconClient.h"
#include "SteamBeaconListener.h"
#include "SteamBeaconGameSession.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameFramework/PlayerState.h"
#include "FindSessionsCallbackProxy.h"
#include "SteamBeaconGameInstance.generated.h"

//Delegate fired when party state has changed and clients need to refresh their data
DECLARE_EVENT(USteamBeaconGameInstance, FOnPartyJoiningGame);

//Derived Game Instance Class that has built in handling for the Steam Beacons System
UCLASS(config = Game)
class STEAMPARTY_API USteamBeaconGameInstance : public UGameInstance
{
public:
	GENERATED_UCLASS_BODY()

	//Overridden to Bind and UnBind delegates
	virtual void Init() override;
	virtual void Shutdown() override;

	//Creates the Hosting Beacon backend and joins the local player as the Party Leader
	virtual void InitBeaconHost();

	//Creates the Client Beacon backend and connects the local player to the desired session
	virtual void InitBeaconClient(const FOnlineSessionSearchResult& DesiredHost);
	
	//Creates the Client Beacon for the PartyHost
	virtual void InitBeaconClientForServerPlayer();

	//Max Party Size
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steam Party Settings")
		int32 MaxPartyCount = 8;

	// Class to use for lobby beacon player states 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steam Party Settings")
		TSubclassOf<ALobbyBeaconPlayerState> LobbyBeaconPlayerStateClass;

	//Enable Party Voice Chat
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steam Party Settings")
		bool VoiceChatEnabled = false;

	//Is the Party Host also creating the Game Server as a listen server?
	UPROPERTY(BlueprintReadWrite, Category = "Party")
		bool IsPartyHostCreatingServer = false;

	//Delay to wait before connecting party members to the PartyLeader Hosted Game, Party Host needs some time to loadup map
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steam Party Settings")
		float PartyHostGameTravelDelay = 3.0f;

	//Max number of retry attempts to connect to party host game session, total connection timeout = (PartyHostGameTravelDelay*PartyHostGameTravelMaxRetries)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steam Party Settings")
		int32 PartyHostGameTravelMaxRetries = 30;

	//Current retry count
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
		int32 CurrentTravelRetries = 0;
	
	//Handles Creating a new Party Host
	UFUNCTION(BlueprintCallable, Category = "Party")
		bool HostParty(int32 MaxPlayers);

	//Disband Party
	UFUNCTION(BlueprintCallable, Category = "Party")
		void DisbandParty();

	//Current Party Status, are we in a party?
	UPROPERTY(BlueprintReadWrite, Category = "Party")
		EPartyStatus PartyStatus = EPartyStatus::PartyNone;

	//Is Player in Party : No Splitscreen support here.
	UFUNCTION(BlueprintPure, Category = "Party")
		bool IsInParty() { return PartyStatus != EPartyStatus::PartyNone; }

	//Is Player in Party : No Splitscreen support here.
	UFUNCTION(BlueprintPure, Category = "Party")
		bool IsPartyLeader() { return PartyStatus == EPartyStatus::PartyLeader; }

	//Return the Player Id to string, Made for Blueprints and UMG sorting options
	UFUNCTION(BlueprintPure, Category = "Party")
		FString GetPlayerIdString(const FUniqueNetIdRepl& UniqueNetId) { return UniqueNetId.ToString(); }
		
	//Current Active Party Beacon Host Manager
	UPROPERTY(BlueprintReadWrite, Category = "Party")
		ASteamBeaconHost* BeaconHostObject = nullptr;

	//Current Active Party Beacon Host Listener
	UPROPERTY(BlueprintReadWrite, Category = "Party")
		ASteamBeaconListener* BeaconHostListener = nullptr;

	//Current Active Party Beacon Client
	UPROPERTY(BlueprintReadWrite, Category = "Party")
		ASteamBeaconClient* BeaconClient = nullptr;
	
	//Dump Party Info in Log file, use (-log -logcmds="logsteambeacon verbose") on command line
	UFUNCTION(BlueprintCallable, Category = "SteamBeacon")
		virtual void DumpDebugParty();

	//Sends an Invite to the current online party session to a friend
	UFUNCTION(BlueprintCallable, Category = "SteamBeacon")
		bool SendPartySessionInviteToFriend(APlayerController *PlayerController, const FUniqueNetIdRepl& FriendUniqueNetId);

	//Accept last party invite received
	UFUNCTION(BlueprintCallable, Category = "SteamBeacon")
	virtual void AcceptPartyInvite();

	FUniqueNetIdSteam LobbyIdInvite;
	FUniqueNetIdSteam FriendIdInvite;

	//Get Steam Friends Avatar
	UFUNCTION(BlueprintCallable, Category = "SteamBeacon")
		UTexture2D* GetSteamFriendAvatar(const FUniqueNetIdRepl& FriendUniqueNetId, ESteamAvatarSize InAvatarSize = ESteamAvatarSize::AvatarMedium);

	
	//Send the entire party to the requested steam session information
	//UFUNCTION(BlueprintCallable, Category = "SteamBeacon")  -- Blueprints don't understand FOnlineSessionSearchResult
		virtual bool JoinPartyToSession(FOnlineSessionSearchResult& InSearchResult);

	//Send the entire party to the requested steam session information Blueprint Version
	UFUNCTION(BlueprintCallable, Category = "SteamBeacon")  //-- Blueprints don't understand FOnlineSessionSearchResult
		virtual void JoinPartyToSessionBP(UPARAM(ref) FBlueprintSessionResult& InSearchResult);

	//Initiates joining a session from the Party Host, Will start a Session Search to find the requested gameserver/lobby
	UFUNCTION(BlueprintCallable, Category = "SteamBeacon")
		virtual bool JoinSessionFromSteamBeacon(const FSteamBeaconSessionResult& InSearchResult, bool bIsPartyHostCreatingServer);
		
	// Callback upon finding sessions from "JoinSessionFromSteamBeacon"
	void OnSearchSessionsFromSteamBeaconCompleted(bool bWasSuccessful);
	
	//Save off Party Info when doing Hard Travel
	UFUNCTION(BlueprintCallable, Category = "Party")
	virtual void SavePartyInfo();

	//Restore Party Info when resuming from Hard Travel
	UFUNCTION(BlueprintCallable, Category = "Party")
	virtual void RestorePartyInfo();

	//Get Number Of Members in Party
	UFUNCTION(BlueprintCallable, Category = "Party")
		virtual int32 GetPartyCount();

	//Get Maximum Number Of Possible Members in a Party
	UFUNCTION(BlueprintCallable, Category = "Party")
		virtual int32 GetMaxPartyCount() { return MaxPartyCount; }

	//Is the Party at the Max Party Member Count 
	UFUNCTION(BlueprintPure, Category = "Party")
		virtual bool IsPartyFull() { return (GetPartyCount() >= MaxPartyCount); }

	
	//Handles Creating a new Party Host
	UFUNCTION(BlueprintCallable, Category = "Game")
		bool HostGame(int32 MaxPlayers);

	//Handles Destroying GameSession
	UFUNCTION(BlueprintCallable, Category = "Game")
		void DestroyGameSession();
	
	//Get Current GameSession of Host as String
	UFUNCTION(BlueprintCallable, Category = "Game")
		FString GetHostCurrentGameSessionId();

	//Get Current GameSession of Host and join the party, this is for listen servers not dedicated
	UFUNCTION(BlueprintCallable, Category = "Game")
		bool JoinPartyToHostGameSession();

	//Event triggered when a presence GAME session is created
	///@param bWasSuccessful was the create successful
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCreateGameSessionCompleted, bool, bWasSuccessful);
	
	//Blueprint exposted Delegate
	UPROPERTY(BlueprintAssignable, Category = "Game")
		FOnCreateGameSessionCompleted OnCreateGameSessionCompleted;
		
	//Event triggered when joing GAME session is complete
	///@param bWasSuccessful was the join successful
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJoinGameSessionComplete, bool, bWasSuccessful);

	//Blueprint exposted Delegate
	UPROPERTY(BlueprintAssignable, Category = "Game")
		FOnJoinGameSessionComplete OnJoinGameSessionComplete;

	//Retrieve Game Session
	ASteamBeaconGameSession* GetSteamBeaconGameSession();

	//Destroy any existing beacons
	UFUNCTION(BlueprintCallable, Category = "SteamBeacon")
	virtual void ClearPartyBeacons();

	//Steam Beacon Player State Data Hook, Override for specific game project variables to be replicated to party members
	//Overridable in Blueprints for launcher build access
	UFUNCTION(BlueprintNativeEvent, Category = "SteamBeacon")
		void AddCustomPlayerStateData(ASteamBeaconPlayerState* InPlayerState);
	
	//C++ default implementation, make sure to call this if overridden in BaseClass, no longer required to declare in header file...Here for reference
	//virtual void AddCustomPlayerStateData_Implementation(ASteamBeaconPlayerState* InPlayerState);

	//Access PartyJoingGameEvent Delegate
	FORCEINLINE FOnPartyJoiningGame& OnPartyJoiningGameEvent() { return PartyJoiningGameEvent; }

	//Request to remove steam session with player
	void RemoveSteamP2PSession(CSteamID* SteamPlayer);
	void RemoveSteamP2PSession(APlayerState* InPlayerState);
	TArray<CSteamID> QueueRemoveSteamP2PSessions;

	//Blueprint exposted P2P session remove
	UFUNCTION(BlueprintCallable, Category = "Game")
		virtual void RemoveSteamPlayerSession(APlayerState* InPlayerState);

protected:
	//Remove next Steam P2P session that was queued
	void ProcessRemoveSteamP2PSession();

	//Delegate Handle for Create Party Session
	FDelegateHandle OnCreatePresencePartySessionCompleteDelegateHandle;
	// Callback which is intended to be called upon Party session creation 
	void OnCreatePresencePartySessionComplete(FName SessionName, bool bWasSuccessful);
	
	//Delegate Handle for SteamBeacon Search
	FDelegateHandle OnSearchSessionsFromSteamBeaconCompletedDelegateHandle;
	FString SessionIdRequested;

	
	// Session invite accepted delegates
	FOnSessionUserInviteAcceptedDelegate SessionInviteAcceptedDelegate;
	FDelegateHandle SessionInviteAcceptedDelegateHandle;
	//Callback when User Accepts Steam Invite
	virtual void OnSessionUserInviteAccepted(const bool bWasSuccess, const int32 ControllerId, TSharedPtr<const FUniqueNetId > UserId, const FOnlineSessionSearchResult & InviteResult);	

	// Delgates For Reading Friends List
	FOnReadFriendsListComplete ReadFriendsListCompleteDelegate;
	//Callback when User Request Friends List
	virtual void OnReadFriendsListComplete(int32 LocalPlayerNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);

	//Event: Login Complete Connected to Lobby/Party
	virtual void OnPlayerStateEvent(ASteamBeaconPlayerState* InPlayerState);
	//Event: Host Connection Failed
	virtual void OnSteamBeaconHostConnectionFailureEvent();
	


	//Game Sessions
	//Delegate Handle for Create Game Session
	FDelegateHandle OnCreatePresenceGameSessionCompleteDelegateHandle;
	//Callback which is intended to be called upon Game session creation 
	void OnCreatePresenceGameSessionCompleted(FName SessionName, bool bWasSuccessful);

	//Delegate Handle for Join Game Session
	FDelegateHandle OnJoinGameSessionCompletedDelegateHandle;
	//Callback which is intended to be called upon Game session joined 
	void OnJoinGameSessionCompleted(EOnJoinSessionCompleteResult::Type Result);
	
	//Connect the player to Server from Session Data
	virtual void TravelToGameSession();

public:
	//Direct Connection to steam socket id   "steam.76561198146823000:7777"  "steam.accountid:port"
	UFUNCTION(BlueprintCallable, Category = "Game")
	virtual void DirectTravelToGameSession(FString ServerURL);

private:
	
	//Delayed version of JoinSessionFromSteamBeacon()  ... Delayed Connect, turned off 
	virtual void ResumeJoinSessionFromSteamBeacon();
	FSteamBeaconSessionResult SavedSearchResultFromSteamBeacon;


	// Delegate registered with Steam to trigger when a user receives lobby invite, notify also in SteamClient
	STEAM_CALLBACK_MANUAL(USteamBeaconGameInstance, OnLobbyInviteReceived, LobbyInvite_t, OnLobbyInviteReceivedCallback);

	// Delegate registered with Steam to trigger when a user enters / leaves lobby
	STEAM_CALLBACK_MANUAL(USteamBeaconGameInstance, OnLobbyChatUpdate, LobbyChatUpdate_t, OnLobbyChatUpdateCallback);

	

	//Store Last Requested Party Info
	FPartySaveInfo PartySavedInfo;
	
	//Delegate fired when the party state changes
	FOnPartyJoiningGame PartyJoiningGameEvent;

	friend ASteamBeaconGameSession;
};

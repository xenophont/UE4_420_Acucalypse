// Copyright 2013-2017 W3 Studios, LLC. All Rights Reserved.

#pragma once

#include "LobbyBeaconHost.h"
#include "SteamBeaconTypes.h"
#include "SteamBeaconState.h"
#include "SteamBeaconClient.h"
#include "SteamBeaconHost.generated.h"

//The Beacon Host, Authority of the Party similiar to AGameModeBase but for Beacons
UCLASS()
class STEAMPARTY_API ASteamBeaconHost : public ALobbyBeaconHost
{
	GENERATED_UCLASS_BODY()

public:

	//Send Connection Info to Party members and Connect to GameServer
	virtual void ConnectPartyToGameServer(const FSteamBeaconSessionResult& InSearchResult, bool bIsPartyHostCreatingServer = true);
	virtual void ConnectPartyToGameServer(FString GameServerURL);
	
	//Kick Player From the Lobby
	void KickPlayerFromLobby(ALobbyBeaconClient* ClientActor, const FText& KickReason);

	//Handle a detected disconnect of an existing player on the server
	///@param InUniqueId unique id of the player
	virtual void HandlePlayerLogout(const FUniqueNetIdRepl& InUniqueId) override;
	virtual bool ProcessKickPlayer(ALobbyBeaconClient* Instigator, const FUniqueNetIdRepl& PlayerToKick, const FText& Reason) override;

	virtual void NotifyClientDisconnected(AOnlineBeaconClient* LeavingClientActor) override;
	virtual void OnClientConnected(AOnlineBeaconClient* NewClientActor, UNetConnection* ClientConnection) override;
	

	//Get Lobby State
	UFUNCTION(BlueprintCallable, Category = "SteamBeacon")
		virtual ASteamBeaconState* GetPartyState();

	//Disband Party
	UFUNCTION(BlueprintCallable, Category = "SteamBeacon")
		virtual void DisbandParty();

	//In the Process of disbanding the party, ignore on player left events as we don't care anymore
	UPROPERTY(BlueprintReadWrite, Category = "SteamBeacon")
		bool bIsDisbandingParty = false;

	//In the Process of joining a game, used to ingore disconnection notifications
	UPROPERTY(BlueprintReadWrite, Category = "SteamBeacon")
		bool bIsJoiningGame = false;
	
	//Dump Debug Info to the Log file
	virtual void DumpPartyState() const;

	//Notification that all Actor Channels are connected
	void SetHandshakeCompleteForBeacon(ASteamBeaconClient* ClientActor);
	//Run NetCleanup on any associated SteamBeaconClients to Connection
	void ProcessBeaconNetCleanup(UNetConnection* Connection);
	
	//Debugging Info for Network Replication
	void PrintNetworkDebugInfo();

	//Update Party Session Info
	void UpdateSessionPartyOnline();

protected:

	//Used to Fixup Client NetDriverName
	virtual void PostLogin(ALobbyBeaconClient* ClientActor) override;

	//Process the login for a given connection
	///@param Client client beacon making the request
	///@param InSessionId id of the session that is being checked
	///@param InUniqueId id of the player logging in
	///@param UrlString URL containing player options (name, etc)
	void ProcessPlayerLogin(ASteamBeaconClient* ClientActor, const FString& InSessionId, const FUniqueNetIdRepl& InUniqueId, const FString& UrlString);

	//Handle a player logging in via the host beacon
	///@param ClientActor client that is making the request
	///@param InUniqueId unique id of the player
	///@param Options game options passed in by the client at login
	///@return new player state object for the logged in player, null if there was any failure
	virtual ASteamBeaconPlayerState* HandlePlayerLoginOverride(ALobbyBeaconClient* ClientActor, const FUniqueNetIdRepl& InUniqueId, const FString& Options);
	
	//Give access to my protected members to my friends
	friend ASteamBeaconClient;
		
};

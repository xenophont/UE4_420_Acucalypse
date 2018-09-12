// Copyright 2013-2017 W3 Studios, LLC. All Rights Reserved.

#pragma once

#include "LobbyBeaconClient.h"
#include "SteamBeaconTypes.h"
//#include "GameFramework/PlayerController.h"
#include "Engine/NetConnection.h"
#include "SteamBeaconClient.generated.h"

//Delegate called when the PlayerState has been received for this client is complete
DECLARE_DELEGATE_OneParam(FOnPlayerState, ASteamBeaconPlayerState* /*InPlayerState*/);

//Steam Party Beacon Client, the networking interface for connected players handles RPCs
UCLASS()
class STEAMPARTY_API ASteamBeaconClient : public ALobbyBeaconClient
{
	GENERATED_UCLASS_BODY()
	
public:
	
	//~ Begin AActor Interface
	//virtual void PostInitializeComponents() override;
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;
	//virtual void Tick(float DeltaTime) override;
	//~ End AActor Interface


	//~ Begin FNetworkNotify Interface
	//virtual EAcceptConnection::Type NotifyAcceptingConnection() override;
	//virtual void NotifyAcceptedConnection(UNetConnection* Connection) override;
	virtual bool NotifyAcceptingChannel(UChannel* Channel) override;
	//virtual void NotifyControlMessage(UNetConnection* Connection, uint8 MessageType, FInBunch& Bunch) override;
	//~ End FNetworkNotify Interface
	
	//Spawn a DummyPC to handle steam voice chat
	virtual void SetupDummyPCForVoiceChat(UNetConnection* Connection);

	//Called when we received our relicated Lobby & Player State variables
	virtual void OnReplicatedActorsSet();
	
	//Have we sent a HandshakeCompleted Signal back to Server
	UPROPERTY()
		bool bHasHandShakeCompleted = false;

	//Tell Server that handshakingis complete
	UFUNCTION(server, reliable, withValidation)
		virtual void ServerSetHandshakeComplete();

	//Beacon lobby state
	//UPROPERTY(ReplicatedUsing = OnRep_LobbyState)
		//ASteamBeaconState* BeaconLobbyState = nullptr;

	//Beacon lobby state
	UPROPERTY(ReplicatedUsing = OnRep_LobbyState)
		ASteamBeaconState* BeaconLobbyState;
	//Player state associated with this beacon
	UPROPERTY(ReplicatedUsing = OnRep_PlayerState)
		ASteamBeaconPlayerState* BeaconPlayerState;
		
	//On replicated function for LobbyState
	UFUNCTION()
		virtual void OnRep_LobbyState();
	//On replicated function for PlayerState
	UFUNCTION()
		virtual void OnRep_PlayerState();
	
	
	void SetSessionId(const FString& NewId) { DestSessionId = NewId; }
	
	virtual void ServerSetPartyOwner_Implementation(const FUniqueNetIdRepl& InUniqueId, const FUniqueNetIdRepl& InPartyOwnerId) override;

	//Tell Clients to connect to game session
	UFUNCTION(client, reliable)
		virtual void ClientJoinGameSession(FSteamBeaconSessionResult InSearchResult, bool bIsPartyHostCreatingServer);

	//Tell Clients to connect to game session
	UFUNCTION(client, reliable)
		virtual void ClientJoinGameServer(const FString& GameServerURL);

	//Get Lobby State
	UFUNCTION(BlueprintCallable, Category = "SteamBeacon")
		virtual ASteamBeaconState* GetPartyState();

	//Get Player State
	UFUNCTION(BlueprintCallable, Category = "SteamBeacon")
		ASteamBeaconPlayerState* GetPlayerState();
	
	//Leave Party
	UFUNCTION(BlueprintCallable, Category = "SteamBeacon")
		virtual void LeaveParty();

	//On Replicated Function to disband party
	UFUNCTION(Client, Reliable)
		virtual void OnPartyDisbanded();

	//Kick Player from Party
	UFUNCTION(BlueprintCallable, Category = "SteamBeacon")
		virtual void KickFromParty(const FUniqueNetIdRepl& PlayerToKick, const FText Reason) { KickPlayer(PlayerToKick, Reason); }

	//Wrapper to transmit back to player they were kicked
	virtual void ClientWasKickedFromLobby(const FText& KickReason);

	//Chat Privately with Party member
	UFUNCTION(BlueprintCallable, Category = "SteamBeacon")
		virtual void ChatToPartyMember(ASteamBeaconPlayerState* InPlayer) {};
	
	virtual void ServerDisconnectFromLobby_Implementation() override;
	virtual void ServerKickPlayer_Implementation(const FUniqueNetIdRepl& PlayerToKick, const FText& Reason) override;
	virtual void ClientWasKicked_Implementation(const FText& KickReason) override;
	
	virtual void ClientPlayerJoined_Implementation(const FText& NewPlayerName, const FUniqueNetIdRepl& InUniqueId) override;
	virtual void ClientPlayerLeft_Implementation(const FUniqueNetIdRepl& InUniqueId) override;

	//Handles cleaning up the associated Actor when killing the connection
	///@param Connection the connection associated with this actor
	virtual void OnNetCleanup(class UNetConnection* Connection) override;

	virtual void DestroyBeacon() override;

	/// @return delegate fired when PlayerState Has been received
	FOnPlayerState& OnPlayerStateEvent() { return OnPlayerStateDelegate; }

	//Override wrappers
	virtual void ClientPlayerJoinedOverride(const FText& NewPlayerName, const FUniqueNetIdRepl& InUniqueId) { ClientPlayerJoined(NewPlayerName, InUniqueId); }
	virtual void SetLobbyStateOverride(ALobbyBeaconState* InLobbyState);	// { SetLobbyState(InLobbyState); BeaconLobbyState = Cast<ASteamBeaconState>(InLobbyState); }
	virtual void ClientLoginCompleteOverride(const FUniqueNetIdRepl& InUniqueId, bool bWasSuccessful) { ClientLoginComplete(InUniqueId, bWasSuccessful); bLoggedIn = bWasSuccessful; }

protected:

	// Delegate broadcast when login is complete (clientside)
	FOnPlayerState OnPlayerStateDelegate;

	//Reroute Player Login away from base class implementation which is locked, PR Request submitted to resolve this workaround in a future build
	virtual void ServerLoginPlayer_Implementation(const FString& InSessionId, const FUniqueNetIdRepl& InUniqueId, const FString& UrlString) override;
};

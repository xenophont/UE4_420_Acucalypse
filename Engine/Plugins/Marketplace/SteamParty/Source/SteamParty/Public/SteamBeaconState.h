// Copyright 2013-2017 W3 Studios, LLC. All Rights Reserved.

#pragma once

#include "LobbyBeaconState.h"
#include "SteamBeaconPlayerState.h"
#include "SteamBeaconTypes.h"
#include "SteamBeaconState.generated.h"


//Delegate fired when any player state has changed in the party
///@param UniqueId id of the player that changed
DECLARE_EVENT_OneParam(ASteamBeaconState, FOnPartyPlayerStateChanged, ASteamBeaconPlayerState* /** InPlayerState */);

//Delegate fired when party state has changed and clients need to refresh their data
DECLARE_EVENT(ASteamBeaconState, FOnBeaconPartyStateChanged);

//Party State, much like a GameState, Replicates All Party info to partymembers
UCLASS()
class STEAMPARTY_API ASteamBeaconState : public ALobbyBeaconState
{
	GENERATED_UCLASS_BODY()
	
public:

	//~ Begin AActor Interface
	//virtual void PostInitializeComponents() override;
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;
	//~ End AActor Interface

	//Send Connection Info to Party members and Connect to GameServer
	virtual void ConnectPartyToGameServer(const FSteamBeaconSessionResult& InSearchResult, bool bIsPartyHostCreatingServer);
	virtual void ConnectPartyToGameServer(FString GameServerURL);
	
	//Get Member of Party by Id
	UFUNCTION(BlueprintCallable, Category = "SteamBeacon")
	ASteamBeaconPlayerState* GetPartyPlayer(const FUniqueNetIdRepl& UniqueId);
	
	//Get Array of all Party members
	UFUNCTION(BlueprintCallable, Category = "SteamBeacon")
	void GetAllPartyMembers(UPARAM(ref) TArray<ASteamBeaconPlayerState*>& InPlayerArray);

	//Current Party Leader id
	UPROPERTY(ReplicatedUsing = OnRep_PartyOwnerUniqueId)
		FUniqueNetIdRepl PartyOwnerUniqueId;
	//On Replicated Function for Party Leader Change
	UFUNCTION()
		virtual void OnRep_PartyOwnerUniqueId() {};
	
	
	//On Replicated Function for some LobbyState Info changed/updated
	UFUNCTION(NetMulticast, Reliable)
		virtual void OnLobbyStateInfoUpdated();

	UFUNCTION()
	virtual void OnLobbyStateInfoUpdated_NetRefresh();
	
	
	//Process Chat Messages
	virtual void ProcessChatMessage(ASteamBeaconPlayerState* InPlayerState, FPartyMessage InChatMessage);
	
	//Dump Debug Info to the Log file
	virtual void DumpPartyState() const;

	//@return delegate fired when the state of any player has changed in some way
	FORCEINLINE FOnPartyPlayerStateChanged& OnPartyPlayerStateChangedEvent() { return PartyPlayerStateChangedEvent; }

	//Access PartyStateChangedEvent Delegate
	FORCEINLINE FOnBeaconPartyStateChanged& OnPartyStateChangedEvent() { return PartyStateChangedEvent; }

protected:
	virtual void OnPreLobbyStartedTickInternal(double DeltaTime) override;
	virtual void OnPostLobbyStartedTickInternal(double DeltaTime) override;

	//Create a new player for this lobby beacon state
	virtual ALobbyBeaconPlayerState* CreateNewPlayer(const FText& PlayerName, const FUniqueNetIdRepl& UniqueId) override;

private:

	//Delegate fired when player state changes 
	FOnPartyPlayerStateChanged PartyPlayerStateChangedEvent;
	//Delegate fired when the party state changes
	FOnBeaconPartyStateChanged PartyStateChangedEvent;

};

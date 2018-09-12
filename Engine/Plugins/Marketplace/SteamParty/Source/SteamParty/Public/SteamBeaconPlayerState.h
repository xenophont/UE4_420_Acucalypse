// Copyright 2013-2017 W3 Studios, LLC. All Rights Reserved.

#pragma once

#include "LobbyBeaconPlayerState.h"
#include "SteamBeaconTypes.h"
#include "GameFramework/PlayerState.h"
#include "SteamBeaconPlayerState.generated.h"

//Delegate fired when a party chat message is received
///@param Party Message struct
DECLARE_EVENT_OneParam(ASteamBeaconPlayerState, FOnChatMessageReceived, FPartyMessage /** InPartyMessage */);

//Create Delegate for TalkingStates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerTalkingStateChanged, bool, IsTalking);

//Beacon Player State, holds data specific to each player, very similiar to the idea of APlayerState but for beacons
UCLASS(BlueprintType, Blueprintable, meta = (ShortTooltip = "A SteamBeacon PlayerState is the player representation for Online Beacons containing player data."))
class STEAMPARTY_API ASteamBeaconPlayerState : public ALobbyBeaconPlayerState
{
	GENERATED_UCLASS_BODY()
public:

	//~ Begin AActor Interface
	//virtual void PostInitializeComponents() override;
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;
	//~ End AActor Interface

	//Get Player UniqueId
	UFUNCTION(BlueprintPure, Category = "SteamBeacon")
		FUniqueNetIdRepl GetPlayerUniqueId() { return UniqueId; }
	
	//Get Player name
	UFUNCTION(BlueprintPure, Category = "SteamBeacon")
		FText GetDisplayName() { return DisplayName; }

	//Does BeaconPlayer State match PlayerController State
	UFUNCTION(BlueprintPure, Category = "SteamBeacon")
		bool IsPlayer(APlayerState* InPlayerState);
	
	//Is PlayerState passed in Party Leader
	UFUNCTION(BlueprintPure, Category = "SteamBeacon")
		bool IsPlayerPartyLeader(APlayerState* InPlayerState);

	//Is this Player Party Leader
	UFUNCTION(BlueprintPure, Category = "SteamBeacon")
		bool IsPartyLeader();
	
		
	//Game Specific Variables Added here, Example
	//Player Profile Level
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "SteamBeacon")	//Using = OnRep_PlayerProfileLevel)
		int32 PlayerProfileLevel = 0;

	//Set Player Profile on server, will be replicated back down
	UFUNCTION(Reliable, Server, WithValidation)
		virtual void ServerSetPlayerProfileLevel(int32 InPlayerProfileLevel);
	

	//Chat Handling
public:

	//Send Chat Message, Called from UMG Text Box or any other BP/Widget
	UFUNCTION(BlueprintCallable, Category = "Chat")
		virtual void SendPartyMessage(FPartyMessage ChatMessage);
		
	//Send Chat Message to server for processing and distribution
	UFUNCTION(Reliable, Server, WithValidation)
		virtual void ServerSendPartyMessage(FPartyMessage ChatMessage);

	//Chat Message Received from PartyState
	UFUNCTION(Reliable, NetMulticast, BlueprintCallable, Category = "Chat")
		virtual void OnPartyMessageReceived(FPartyMessage InPartyMessage);


	//Access Party Cheat Message Received Event
	FORCEINLINE FOnChatMessageReceived& OnChatMessageReceivedEvent() { return ChatMessageReceivedEvent; }

private:

	//Delegate fired when player receives chat message
	FOnChatMessageReceived ChatMessageReceivedEvent;



	//VOICE Chat Handling
public:

	// Delegate called when Player Talking State Changes, Start or Stop
	UPROPERTY(BlueprintAssignable, Category = "Voice Chat")
		FPlayerTalkingStateChanged OnPlayerTalkingStateChanged;
};

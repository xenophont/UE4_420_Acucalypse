// Copyright 2013-2017 W3 Studios, LLC. All Rights Reserved.

#pragma once

#include "GameFramework/PlayerController.h"
#include "SteamBeaconState.h"
#include "SteamBeaconPlayerState.h"
#include "SteamBeaconClient.h"
#include "UObject/CoreOnline.h"
#include "Interfaces/VoiceInterface.h"
#include "SteamBeaconPlayerController.generated.h"

//Steam Beacon Player Controller class to handle Events from the Party System
UCLASS()
class STEAMPARTY_API ASteamBeaconPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	//Register to SteamBeacon Events
	virtual void RegisterPartyBeacon(ASteamBeaconClient* InPartyBeacon);
	
	//Our Current Party State we are part of 
	UPROPERTY()
		ASteamBeaconState* PartyBeaconState = nullptr;

	//Event: Player Joined the Party
	void OnPlayerJoinedParty(ALobbyBeaconPlayerState* InPlayerBeaconStateJoined);
	//Event: Player Left the Party
	void OnPlayerLeftParty(ALobbyBeaconPlayerState* InPlayerBeaconStateLeft);
	//Event: Player State Changed
	void OnPlayerStateChanged(ASteamBeaconPlayerState* InPlayerBeaconStateChanged);
	//Event: Party Leader Changed
	void OnPartyOwnerChanged(const FUniqueNetIdRepl& InUniqueId);
	//Event: Party State Changed
	void OnPartyStateChanged();
	//Event: Party Joining Game
	void OnPartyJoiningGame();
	//Event: Party Chat Message Received
	void OnChatMessageReceived(FPartyMessage InPartyMessage);
	//Event: Failed to Connect to Party Beacon
	virtual void OnPartyHostConnectionFailure();
	//Event: Started to Re-Connect to Party Beacon, after match, map travel, etc....
	virtual void OnPartyHostReconnecting();
	//Event: Invite to a Party Received
	virtual void OnPartyInviteReceived(const FString& FriendName);

	//Send Blueprint Event Notification, Player Joined Party, used Mainly for UMG Communication
	UFUNCTION(BlueprintImplementableEvent)
		void BPEvent_PlayerJoinedParty(ASteamBeaconPlayerState* InPlayerStateJoined);

	//Send Blueprint Event Notification, Player Left Party, used Mainly for UMG Communication
	UFUNCTION(BlueprintImplementableEvent)
		void BPEvent_PlayerLeftParty(ASteamBeaconPlayerState* InPlayerStateLeft);

	//Send Blueprint Event Notification, Player State Changed, used Mainly for UMG Communication
	UFUNCTION(BlueprintImplementableEvent)
		void BPEvent_PlayerStateChanged(ASteamBeaconPlayerState* InPlayerBeaconStateChanged);

	//Send Blueprint Event Notification, Party Leader Changed, used Mainly for UMG Communication
	UFUNCTION(BlueprintImplementableEvent)
		void BPEvent_PartyOwnerChanged(const FUniqueNetIdRepl& InUniqueId);

	//Send Blueprint Event Notification, Party State Changed, used Mainly for UMG Communication
	UFUNCTION(BlueprintImplementableEvent)
		void BPEvent_PartyStateChanged();

	//Send Blueprint Event Notification, Party State Changed, used Mainly for UMG Communication
	UFUNCTION(BlueprintImplementableEvent)
		void BPEvent_PartyJoiningGame();

	//Send Blueprint Event Notification, Chat Message Received, used Mainly for UMG Communication
	UFUNCTION(BlueprintImplementableEvent)
		void BPEvent_ChatMessageReceived(FPartyMessage InPartyMessage);

	//Send Blueprint Event Notification, Party Host Connection Failure, used Mainly for UMG Communication
	UFUNCTION(BlueprintImplementableEvent)
		void BPEvent_PartyHostConnectionFailure();

	//Send Blueprint Event Notification, Reconnecting to Party Host, used Mainly for UMG Communication
	UFUNCTION(BlueprintImplementableEvent)
		void BPEvent_PartyHostReconnecting();

	//Send Blueprint Event Notification, Invite Received from Steam Friend, used Mainly for UMG Communication
	UFUNCTION(BlueprintImplementableEvent)
		void BPEvent_PartyInviteReceived(const FString& FriendName);
		
	//Unhook from all events
	virtual void UnBindAllPartyDelegates();

	//Simple helper function to check to see if this PlayerController matches the supplied NetId,  NetIds are not very usable in blueprints
	UFUNCTION(BlueprintCallable, Category = "Steam Party")
		virtual bool IsPlayerControllerUniqueIdMatch(const FUniqueNetIdRepl& PlayerId);

	//~~~~~~~~~~ VOICE Implementation
	
	//Override to update the mute lists specific to online beacons since there is no PC in the game world for the party members
	virtual void ServerMutePlayer_Implementation(FUniqueNetIdRepl PlayerId) override;

	//Override to update the mute lists specific to online beacons since there is no PC in the game world for the party members
	virtual void ServerUnmutePlayer_Implementation(FUniqueNetIdRepl PlayerId) override;
	
	//Player Talk State Changed
	virtual void OnPlayerTalkingStateChanged(TSharedRef<const FUniqueNetId> TalkerId, bool bIsTalking);
	
	// Delegate for Player Talking State Changes, Talking/Not Talking
	FOnPlayerTalkingStateChangedDelegate OnPlayerTalkingStateChangedDelegate;
	FDelegateHandle OnPlayerTalkingStateChangedDelegateHandle;

	//Is Player Muted
	UFUNCTION(BlueprintCallable, Category = "Voice Chat")
		virtual bool IsPlayerVoiceMuted(const FUniqueNetIdRepl& PlayerId);
	//Mute this Player 
	UFUNCTION(BlueprintCallable, Category = "Voice Chat")
		virtual void MutePlayerVoice(const FUniqueNetIdRepl& PlayerId);
	//UnMute this Player
	UFUNCTION(BlueprintCallable, Category = "Voice Chat")
		virtual void UnMutePlayerVoice(const FUniqueNetIdRepl& PlayerId);
	
	//Enable voice chat transmission
	UFUNCTION(BlueprintCallable, Category = "Voice Chat")
		void StartPlayerTalking();

	// Disable voice chat transmission
	UFUNCTION(BlueprintCallable, Category = "Voice Chat")
		void StopPlayerTalking();

	//Is Push To Talk enabled, pulled from config bRequiresPushToTalk
	UPROPERTY(BlueprintReadWrite, Category = "Voice Chat")
		bool IsPushToTalk = false;
};

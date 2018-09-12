// Copyright 2013-2017 W3 Studios, LLC. All Rights Reserved.

#include "SteamBeaconPlayerController.h"
#include "SteamParty.h"
#include "SteamBeaconGameInstance.h"
#include "GameFramework/PlayerMuteList.h"
#include "Net/OnlineEngineInterface.h"
#include "Misc/ConfigCacheIni.h"
#include "Async/Async.h"

//Register to SteamBeacon Events
void ASteamBeaconPlayerController::RegisterPartyBeacon(ASteamBeaconClient* InPartyBeacon)
{
	if (!InPartyBeacon) return;

	//Register our Listener functions for Lobby Events
	ASteamBeaconState* InPartyState = InPartyBeacon->GetPartyState();
	if (!InPartyState) return;
	PartyBeaconState = InPartyState;

	InPartyState->OnPlayerLobbyStateAdded().AddUObject(this, &ThisClass::OnPlayerJoinedParty);
	InPartyState->OnPlayerLobbyStateRemoved().AddUObject(this, &ThisClass::OnPlayerLeftParty);
	InPartyState->OnPartyPlayerStateChangedEvent().AddUObject(this, &ThisClass::OnPlayerStateChanged);
	InPartyState->OnPartyStateChangedEvent().AddUObject(this, &ThisClass::OnPartyStateChanged);
	
	
	//Need to move to Party, since there can be X Numbere of playerstates, centralize to lobby all delegates, smarter
	//Register our Listener functions for BeaconPlayerState Events
	ASteamBeaconPlayerState* InPartyPlayerState = Cast<ASteamBeaconPlayerState>(InPartyBeacon->PlayerState);
	if (InPartyPlayerState)
	{
		InPartyPlayerState->OnPartyOwnerChanged().AddUObject(this, &ThisClass::OnPartyOwnerChanged);
		InPartyPlayerState->OnChatMessageReceivedEvent().AddUObject(this, &ThisClass::OnChatMessageReceived);
	}

	USteamBeaconGameInstance* pSBGI = Cast<USteamBeaconGameInstance>(GetWorld()->GetGameInstance());
	if (pSBGI)
	{
		pSBGI->OnPartyJoiningGameEvent().AddUObject(this, &ThisClass::OnPartyJoiningGame);
	}

	//Lets Update our Session Data to keep in sync with Server, Steam, and make OSS subsystem happy
	TArray<ASteamBeaconPlayerState*> PartyMembers;
	InPartyState->GetAllPartyMembers(PartyMembers);
	if (PartyMembers.Num())
	{
		for (int32 i =0; i< PartyMembers.Num(); i++)
		{
			if (PartyMembers[i])
			{
				//Register With Online Session
				UOnlineEngineInterface::Get()->RegisterPlayer(GetWorld(), PartySessionName, *PartyMembers[i]->UniqueId, true);
			}
		}
	}

	//If Voice Chat is enable, hook up the delegates
	if (pSBGI->VoiceChatEnabled)
	{
		//Setup Voice Chat Delegate
		IOnlineVoicePtr VoiceInt = Online::GetVoiceInterface();
		if (VoiceInt.IsValid() && !OnPlayerTalkingStateChangedDelegateHandle.IsValid())
		{
			OnPlayerTalkingStateChangedDelegate = FOnPlayerTalkingStateChangedDelegate::CreateUObject(this, &ASteamBeaconPlayerController::OnPlayerTalkingStateChanged);
			OnPlayerTalkingStateChangedDelegateHandle = VoiceInt->AddOnPlayerTalkingStateChangedDelegate_Handle(OnPlayerTalkingStateChangedDelegate);

			//Check and Save Push To Talk
			GConfig->GetBool(TEXT("/Script/Engine.GameSession"), TEXT("bRequiresPushToTalk"), IsPushToTalk, GGameIni);
		}
	}
}

//Player Started or Stopped Talking, Notify
void ASteamBeaconPlayerController::OnPlayerTalkingStateChanged(TSharedRef<const FUniqueNetId> TalkerId, bool bIsTalking)
{
	//Send Blueprint notification
	if (PartyBeaconState)
	{
		TArray<ASteamBeaconPlayerState*> PartyMembers;
		PartyBeaconState->GetAllPartyMembers(PartyMembers);
		if (PartyMembers.Num())
		{
			for (int32 i = 0; i < PartyMembers.Num(); i++)
			{
				if (PartyMembers[i] && (PartyMembers[i]->UniqueId == TalkerId))
				{
					//Call the BP Assigned Delegates for any listeners like in UMG
					PartyMembers[i]->OnPlayerTalkingStateChanged.Broadcast(bIsTalking);
					break;
				}
			}
		}
	}
}


//Event: Player Joined the Party
void ASteamBeaconPlayerController::OnPlayerJoinedParty(ALobbyBeaconPlayerState* InPlayerBeaconStateJoined)
{
	ASteamBeaconPlayerState* pSteamBeaconPlayerState = Cast<ASteamBeaconPlayerState>(InPlayerBeaconStateJoined);
	if (!pSteamBeaconPlayerState) return;

	//Send Blueprints Notification
	BPEvent_PlayerJoinedParty(pSteamBeaconPlayerState);
	
	//Register With Online Session
	UOnlineEngineInterface::Get()->RegisterPlayer(GetWorld(), PartySessionName, *pSteamBeaconPlayerState->UniqueId, true);
}

//Event: Player Left the Party
void ASteamBeaconPlayerController::OnPlayerLeftParty(ALobbyBeaconPlayerState* InPlayerBeaconStateLeft)
{
	ASteamBeaconPlayerState* pSteamBeaconPlayerState = Cast<ASteamBeaconPlayerState>(InPlayerBeaconStateLeft);
	if (!pSteamBeaconPlayerState) return;

	//Send Blueprints Notification
	BPEvent_PlayerLeftParty(pSteamBeaconPlayerState);

	//UnRegister With Online Session
	UOnlineEngineInterface::Get()->UnregisterPlayer(GetWorld(), PartySessionName, *pSteamBeaconPlayerState->UniqueId);
}

//Event: Player State Changed
void ASteamBeaconPlayerController::OnPlayerStateChanged(ASteamBeaconPlayerState* InPlayerBeaconStateChanged)
{
	//Send Blueprints Notification
	BPEvent_PlayerStateChanged(InPlayerBeaconStateChanged);
}

//Event: Party Leader Changed
void ASteamBeaconPlayerController::OnPartyOwnerChanged(const FUniqueNetIdRepl& InUniqueId)
{
	//Send Blueprints Notification
	BPEvent_PartyOwnerChanged(InUniqueId);
}

//Event: Party State Changed
void ASteamBeaconPlayerController::OnPartyStateChanged()
{
	//Send Blueprints Notification
	BPEvent_PartyStateChanged();
}

//Event: Party Joining Game
void ASteamBeaconPlayerController::OnPartyJoiningGame()
{
	//Send Blueprints Notification
	BPEvent_PartyJoiningGame();

	//Here We need to unbind all our delegates during map change
	UnBindAllPartyDelegates();
}

//Event: Party Chat Message Received
void ASteamBeaconPlayerController::OnChatMessageReceived(FPartyMessage InPartyMessage)
{
	//Send Blueprints Notification
	BPEvent_ChatMessageReceived(InPartyMessage);
}

//Event: Failed to Connect to Party Host Beacon
void ASteamBeaconPlayerController::OnPartyHostConnectionFailure()
{
	//Send Blueprints Notification
	BPEvent_PartyHostConnectionFailure();
}

//Event: Started Reconnecting to Party Host Beacon
void ASteamBeaconPlayerController::OnPartyHostReconnecting()
{
	//Send Blueprints Notification
	BPEvent_PartyHostReconnecting();
}

//Event: Invite to a Party Received
void ASteamBeaconPlayerController::OnPartyInviteReceived(const FString& FriendName)
{
	//If you are receiving NotRunning on GameThread Errors, run it like this.
	/*AsyncTask(ENamedThreads::GameThread, [this, FriendName]() {
		UE_LOG(LogSteamBeacon, Log, TEXT("Running inside AsyncTask() TRUE"));
		BPEvent_PartyInviteReceived(FriendName);
		UE_LOG(LogSteamBeacon, Log, TEXT("Exiting AsyncTask()"));
	});*/
	
	//Send Blueprints Notification
	BPEvent_PartyInviteReceived(FriendName);
}

//Remove all delegates hooked into the party system
void ASteamBeaconPlayerController::UnBindAllPartyDelegates()
{
	USteamBeaconGameInstance* pSBGI = Cast<USteamBeaconGameInstance>(GetWorld()->GetGameInstance());
	if (!pSBGI) return;
	
	if (pSBGI->OnPartyJoiningGameEvent().IsBoundToObject(this))
	{
		pSBGI->OnPartyJoiningGameEvent().RemoveAll(this);
	}
	
	ASteamBeaconClient* InPartyBeacon = pSBGI->BeaconClient;
	if (!InPartyBeacon) return;

	//UnRegister our Listener functions for Lobby Events
	ASteamBeaconState* InPartyState = InPartyBeacon->GetPartyState();
	if (InPartyState)
	{
		InPartyState->OnPlayerLobbyStateAdded().RemoveAll(this);
		InPartyState->OnPlayerLobbyStateRemoved().RemoveAll(this);
		InPartyState->OnPartyPlayerStateChangedEvent().RemoveAll(this);

		InPartyState->OnPartyStateChangedEvent().RemoveAll(this);
	}

	//UnRegister our Listener functions for BeaconPlayerState Events
	ASteamBeaconPlayerState* InPartyPlayerState = Cast<ASteamBeaconPlayerState>(InPartyBeacon->PlayerState);
	if (InPartyPlayerState)
	{
		InPartyPlayerState->OnPartyOwnerChanged().RemoveAll(this);
		InPartyPlayerState->OnChatMessageReceivedEvent().RemoveAll(this);
	}

	//Remove Voice Chat Delegate
	IOnlineVoicePtr VoiceInt = Online::GetVoiceInterface();
	if (VoiceInt.IsValid() && OnPlayerTalkingStateChangedDelegateHandle.IsValid())
	{
		VoiceInt->ClearOnPlayerTalkingStateChangedDelegate_Handle(OnPlayerTalkingStateChangedDelegateHandle);
	}
	OnPlayerTalkingStateChangedDelegateHandle.Reset();

	PartyBeaconState = nullptr;
}

//Does passed in NetId match our NetId
bool ASteamBeaconPlayerController::IsPlayerControllerUniqueIdMatch(const FUniqueNetIdRepl& PlayerId)
{
	if (PlayerState && PlayerState->UniqueId == PlayerId)
	{
		return true;
	}
	return false;
}

//Mute Party Member ::TODO:: Deprecated?? Using the "MutePlayerVoice", Look at for removal or refactor
void ASteamBeaconPlayerController::ServerMutePlayer_Implementation(FUniqueNetIdRepl PlayerId)
{
	USteamBeaconGameInstance* pSBGI = Cast<USteamBeaconGameInstance>(GetWorld()->GetGameInstance());
	if (!pSBGI || !pSBGI->VoiceChatEnabled) return;
}

//Unmute Party Member ::TODO:: Deprecated?? Using the "UnMutePlayerVoice", Look at for removal or refactor
void ASteamBeaconPlayerController::ServerUnmutePlayer_Implementation(FUniqueNetIdRepl PlayerId)
{
	USteamBeaconGameInstance* pSBGI = Cast<USteamBeaconGameInstance>(GetWorld()->GetGameInstance());
	if (!pSBGI || !pSBGI->VoiceChatEnabled) return;
	
	ClientUnmutePlayer(PlayerId);
}

//Check if Muted on Voice Interface
bool ASteamBeaconPlayerController::IsPlayerVoiceMuted(const FUniqueNetIdRepl& PlayerId)
{
	IOnlineVoicePtr VoiceInt = Online::GetVoiceInterface();
	if (VoiceInt.IsValid())
	{
		return VoiceInt->IsMuted(0, *PlayerId);
	}
	return false;
}

//Mute Player on Voice Interface
void ASteamBeaconPlayerController::MutePlayerVoice(const FUniqueNetIdRepl& PlayerId)
{
	IOnlineVoicePtr VoiceInt = Online::GetVoiceInterface();
	if (VoiceInt.IsValid())
	{
		VoiceInt->MuteRemoteTalker(0, *PlayerId, false);
	}
}

//UnMuted Player on Voice Interface
void ASteamBeaconPlayerController::UnMutePlayerVoice(const FUniqueNetIdRepl& PlayerId)
{
	IOnlineVoicePtr VoiceInt = Online::GetVoiceInterface();
	if (VoiceInt.IsValid())
	{
		VoiceInt->UnmuteRemoteTalker(0, *PlayerId, false);
	}
}

//Start Voice Chatting, Blueprint Wrappers
void ASteamBeaconPlayerController::StartPlayerTalking()
{
	if (IsPushToTalk)
	{
		StartTalking();
	}
}

//Stop Voice Chatting, Blueprint Wrappers
void ASteamBeaconPlayerController::StopPlayerTalking()
{
	if (IsPushToTalk)
	{
		StopTalking();
	}
}
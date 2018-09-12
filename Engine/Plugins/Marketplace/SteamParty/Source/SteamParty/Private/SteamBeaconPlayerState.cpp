// Copyright 2013-2017 W3 Studios, LLC. All Rights Reserved.

#include "SteamBeaconPlayerState.h"
#include "SteamParty.h"
#include "Net/UnrealNetwork.h"
#include "SteamBeaconClient.h"
#include "SteamBeaconState.h"
#include "GameFramework/PlayerController.h"


//Beacon Player State, holds data specific to each player, very similiar to the idea of APlayerState but for beacons
ASteamBeaconPlayerState::ASteamBeaconPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//Force this Actor Always Revelant to prevent closing down its actor net channel
	bReplicates = true;
	bAlwaysRelevant = true;
	bOnlyRelevantToOwner = false;
	//NetPriority = 2.0;
}

//Replicated Variables registering
void ASteamBeaconPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASteamBeaconPlayerState, PlayerProfileLevel);
}

//Net Relevancy check
bool ASteamBeaconPlayerState::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	//If Voice Chat is enabled PCs are now the owners instead of the beacons for internal handling
	UClass* PlayerControllerClass = RealViewer->GetClass();
	if (PlayerControllerClass && PlayerControllerClass->IsChildOf(APlayerController::StaticClass()))
	{
		return true;
	}

	return Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
}

//See if passed in player state matches this playerstate
bool ASteamBeaconPlayerState::IsPlayer(APlayerState* InPlayerState)
{
	if (InPlayerState)
	{
		return (InPlayerState->UniqueId == UniqueId);
	}
	return false;
}

//Check if passed in player is Party Leader
bool ASteamBeaconPlayerState::IsPlayerPartyLeader(APlayerState* InPlayerState)
{
	if (InPlayerState)
	{
		return (InPlayerState->UniqueId == PartyOwnerUniqueId);
	}
	return false;
}

//Check if we are the Party leader
bool ASteamBeaconPlayerState::IsPartyLeader()
{
	return (UniqueId == PartyOwnerUniqueId);
}

//Validation, no restriction set
bool ASteamBeaconPlayerState::ServerSetPlayerProfileLevel_Validate(int32 InPlayerProfileLevel)
{
	return true;
}

void ASteamBeaconPlayerState::ServerSetPlayerProfileLevel_Implementation(int32 InPlayerProfileLevel)
{
	PlayerProfileLevel = InPlayerProfileLevel;
	UE_LOG(LogSteamBeacon, Verbose, TEXT("SteamBeaconState::ServerSetPlayerProfileLevel_Implementation: Profile Level Received %i"), PlayerProfileLevel);

	//NotifyLobby, thanks have changed
	ASteamBeaconClient* pSBClient = Cast<ASteamBeaconClient>(ClientActor);
	if (pSBClient && pSBClient->GetPartyState())
	{
		//Multicast the change, but 
		pSBClient->GetPartyState()->OnLobbyStateInfoUpdated();
	}
}


void ASteamBeaconPlayerState::SendPartyMessage(FPartyMessage ChatMessage)
{
	//Perform any filtering here
	/////
	
	//Send to server
	ServerSendPartyMessage(ChatMessage);
}

bool ASteamBeaconPlayerState::ServerSendPartyMessage_Validate(FPartyMessage ChatMessage)
{
	return true;
}

void ASteamBeaconPlayerState::ServerSendPartyMessage_Implementation(FPartyMessage ChatMessage)
{
	ASteamBeaconClient* pSBClient = Cast<ASteamBeaconClient>(ClientActor);
	if (pSBClient && pSBClient->GetPartyState())
	{
		//Party States handles centralized distribution
		pSBClient->GetPartyState()->ProcessChatMessage(this, ChatMessage);
	}
}

void ASteamBeaconPlayerState::OnPartyMessageReceived_Implementation(FPartyMessage InPartyMessage)
{
	//Broadcast Delegates
	OnChatMessageReceivedEvent().Broadcast(InPartyMessage);
}
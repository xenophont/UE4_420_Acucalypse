// Copyright 2013-2017 W3 Studios, LLC. All Rights Reserved.

#include "SteamBeaconState.h"
#include "SteamParty.h"
#include "SteamBeaconPlayerState.h"
#include "SteamBeaconClient.h"
#include "SteamBeaconGameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

//Party State, much like a GameState, Replicates All Party info to partymembers
ASteamBeaconState::ASteamBeaconState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	LobbyBeaconPlayerStateClass = ASteamBeaconPlayerState::StaticClass();

	//Force this Actor Always Revelant to prevent closing down its actor net channel
	bReplicates = true;
	bAlwaysRelevant = true;
	bOnlyRelevantToOwner = false;
	//NetPriority = 3.0;
}

//Replicated Variables registering
void ASteamBeaconState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ASteamBeaconState, PartyOwnerUniqueId);
}

bool ASteamBeaconState::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	//If Voice Chat is enabled PCs are now the owners instead of the beacons for internal handling
	UClass* PlayerControllerClass = RealViewer->GetClass();
	if (PlayerControllerClass && PlayerControllerClass->IsChildOf(APlayerController::StaticClass()))
	{
		return true;
	}

	return Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
}

//Relay connection to the BeaconClients
void ASteamBeaconState::ConnectPartyToGameServer(const FSteamBeaconSessionResult& InSearchResult, bool bIsPartyHostCreatingServer)
{
	TArray<FLobbyPlayerStateActorInfo>& AllPlayers = Players.GetAllPlayers();
	
	if (AllPlayers.Num())
	{
		for (const FLobbyPlayerStateActorInfo& Player : AllPlayers)
		{
			ALobbyBeaconPlayerState* PlayerState = Player.LobbyPlayerState;
			if (PlayerState && PlayerState->bInLobby && PlayerState->ClientActor)
			{
				ASteamBeaconClient* pSteamBeaconClient = Cast<ASteamBeaconClient>(PlayerState->ClientActor);
				if (pSteamBeaconClient)
				{
					UE_LOG(LogSteamBeacon, Verbose, TEXT("ClientJoinGameSession Called on Server"));
					pSteamBeaconClient->ClientJoinGameSession(InSearchResult, bIsPartyHostCreatingServer);
					pSteamBeaconClient->ForceNetUpdate();
				}
			}
		}
	}
}

void ASteamBeaconState::ConnectPartyToGameServer(FString GameServerURL)
{
	TArray<FLobbyPlayerStateActorInfo>& AllPlayers = Players.GetAllPlayers();

	if (AllPlayers.Num())
	{
		for (const FLobbyPlayerStateActorInfo& Player : AllPlayers)
		{
			ALobbyBeaconPlayerState* PlayerState = Player.LobbyPlayerState;
			if (PlayerState && PlayerState->bInLobby && PlayerState->ClientActor)
			{
				ASteamBeaconClient* pSteamBeaconClient = Cast<ASteamBeaconClient>(PlayerState->ClientActor);
				if (pSteamBeaconClient)
				{
					pSteamBeaconClient->ClientJoinGameServer(GameServerURL);
				}
			}
		}
	}
}

ASteamBeaconPlayerState* ASteamBeaconState::GetPartyPlayer(const FUniqueNetIdRepl& UniqueId)
{
	return Cast<ASteamBeaconPlayerState>(GetPlayer(UniqueId));
}

void ASteamBeaconState::GetAllPartyMembers(TArray<ASteamBeaconPlayerState*>& InPlayerArray)
{
	InPlayerArray.Empty();
	TArray<FLobbyPlayerStateActorInfo>& AllPlayers = Players.GetAllPlayers();

	if (AllPlayers.Num())
	{
		for (const FLobbyPlayerStateActorInfo& Player : AllPlayers)
		{
			ASteamBeaconPlayerState* PlayerState = Cast<ASteamBeaconPlayerState>(Player.LobbyPlayerState);
			if (PlayerState && PlayerState->bInLobby)
			{
				//Add Player to the Array
				InPlayerArray.Add(PlayerState);
			}
		}
	}
}

void ASteamBeaconState::OnPreLobbyStartedTickInternal(double DeltaTime)
{
	//This starts the lobby in 20 secons by default, not used in our system
	//Super::OnPreLobbyStartedTickInternal(DeltaTime);

	//UE_LOG(LogSteamBeacon, Warning, TEXT("SteamBeaconState::OnPreLobbyStartedTickInternal: Tick Tick Tick"));

	/*TArray<FLobbyPlayerStateActorInfo> RefreshPlayerArray = Players.GetAllPlayers();
	
	//Force network updates
	if (RefreshPlayerArray.Num())
	{
		for (int32 i = 0; i < RefreshPlayerArray.Num(); i++)
		{
			Players.MarkItemDirty(RefreshPlayerArray[i]);
		}
	}*/
}

void ASteamBeaconState::OnPostLobbyStartedTickInternal(double DeltaTime)
{
	//Per 1 second Tick afer lobby has started
}

void ASteamBeaconState::DumpPartyState() const
{
	//Find and Print out leader Info
	TArray<FLobbyPlayerStateActorInfo> PlayersArray = Players.GetAllPlayers();
	if (PlayersArray.Num())
	{
		for (const FLobbyPlayerStateActorInfo& Item : PlayersArray)
		{
			ASteamBeaconPlayerState* pPlayerState = Cast<ASteamBeaconPlayerState>(PlayersArray[0].LobbyPlayerState);
			if (pPlayerState && (pPlayerState->UniqueId == pPlayerState->PartyOwnerUniqueId))
			{
				UE_LOG(LogSteamBeacon, Display, TEXT("PartyLeader: %s (%s)"), *pPlayerState->DisplayName.ToString(), *pPlayerState->UniqueId.ToString());
			}
		}		
	}
	
	//Base class call
	DumpState();
}

//New or Changed Info on the LobbyState generic refresh call
void ASteamBeaconState::OnLobbyStateInfoUpdated_Implementation()
{
	UE_LOG(LogSteamBeacon, Log, TEXT("ASteamBeaconState::OnLobbyStateInfoUpdated_Implementation"));
	//Instant Broadcast the change notification to all listeners
	OnPartyStateChangedEvent().Broadcast();

	//Set a Timer to refresh in 1 second due to packet order for the net variable may be after RPC call
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ASteamBeaconState::OnLobbyStateInfoUpdated_NetRefresh, 2.f, false);
}

void ASteamBeaconState::OnLobbyStateInfoUpdated_NetRefresh()
{
	UE_LOG(LogSteamBeacon, Log, TEXT("ASteamBeaconState::OnLobbyStateInfoUpdated_NetRefresh"));
	//Broadcast the change notification to all listeners
	OnPartyStateChangedEvent().Broadcast();
}

//Process and distribute incoming chat messages
void ASteamBeaconState::ProcessChatMessage(ASteamBeaconPlayerState* InPlayerState, FPartyMessage InChatMessage)
{
	//Build the Identifications
	FPartyMessage NewMessage = InChatMessage;
	NewMessage.SenderName = InPlayerState->DisplayName.ToString();
	NewMessage.SenderId = InPlayerState->UniqueId;
		
	//Dispatch the message out to all the players
	TArray<FLobbyPlayerStateActorInfo> PlayersArray = Players.GetAllPlayers();
	if (PlayersArray.Num())
	{
		for (const FLobbyPlayerStateActorInfo& Item : PlayersArray)
		{
			//Don't send to Sender
			ASteamBeaconPlayerState* pPlayerState = Cast<ASteamBeaconPlayerState>(Item.LobbyPlayerState);
			if (pPlayerState && (pPlayerState->UniqueId != NewMessage.SenderId))
			{
				if (NewMessage.ChatType == EPartyChatType::Global)
				{
					pPlayerState->OnPartyMessageReceived(NewMessage);
					continue;
				}
				else if (NewMessage.ChatType == EPartyChatType::Whisper)
				{
					if ((pPlayerState->UniqueId == NewMessage.ReceiverId) || (pPlayerState->UniqueId == NewMessage.SenderId))
					{
						pPlayerState->OnPartyMessageReceived(NewMessage);
					}
					continue;
				}
			}
		}
	}
}

// Override to allow for project specific beaconplayerstates, for custom blueprintable data replication
ALobbyBeaconPlayerState* ASteamBeaconState::CreateNewPlayer(const FText& PlayerName, const FUniqueNetIdRepl& UniqueId)
{
	//Check GameInstance if project is supplying a cusom BeaconPlayerState blueprint for custom data replication, if not use default
	USteamBeaconGameInstance* pGI = Cast<USteamBeaconGameInstance>(GetWorld()->GetGameInstance());
	if (pGI && pGI->LobbyBeaconPlayerStateClass)
	{
		LobbyBeaconPlayerStateClass = pGI->LobbyBeaconPlayerStateClass;
	}
	
	UE_LOG(LogSteamBeacon, Verbose, TEXT("ASteamBeaconState::CreateNewPlayer: BeaconPlayerStateCreated!"));
	return Super::CreateNewPlayer(PlayerName, UniqueId);
}
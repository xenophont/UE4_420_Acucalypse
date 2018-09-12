// Copyright 2013-2017 W3 Studios, LLC. All Rights Reserved.

#include "SteamBeaconClient.h"
#include "SteamParty.h"
#include "SteamBeaconPlayerState.h"
#include "SteamBeaconGameInstance.h"
#include "OnlineSubsystemUtils.h"
#include "Net/UnrealNetwork.h"
#include "DummyPlayerController.h"

//Replicated Variable Registration
void ASteamBeaconClient::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASteamBeaconClient, BeaconLobbyState);
	DOREPLIFETIME(ASteamBeaconClient, BeaconPlayerState);
}

//Constructor
ASteamBeaconClient::ASteamBeaconClient(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//Force this Actor Always Revelant to prevent closing down its actor net channel
	bReplicates = true;
	bAlwaysRelevant = true;
	//NetUpdateFrequency = 100.0f;
	//NetPriority = 4.0;
	bOnlyRelevantToOwner = false;

	//PrimaryActorTick.bCanEverTick = true;
	//PrimaryActorTick.bTickEvenWhenPaused = true;
}

//Fix up netrelevancy for dummy playercontroller owners
bool ASteamBeaconClient::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	//If Voice Chat is enabled PCs are now the owners instead of the beacons for internal handling
	UClass* PlayerControllerClass = RealViewer->GetClass();
	if (PlayerControllerClass && PlayerControllerClass->IsChildOf(APlayerController::StaticClass()))
	{
		return true;
	}

	return Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
}

bool ASteamBeaconClient::NotifyAcceptingChannel(UChannel* Channel)
{
	//Base Class will perform checks on the data for validity, no need to do it again
	bool bResult = Super::NotifyAcceptingChannel(Channel);

	if (bResult)
	{
		if (!Channel->Connection->PlayerController)
		{
			SetupDummyPCForVoiceChat(Channel->Connection);
		}
	}
	return bResult;
}


void ASteamBeaconClient::SetupDummyPCForVoiceChat(UNetConnection* Connection)
{
	//We need to add a dummy PlayerController here to satisfy internal checks for Voice Chatting
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = Instigator;
	SpawnInfo.ObjectFlags |= RF_Transient;	// We never want to save player controllers into a map
	ADummyPlayerController* DummyPC = GetWorld()->SpawnActor<ADummyPlayerController>(ADummyPlayerController::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);
	
	//Save out the pointer to the connection
	Connection->PlayerController = DummyPC;
}

bool ASteamBeaconClient::ServerSetHandshakeComplete_Validate()
{
	return true;
}

void ASteamBeaconClient::ServerSetHandshakeComplete_Implementation()
{
	UE_LOG(LogSteamBeacon, Log, TEXT("ASteamBeaconClient::ServerSetHandshakeComplete_Implementation"));
	ASteamBeaconHost* BeaconHost = Cast<ASteamBeaconHost>(GetBeaconOwner());
	if (BeaconHost)
	{
		//Inform Host that Handshaking is complete and all variables have been setup through their replication channels for this beaconclient
		BeaconHost->SetHandshakeCompleteForBeacon(this);
	}
}


void ASteamBeaconClient::ServerLoginPlayer_Implementation(const FString& InSessionId, const FUniqueNetIdRepl& InUniqueId, const FString& UrlString)
{
	UE_LOG(LogSteamBeacon, Log, TEXT("ServerLoginPlayer %s %s."), *InUniqueId.ToString(), *UrlString);
	ASteamBeaconHost* BeaconHost = Cast<ASteamBeaconHost>(GetBeaconOwner());
	if (BeaconHost)
	{
		//BeaconHost->ProcessLogin(this, InSessionId, InUniqueId, UrlString); //Reroute Base Class
		BeaconHost->ProcessPlayerLogin(this, InSessionId, InUniqueId, UrlString);
	}
}

//Player has joined the Party
void ASteamBeaconClient::ClientPlayerJoined_Implementation(const FText& NewPlayerName, const FUniqueNetIdRepl& InUniqueId)
{
	UE_LOG(LogSteamBeacon, Log, TEXT("ClientPlayerJoined %s %s."), *NewPlayerName.ToString(), *InUniqueId.ToString());

	if (GetNetMode() != NM_Standalone)
	{
		IOnlineSessionPtr SessionInt = Online::GetSessionInterface(GetWorld());
		if (SessionInt.IsValid() && InUniqueId.IsValid())
		{
			// Register the player as part of the session
			SessionInt->RegisterPlayer(PartySessionName, *InUniqueId, false);
		}
	}

	OnPlayerJoined().ExecuteIfBound(NewPlayerName, InUniqueId);
}

//Player has left the Party
void ASteamBeaconClient::ClientPlayerLeft_Implementation(const FUniqueNetIdRepl& InUniqueId)
{
	UE_LOG(LogSteamBeacon, Log, TEXT("ClientPlayerLeft %s"), *InUniqueId.ToString());

	if (GetNetMode() != NM_Standalone)
	{
		IOnlineSessionPtr SessionInt = Online::GetSessionInterface(GetWorld());
		if (SessionInt.IsValid() && InUniqueId.IsValid())
		{
			// Register the player as part of the session
			SessionInt->UnregisterPlayer(PartySessionName, *InUniqueId);
		}
	}

	OnPlayerLeft().ExecuteIfBound(InUniqueId);	
}

//Set the Party Leader
void ASteamBeaconClient::ServerSetPartyOwner_Implementation(const FUniqueNetIdRepl& InUniqueId, const FUniqueNetIdRepl& InPartyOwnerId)
{
	if (PlayerState)
	{
		PlayerState->PartyOwnerUniqueId = InPartyOwnerId;
	}
}

void ASteamBeaconClient::ClientJoinGameSession_Implementation(FSteamBeaconSessionResult InSearchResult, bool bIsPartyHostCreatingServer)
{
	UE_LOG(LogSteamBeacon, Log, TEXT("ClientJoinGameSession signal Logged On == %d"), bLoggedIn);
	if (bLoggedIn)
	{
		USteamBeaconGameInstance* pGI = Cast<USteamBeaconGameInstance>(GetWorld()->GetGameInstance());
		if (pGI)
		{
			if (!pGI->JoinSessionFromSteamBeacon(InSearchResult, bIsPartyHostCreatingServer))
			{
				UE_LOG(LogSteamBeacon, Warning, TEXT("ClientJoinGameSession JoinSessionFromSteamBeacon Failed"));
			}
		}
	}
}

void ASteamBeaconClient::ClientJoinGameServer_Implementation(const FString& GameServerURL)
{
	UE_LOG(LogSteamBeacon, Log, TEXT("ClientJoinGameServer signal Logged On == %d"), bLoggedIn);
	if (bLoggedIn)
	{
		USteamBeaconGameInstance* pGI = Cast<USteamBeaconGameInstance>(GetWorld()->GetGameInstance());
		if (pGI)
		{
			//Travel to Game Server URL
			pGI->DirectTravelToGameSession(GameServerURL);
		}
	}
}

ASteamBeaconState* ASteamBeaconClient::GetPartyState()
{
	return Cast<ASteamBeaconState>(LobbyState);
}

ASteamBeaconPlayerState* ASteamBeaconClient::GetPlayerState()
{
	return Cast<ASteamBeaconPlayerState>(PlayerState);
}

void ASteamBeaconClient::OnRep_PlayerState()
{
	if (BeaconPlayerState)
	{
		//Register with correct NetDriver for Server RPC's
		BeaconPlayerState->SetNetDriverName(GetNetDriverName());

		UE_LOG(LogSteamBeacon, Log, TEXT("Driver(%s), ASteamBeaconClient::OnRep_PlayerState: %s"), *GetNetDriverName().ToString(), *BeaconPlayerState->GetDisplayName().ToString());
	}
	//Send Event Player State Received for any functions listening
	OnPlayerStateEvent().ExecuteIfBound(Cast<ASteamBeaconPlayerState>(BeaconPlayerState));

	//Once we have replicated and setup all the actor channels, tell the server handshaking is complete, mainly implemented to turn on Voice Chat
	if (BeaconPlayerState && BeaconLobbyState)
	{
		//Possibly called in 2 locations due to packet orders can get mixed 
		OnReplicatedActorsSet();
	}
}

void ASteamBeaconClient::SetLobbyStateOverride(ALobbyBeaconState* InLobbyState)
{
	SetLobbyState(InLobbyState);
	BeaconLobbyState = Cast<ASteamBeaconState>(InLobbyState); 
}

void ASteamBeaconClient::OnRep_LobbyState()
{
	if (BeaconLobbyState)
	{
		//Register with correct NetDriver for Server RPC's
		BeaconLobbyState->SetNetDriverName(GetNetDriverName());

		UE_LOG(LogSteamBeacon, Log, TEXT("Driver(%s), ASteamBeaconClient::OnRep_LobbyState: %s"), *GetNetDriverName().ToString(), *BeaconLobbyState->GetName());
	}
	
	//Once we have replicated and setup all the actor channels, tell the server handshaking is complete, mainly implemented to turn on Voice Chat
	if (BeaconPlayerState && BeaconLobbyState)
	{
		//Possibly called in 2 locations due to packet orders can get mixed 
		OnReplicatedActorsSet();
	}
}

void ASteamBeaconClient::OnReplicatedActorsSet()
{
	//Send the Handshake Signal back on all ActorData Channels hookedup, safe to transmit
	if (!bHasHandShakeCompleted)
	{
		ServerSetHandshakeComplete();
		bHasHandShakeCompleted = true;

		USteamBeaconGameInstance* pSBGI = Cast<USteamBeaconGameInstance>(GetWorld()->GetGameInstance());
		if (pSBGI && pSBGI->VoiceChatEnabled)
		{
			//Its now Okay to start sending Voice Data, so Unmute the Player Controller
			UNetConnection* pConnection = GetNetConnection();
			if (pConnection && pConnection->PlayerController)
			{
				pConnection->PlayerController->MuteList.bHasVoiceHandshakeCompleted = true;
			}
		}
	}
}

void ASteamBeaconClient::ServerDisconnectFromLobby_Implementation()
{
	//Before Disconnectiong we need to change our Connection owner for proper NetCleanup back to SteamBeacon instead of PC
	UNetConnection* pConnection = GetNetConnection();
	if (pConnection)
	{
		if (pConnection->PlayerController)
		{
			//Remove our Dummy PlayerController
			pConnection->PlayerController->Destroy();
			pConnection->PlayerController = nullptr;
		}
		pConnection->OwningActor = this;
	}
	
	Super::ServerDisconnectFromLobby_Implementation();
}


void ASteamBeaconClient::ServerKickPlayer_Implementation(const FUniqueNetIdRepl& PlayerToKick, const FText& Reason)
{
	Super::ServerKickPlayer_Implementation(PlayerToKick, Reason);
}

void ASteamBeaconClient::ClientWasKicked_Implementation(const FText& KickReason)
{
	Super::ClientWasKicked_Implementation(KickReason);

	LeaveParty();
}

void ASteamBeaconClient::OnNetCleanup(class UNetConnection* Connection)
{
	//Request to disconnect from the P2P session instead of waiting for default timeouts (25s)
	USteamBeaconGameInstance* pGI = Cast<USteamBeaconGameInstance>(GetWorld()->GetGameInstance());
	if (pGI && GetUniqueId().IsValid())
	{
		CSteamID SteamPlayerId(*(uint64*)GetUniqueId()->GetBytes());
		pGI->RemoveSteamP2PSession(&SteamPlayerId);
	}
		
	Super::OnNetCleanup(Connection);
}

//Remove ourself from the party
void ASteamBeaconClient::LeaveParty()
{
	DisconnectFromLobby();

	//Clear the Beacons
	USteamBeaconGameInstance* pGI = Cast<USteamBeaconGameInstance>(GetWorld()->GetGameInstance());
	if (pGI)
	{
		pGI->ClearPartyBeacons();
		//Clear party info, by resaving after clearpartybeacons
		pGI->SavePartyInfo();
	}

	//Notify ourselves we have been removed from the party, mainly used to fire off PlayerController Events, for UMG updates, cleanup ect..
	if (GetPartyState())
	{
		GetPartyState()->OnPlayerLobbyStateRemoved().Broadcast(PlayerState);
	}

	//Clean up any Online Party Sessions
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		IOnlineSessionPtr SessionInt = OnlineSub->GetSessionInterface();
		if (SessionInt.IsValid())
		{
			//Destroy the Session Record
			SessionInt->DestroySession(PartySessionName);
			//SessionInt->RemoveNamedSession(PartySessionName);
		}
	}
}

//Party has been disbanded, leave now 
void ASteamBeaconClient::OnPartyDisbanded_Implementation()
{
	LeaveParty();
}

void ASteamBeaconClient::DestroyBeacon()
{
	//Request to disconnect from the P2P session instead of waiting for default timeouts (25s)
	USteamBeaconGameInstance* pGI = Cast<USteamBeaconGameInstance>(GetWorld()->GetGameInstance());
	if (pGI && PlayerState)
	{
		CSteamID SteamPlayerId(*(uint64*)PlayerState->PartyOwnerUniqueId->GetBytes());
		pGI->RemoveSteamP2PSession(&SteamPlayerId);
	}

	Super::DestroyBeacon();
}

void ASteamBeaconClient::ClientWasKickedFromLobby(const FText& KickReason)
{
	ClientWasKicked(KickReason);
}




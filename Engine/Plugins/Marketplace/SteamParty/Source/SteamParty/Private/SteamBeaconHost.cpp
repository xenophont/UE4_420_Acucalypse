// Copyright 2013-2017 W3 Studios, LLC. All Rights Reserved.

#include "SteamBeaconHost.h"
#include "SteamParty.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "SteamBeaconState.h"
#include "SteamBeaconClient.h"
#include "OnlineSubsystemUtils.h"
#include "Engine/NetConnection.h"
#include "Engine/World.h"
#include "DummyPlayerController.h"
#include "SteamBeaconGameInstance.h"



//The Beacon Host, Authority of the Party similiar to AGameModeBase but for Beacons
ASteamBeaconHost::ASteamBeaconHost(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ClientBeaconActorClass = ASteamBeaconClient::StaticClass();
	LobbyStateClass = ASteamBeaconState::StaticClass();
}

//Process the PLayer Login, exposed from lobby parent
void ASteamBeaconHost::ProcessPlayerLogin(ASteamBeaconClient* ClientActor, const FString& InSessionId, const FUniqueNetIdRepl& InUniqueId, const FString& UrlString)
{
	UE_LOG(LogBeacon, Verbose, TEXT("ProcessLogin %s SessionId %s %s %s from (%s)"),
		ClientActor ? *ClientActor->GetName() : TEXT("NULL"),
		*InSessionId,
		*InUniqueId.ToString(),
		*UrlString,
		(ClientActor&&ClientActor->GetNetConnection()) ? *ClientActor->GetNetConnection()->LowLevelDescribe() : TEXT("NULL"));

	if (ClientActor)
	{
		bool bSuccess = false;
		if (DoesSessionMatch(InSessionId) && InUniqueId.IsValid())
		{
			FURL InURL(NULL, *UrlString, TRAVEL_Absolute);
			if (InURL.Valid)
			{
				// Make the option string.
				FString Options;
				for (int32 i = 0; i < InURL.Op.Num(); i++)
				{
					Options += TEXT('?');
					Options += InURL.Op[i];
				}

				if (PreLogin(InUniqueId, Options))
				{
					ASteamBeaconPlayerState* NewPlayer = HandlePlayerLoginOverride(ClientActor, InUniqueId, Options);
					if (NewPlayer && NewPlayer->IsValid())
					{
						bSuccess = true;

						NewPlayer->SetOwner(ClientActor);
						ClientActor->PlayerState = NewPlayer;
						ClientActor->BeaconPlayerState = NewPlayer;
						
						NewPlayer->ClientActor = ClientActor;

						ClientActor->SetLobbyStateOverride(LobbyState);
						for (AOnlineBeaconClient* ExistingClient : ClientActors)
						{
							if (ExistingClient != ClientActor)
							{
								ASteamBeaconClient* LBC = Cast<ASteamBeaconClient>(ExistingClient);
								LBC->ClientPlayerJoinedOverride(NewPlayer->DisplayName, NewPlayer->UniqueId);
							}
						}
					}
				}
			}

			if (LobbyState && !LobbyState->HasLobbyStarted())
			{
				int32 NumLobbyPlayers = LobbyState->GetNumPlayers();
				if (NumLobbyPlayers == LobbyState->GetMaxPlayers())
				{
					// Session is full, load into game
					LobbyState->StartLobby();
				}
				else if (NumLobbyPlayers == 1)
				{
					// First player has joined, start waiting for others
					LobbyState->StartWaiting();
				}
			}
		}

		ClientActor->ClientLoginCompleteOverride(InUniqueId, bSuccess);
		//ClientActor->ClientLoginComplete(InUniqueId, bSuccess);
		//ClientActor->bLoggedIn = bSuccess;

		if (bSuccess)
		{
			PostLogin(ClientActor);
		}
		else
		{
			DisconnectClient(ClientActor);
		}
	}
}

//Player completely logged in, handle post login here
void ASteamBeaconHost::PostLogin(ALobbyBeaconClient* ClientActor)
{
	ASteamBeaconClient* BeaconClientActor = Cast<ASteamBeaconClient>(ClientActor);
	if (!BeaconClientActor) return;

	//Register Correct NetDriverName for clients
	ASteamBeaconPlayerState* pPlayerState = Cast<ASteamBeaconPlayerState>(ClientActor->PlayerState);
	if (pPlayerState && GetPartyState())
	{
		pPlayerState->PartyOwnerUniqueId = GetPartyState()->PartyOwnerUniqueId;
		//ForceNetRelevant();
	}
		
	//Everything is logged in for player and playerstates created at this point, do the dummypc
	if (BeaconClientActor && BeaconClientActor->GetNetConnection() && !BeaconClientActor->GetNetConnection()->PlayerController)
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Instigator = Instigator;
		SpawnInfo.ObjectFlags |= RF_Transient;	// We never want to save player controllers into a map
		ADummyPlayerController* DummyPC = GetWorld()->SpawnActor<ADummyPlayerController>(ADummyPlayerController::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);
		if (DummyPC)
		{
			//DummyPC->NetPlayerIndex = 0;
			DummyPC->Role = ROLE_Authority;
			DummyPC->SetReplicates(false);
			//DummyPC->bAlwaysRelevant = true;
			//DummyPC->bOnlyRelevantToOwner = false;
			//DummyPC->SetAutonomousProxy(false);
			DummyPC->SetNetDriverName(NetDriverName);
			DummyPC->Player = BeaconClientActor->GetNetConnection();

			BeaconClientActor->GetNetConnection()->PlayerController = DummyPC;
			BeaconClientActor->GetNetConnection()->OwningActor = DummyPC;

			UE_LOG(LogSteamBeacon, Verbose, TEXT("ASteamBeaconHost::PostLogin: Connection->OwningActor = DummyPC"));
		}
	}

	//Now Register Player to Party session
	//RegisterPlayerToPartySession(BeaconClientActor);
	//UpdateSessionPartyOnline();

}

void ASteamBeaconHost::UpdateSessionPartyOnline()
{
	IOnlineSessionPtr SessionInt = Online::GetSessionInterface(GetWorld());
	if (SessionInt.IsValid())
	{
		FOnlineSessionSettings* SessionSettings = SessionInt->GetSessionSettings(SessionName);
		if (SessionSettings != nullptr)
		{
			//SessionSettings->bShouldAdvertise = bPublicSearchable;
			//SessionSettings->bAllowInvites = bAllowInvites;
			//SessionSettings->bAllowJoinViaPresence = bJoinViaPresence && !bJoinViaPresenceFriendsOnly;
			//SessionSettings->bAllowJoinViaPresenceFriendsOnly = bJoinViaPresenceFriendsOnly;
			//SessionSettings->NumPublicConnections = Settings.MaxPlayers;
			SessionInt->UpdateSession(PartySessionName, *SessionSettings, true);
		}
	}
}


void ASteamBeaconHost::SetHandshakeCompleteForBeacon(ASteamBeaconClient* ClientActor)
{
	if (ClientActor)
	{
		//Its now Okay to start sending Voice Data, so Unmute the Player Controller
		UNetConnection* pConnection = ClientActor->GetNetConnection();
		if (pConnection && pConnection->PlayerController)
		{
			USteamBeaconGameInstance* pSBGI = Cast<USteamBeaconGameInstance>(GetWorld()->GetGameInstance());
			if (pSBGI && pSBGI->VoiceChatEnabled)
			{
				pConnection->PlayerController->MuteList.bHasVoiceHandshakeCompleted = true;
			}
		}
	}
}



//Client Beacon is connected and created, delegate attached to SteamBeaconListen parent class
//BeaconClient is now fully replicating the data at this point
void ASteamBeaconHost::OnClientConnected(AOnlineBeaconClient* NewClientActor, UNetConnection* ClientConnection)
{
	UE_LOG(LogSteamBeacon, Verbose, TEXT("OnClientConnected %s from (%s)"),
		NewClientActor ? *NewClientActor->GetName() : TEXT("NULL"),
		(NewClientActor&&NewClientActor->GetNetConnection()) ? *NewClientActor->GetNetConnection()->LowLevelDescribe() : TEXT("NULL"));

	ASteamBeaconClient* pSBClient = Cast<ASteamBeaconClient>(NewClientActor);
	if (!pSBClient) return;
	ClientActors.Add(NewClientActor);
}

//Connect all Party Members to Game Server from Session result
void ASteamBeaconHost::ConnectPartyToGameServer(const FSteamBeaconSessionResult& InSearchResult, bool bIsPartyHostCreatingServer)
{
	ASteamBeaconState* pSteamBeaconState = GetPartyState();
	if (pSteamBeaconState)
	{
		bIsJoiningGame = true;
		pSteamBeaconState->ConnectPartyToGameServer(InSearchResult, bIsPartyHostCreatingServer);
	}
}

//Connect all Party members to Game Server from Direct Server Address
void ASteamBeaconHost::ConnectPartyToGameServer(FString GameServerURL)
{
	ASteamBeaconState* pSteamBeaconState = GetPartyState();
	if (pSteamBeaconState)
	{
		bIsJoiningGame = true;
		pSteamBeaconState->ConnectPartyToGameServer(GameServerURL);
	}
}

//Base class override to expose needed variables
ASteamBeaconPlayerState* ASteamBeaconHost::HandlePlayerLoginOverride(ALobbyBeaconClient* ClientActor, const FUniqueNetIdRepl& InUniqueId, const FString& Options)
{
	UWorld* World = GetWorld();
	check(World);

	FString NewPlayerName = UGameplayStatics::ParseOption(Options, TEXT("Name")).Left(20);
	if (NewPlayerName.IsEmpty())
	{
		NewPlayerName = TEXT("UnknownUser");
	}

	FString InGameAccountId = UGameplayStatics::ParseOption(Options, TEXT("GameAccountId"));
	FString InAuthTicket = UGameplayStatics::ParseOption(Options, TEXT("AuthTicket"));
	UE_LOG(LogSteamBeacon, Log, TEXT("Lobby beacon received GameAccountId and AuthTicket from client for player: UniqueId:[%s] GameAccountId=[%s]"),
		InUniqueId.IsValid() ? *InUniqueId->ToString() : TEXT("-invalid-"),
		*InGameAccountId);

	if (GetNetMode() != NM_Standalone)
	{
		IOnlineSessionPtr SessionInt = Online::GetSessionInterface(World);
		if (SessionInt.IsValid() && InUniqueId.IsValid())
		{
			// Register the player as part of the session
			bool bWasFromInvite = UGameplayStatics::HasOption(Options, TEXT("bIsFromInvite"));
			SessionInt->RegisterPlayer(PartySessionName, *InUniqueId, bWasFromInvite);

			// Update the Online data
			UpdateSessionPartyOnline();
		}
	}

	FText DisplayName = FText::FromString(NewPlayerName);
	ALobbyBeaconPlayerState* NewLobbyPlayer = LobbyState->AddPlayer(DisplayName, InUniqueId);

	ASteamBeaconState* pPartyState = GetPartyState();
	if (pPartyState)
	{
		// Broadcast Party / Lobby has been changed
		pPartyState->OnLobbyStateInfoUpdated();
	}

	return Cast<ASteamBeaconPlayerState>(NewLobbyPlayer);
}


void ASteamBeaconHost::HandlePlayerLogout(const FUniqueNetIdRepl& InUniqueId)
{
	Super::HandlePlayerLogout(InUniqueId);
}

//Return the party state
ASteamBeaconState* ASteamBeaconHost::GetPartyState()
{
	return Cast<ASteamBeaconState>(LobbyState);
}

//Remove all members from the party and shut down the steam beacons
void ASteamBeaconHost::DisbandParty()
{
	//Flag to ignore disconnects
	bIsDisbandingParty = true;

	if (ClientActors.Num())
	{
		ASteamBeaconClient* pHostBeaconClient = nullptr;
		for (AOnlineBeaconClient* ExistingClient : ClientActors)
		{
			ASteamBeaconClient* pBeaconClient = Cast<ASteamBeaconClient>(ExistingClient);
			if (pBeaconClient && pBeaconClient->PlayerState)	// && (pBeaconClient->GetConnectionState()==EBeaconConnectionState::Open))
			{
				switch(pBeaconClient->GetConnectionState())
				{
					case EBeaconConnectionState::Open:
					case EBeaconConnectionState::Pending:
					{
						pBeaconClient->OnPartyDisbanded();
						break;
					}
					case EBeaconConnectionState::Invalid:
					{
						//Save the Host Beacon Client
						pHostBeaconClient = pBeaconClient;
						break;
					}
					default:
					{
						//Already closed connection
						break;
					}
				}
				
			}
		}

		//Disband ourselves separately
		if (pHostBeaconClient)
		{
			pHostBeaconClient->OnPartyDisbanded();
		}

		//Broadcast our lobby update
		if (GetPartyState())
		{
			GetPartyState()->OnPartyStateChangedEvent().Broadcast();
		}
	}
}

//Debugging information
void ASteamBeaconHost::DumpPartyState() const
{
	UE_LOG(LogSteamBeacon, Display, TEXT("Lobby Beacon: %s"), *GetBeaconType());

	ASteamBeaconState* pSBState = Cast<ASteamBeaconState>(LobbyState);
	if (pSBState)
	{
		pSBState->DumpPartyState();
	}
}


//Client has disconnected from party
void ASteamBeaconHost::NotifyClientDisconnected(AOnlineBeaconClient* LeavingClientActor)
{
	//If Party is joining game, then ignore disconnects
	if (bIsJoiningGame)
	{
		UE_LOG(LogSteamBeacon, Verbose, TEXT("ASteamBeaconHost::NotifyClientDisconnected: Ignore, we are joining game"));
		return;
	}
	//If Party is disbanding, also ignore disconnects
	if (bIsDisbandingParty)
	{
		UE_LOG(LogSteamBeacon, Verbose, TEXT("ASteamBeaconHost::NotifyClientDisconnected: Ignore, we are Disbanding Party"));
		return;
	}

	ASteamBeaconState* pPartyState = GetPartyState();
	if (pPartyState)
	{
		ALobbyBeaconPlayerState* Player = pPartyState->GetPlayer(LeavingClientActor);
		if (Player && Player->bInLobby)
		{
			// Handle beacon connection termination code (code the GameMode would normal handle)
			UWorld* World = GetWorld();
			check(World);

			AGameModeBase* GameMode = World->GetAuthGameMode();
			check(GameMode);
			check(GameMode->GameSession);

			//Fix the gamesession to be not Standalone 
			GameMode->GameSession->SetNetDriverName(GetNetDriverName());

			// Notify the session (updates reservation beacon, unregisters the player, etc)
			GameMode->GameSession->NotifyLogout(PartySessionName, Player->UniqueId);
			HandlePlayerLogout(Player->UniqueId);

			// Update the Online data
			//UpdateSessionPartyOnline();

		}

		//Broadcast Party/Lobby has been changed
		pPartyState->OnLobbyStateInfoUpdated();
	}
	else
	{
		UE_LOG(LogSteamBeacon, Warning, TEXT("No lobby beacon state to handle disconnection!"));
	}

	//Skip through all derived classes in between
	AOnlineBeaconHostObject::NotifyClientDisconnected(LeavingClientActor);

	// Update the Online data
	UpdateSessionPartyOnline();
}

bool ASteamBeaconHost::ProcessKickPlayer(ALobbyBeaconClient* InInstigator, const FUniqueNetIdRepl& PlayerToKick, const FText& Reason)
{
	bool bWasKicked = false;
	if (InInstigator && PlayerToKick.IsValid())
	{
		for (AOnlineBeaconClient* ExistingClient : ClientActors)
		{
			if (ExistingClient != InInstigator)
			{
				ALobbyBeaconClient* LBC = Cast<ALobbyBeaconClient>(ExistingClient);
				if (LBC && LBC->PlayerState && LBC->PlayerState->UniqueId == PlayerToKick)
				{
					// Right now the only eligible lobby kick is a party leader telling the server it kicked a party member
					bool bPartyLeaderKick = (InInstigator->PlayerState->UniqueId == LBC->PlayerState->PartyOwnerUniqueId);
					if (bPartyLeaderKick)
					{
						FText KickReason = NSLOCTEXT("NetworkErrors", "KickedPlayerFromParty", "Kicked from party.");
						KickPlayerFromLobby(LBC, KickReason);
						bWasKicked = true;
					}
					break;
				}
			}
		}
	}
	return bWasKicked;
}

//As the Server, Kick the player from the lobby
void ASteamBeaconHost::KickPlayerFromLobby(ALobbyBeaconClient* ClientActor, const FText& KickReason)
{
	//UE_LOG(LogSteamBeacon, Log, TEXT("KickPlayer for %s. PendingKill %d UNetConnection %s UNetDriver %s State %d"), *GetNameSafe(ClientActor), ClientActor->IsPendingKill(), *GetNameSafe(ClientActor->BeaconConnection), ClientActor->BeaconConnection ? *GetNameSafe(ClientActor->BeaconConnection->Driver) : TEXT("null"), ClientActor->BeaconConnection ? ClientActor->BeaconConnection->State : -1);
	ASteamBeaconClient* pSteamBeaconClient = Cast<ASteamBeaconClient>(ClientActor);
	if (!pSteamBeaconClient) return;

	//Send the notification to the player, they were kicked
	pSteamBeaconClient->ClientWasKickedFromLobby(KickReason);

	//Before Disconnecting we need to change our Connection owner for proper NetCleanup back to SteamBeacon instead of PC
	UNetConnection* pConnection = pSteamBeaconClient->GetNetConnection();
	if (pConnection)
	{
		if (pConnection->PlayerController)
		{
			//Remove our Dummy PlayerController
			pConnection->PlayerController->Destroy();
			pConnection->PlayerController = nullptr;
		}
		//Set Back the original SteamBeaconClient as the owner again
		pConnection->OwningActor = pSteamBeaconClient;
	}
	
	DisconnectClient(pSteamBeaconClient);
}

//Run NetCleanup on any associated SteamBeaconClients to Connection
void ASteamBeaconHost::ProcessBeaconNetCleanup(UNetConnection* Connection)
{
	if (!ClientActors.Num()) return;

	for (int32 i = 0; i < ClientActors.Num(); i++)
	{
		if (ClientActors[i] && (ClientActors[i]->GetNetConnection() == Connection))
		{
			ClientActors[i]->OnNetCleanup(Connection);
		}
	}
}



//Run debugging output for networking troubleshooting
void ASteamBeaconHost::PrintNetworkDebugInfo()
{
	if (!ClientActors.Num()) return;

	for (int32 i = 0; i < ClientActors.Num(); i++)
	{
		if (ClientActors[i] && ClientActors[i]->GetNetConnection() && ClientActors[i]->GetNetConnection()->Driver)
		{
			UE_LOG(LogSteamBeacon, Warning, TEXT("ASteamBeaconHost::PrintNetworkDebugInfo: NetConnection == %s"), *ClientActors[i]->GetNetConnection()->GetName());
			ClientActors[i]->GetNetConnection()->Driver->DebugRelevantActors = true;
		}
	}
}


//Debugging Beacon Functions for Network Actor Replication
static void	DumpBeaconRelevantActors(UWorld* InWorld)
{
	USteamBeaconGameInstance* pGI = Cast<USteamBeaconGameInstance>(InWorld->GetGameInstance());
	if (pGI && pGI->BeaconHostObject)
	{
		pGI->BeaconHostObject->PrintNetworkDebugInfo();
	}
}

FAutoConsoleCommandWithWorld	DumpBeaconRelevantActorsCommand(
	TEXT("beacon.DumpRelevantActors"),
	TEXT("Dumps information on relevant actors during next beacon network update"),
	FConsoleCommandWithWorldDelegate::CreateStatic(DumpBeaconRelevantActors)
);

// Copyright 2013-2017 W3 Studios, LLC. All Rights Reserved.

#include "SteamBeaconGameInstance.h"
#include "SteamParty.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionInterfaceSteam.h"
#include "OnlineSubsystemSteamTypes.h"
#include "IPAddressSteam.h"
#include "SteamBeaconPlayerController.h"
#include "Engine/LocalPlayer.h"
#include "TimerManager.h"
#include "Engine/Texture2D.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/NetConnection.h"

//Derived Game Instance Class that has built in handling for the Steam Beacons System
USteamBeaconGameInstance::USteamBeaconGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SessionInviteAcceptedDelegate(FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &ThisClass::OnSessionUserInviteAccepted))
	, ReadFriendsListCompleteDelegate(FOnReadFriendsListComplete::CreateUObject(this, &ThisClass::OnReadFriendsListComplete))
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		//Register Steam Call backs Manually here
		OnLobbyInviteReceivedCallback.Register(this, &USteamBeaconGameInstance::OnLobbyInviteReceived);
		OnLobbyChatUpdateCallback.Register(this, &USteamBeaconGameInstance::OnLobbyChatUpdate);
	}
}

//Initialize any systems on game instance init, once game starts up
void USteamBeaconGameInstance::Init()
{
	//Register the InviteAccepted Delegate and store its Handle
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		//Add Invite Accepted Delegate Hook
		SessionInviteAcceptedDelegateHandle = SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(SessionInviteAcceptedDelegate);
	}
	else
	{
		UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::Init Failed! no Session Interface!"));
	}
	
	//Get our Friends List Data
	IOnlineFriendsPtr FriendsInterface = Online::GetFriendsInterface();
	if (FriendsInterface.IsValid() && LocalPlayers.Num())
	{
		ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(LocalPlayers[0]);
		if (LocalPlayer)
		{
			//Only the Default List works on Steam
			FriendsInterface->ReadFriendsList(LocalPlayer->GetControllerId(), EFriendsLists::ToString((EFriendsLists::Default)), ReadFriendsListCompleteDelegate);
		}
	}
	else
	{
		UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::Init Failed! no Friends Interface!"));
	}

	Super::Init();
}

void USteamBeaconGameInstance::Shutdown()
{
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface();

	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::Shutdown Failed to get session system!"));
	}
	else
	{
		// Clear all of the delegate handles here
		SessionInterface->ClearOnSessionUserInviteAcceptedDelegate_Handle(SessionInviteAcceptedDelegateHandle);
	}

	Super::Shutdown();
}


//Callback for Friendslist from Steam Request
void USteamBeaconGameInstance::OnReadFriendsListComplete(int32 LocalPlayerNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
	//This would be where to retrieve the information and call delegates for c++ / slate setups..
	//For UMG use the CallbackProxy Nodes, much simplier integrating into UMG menus
	if (bWasSuccessful)
	{
		IOnlineFriendsPtr FriendsInterface = Online::GetFriendsInterface();
		if (FriendsInterface.IsValid())
		{
			TArray< TSharedRef<FOnlineFriend> > FriendsList;
			FriendsInterface->GetFriendsList(LocalPlayerNum, ListName, FriendsList);
		}
	}
}

bool USteamBeaconGameInstance::SendPartySessionInviteToFriend(APlayerController *PlayerController, const FUniqueNetIdRepl &FriendUniqueNetId)
{
	if (!PlayerController)
	{
		UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::SendPartySessionInviteToFriend: SendSessionInviteToFriend Had a bad Player Controller!"));
		return false;
	}

	if (!FriendUniqueNetId.IsValid())
	{
		UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::SendPartySessionInviteToFriend: SendSessionInviteToFriend Had a bad UniqueNetId!"));
		return false;
	}

	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface();

	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::SendPartySessionInviteToFriend: SendSessionInviteToFriend Failed to get session interface!"));
		return false;
	}

	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PlayerController->Player);

	if (!LocalPlayer)
	{
		UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::SendPartySessionInviteToFriend: SendSessionInviteToFriend failed to get LocalPlayer!"));
		return false;
	}

	if (SessionInterface->SendSessionInviteToFriend(LocalPlayer->GetControllerId(), PartySessionName, *FriendUniqueNetId.GetUniqueNetId()))
	{
		return true;
	}

	return false;
}


//Create Party and become Host
bool USteamBeaconGameInstance::HostParty(int32 MaxPlayers)
{
	//Store New Max Count
	MaxPartyCount = MaxPlayers;

	//TODO::Start Setting up the Beacons First, streamline the process
	//InitBeaconHost();
	
	ASteamBeaconGameSession* pSteamBeaconGameSession = GetSteamBeaconGameSession();
	if (pSteamBeaconGameSession)
	{
		// add callback delegate for completion
		if (!OnCreatePresencePartySessionCompleteDelegateHandle.IsValid())
		{
			OnCreatePresencePartySessionCompleteDelegateHandle = pSteamBeaconGameSession->CreatePresencePartySessionCompleteEvent.AddUObject(this, &USteamBeaconGameInstance::OnCreatePresencePartySessionComplete);
		}

		if (pSteamBeaconGameSession->HostPartySession(LocalPlayers[0]->GetPreferredUniqueNetId(), PartySessionName, MaxPartyCount))
		{
			//Success at creating the Party Session
			return true;
		}

		UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::HostParty HostPartySession Failed/Already Exists"));
		
		//short circuit response, if already created
		OnCreatePresencePartySessionComplete(PartySessionName, true);
		return true;
	}

	UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::HostParty No Abatron GameSession Found"));
	return false;
}

//Remove all players from party and shutdown host beacons
void USteamBeaconGameInstance::DisbandParty()
{
	if (BeaconHostObject)
	{
		//Run the Disband on the Host Authority
		BeaconHostObject->DisbandParty();
	}

	ClearPartyBeacons();

	//Kill active party session
	ASteamBeaconGameSession* pSteamBeaconGameSession = GetSteamBeaconGameSession();
	if (pSteamBeaconGameSession)
	{
		pSteamBeaconGameSession->DestroyPartySession(PartySessionName);
	}

	//Clear party info, by resaving after clearpartybeacons
	SavePartyInfo();
}

// Callback which is intended to be called upon session creation 
void USteamBeaconGameInstance::OnCreatePresencePartySessionComplete(FName SessionName, bool bWasSuccessful)
{
	ASteamBeaconGameSession* pSteamBeaconGameSession = GetSteamBeaconGameSession();
	if (pSteamBeaconGameSession)
	{
		pSteamBeaconGameSession->CreatePresencePartySessionCompleteEvent.Remove(OnCreatePresencePartySessionCompleteDelegateHandle);
		OnCreatePresencePartySessionCompleteDelegateHandle.Reset();

		//Create Host Beacon
		if (bWasSuccessful)
		{
			//TODO:: Refactor so the Beacons don't need to wait for Async Callback before setting up, just fill in session data oncallback
			InitBeaconHost();
		}
	}

	if (!bWasSuccessful) return;
	
	//Setup Voice Status for local player, by default when session is created voice chat is turned on.
	ASteamBeaconPlayerController* const pPlayerController = Cast<ASteamBeaconPlayerController>(GetFirstLocalPlayerController());
	if (pPlayerController)
	{
		pPlayerController->ToggleSpeaking(VoiceChatEnabled && !pPlayerController->IsPushToTalk);
	}
}

//Clear Existing Beacons if they exist
void USteamBeaconGameInstance::ClearPartyBeacons()
{
	PartyStatus = EPartyStatus::PartyNone;

	//Clean up any existing Beacon Host, don't use direct pointer
	ASteamBeaconHost* pBeaconHostObject = BeaconHostObject;
	if (pBeaconHostObject)
	{
		BeaconHostObject = nullptr;
		pBeaconHostObject->Destroy();
		
		//Send an update that the state has changed
		if (pBeaconHostObject->GetPartyState())
		{
			//Broadcast locally
			pBeaconHostObject->GetPartyState()->OnLobbyStateInfoUpdated_Implementation();
		}
	}

	//Clean up any existing Beacon Host Listener
	if (BeaconHostListener)
	{
		if (BeaconHostListener->GetNetConnection())
		{
			BeaconHostListener->GetNetConnection()->CleanUp();
		}
		BeaconHostListener->DestroyBeacon();
		BeaconHostListener = nullptr;
	}

	//Clean up any existing Beacon Client, don't use direct pointer
	ASteamBeaconClient* pBeaconClient = BeaconClient;
	if (pBeaconClient)
	{
		BeaconClient = nullptr;
		pBeaconClient->DestroyBeacon();
		
		//Clean up any existing net connection
		if (pBeaconClient->GetNetConnection())
		{
			pBeaconClient->GetNetConnection()->Close();
		}
		
		//Send an update that the state has changed
		if (pBeaconClient->GetPartyState())
		{
			//Broadcast locally
			pBeaconClient->GetPartyState()->OnLobbyStateInfoUpdated_Implementation();
		}
	}
}

//Creates the Hosting Beacon backend and joins the local player as the Party Leader
void USteamBeaconGameInstance::InitBeaconHost()
{
	//Remove any active beacons
	ClearPartyBeacons();

	// Create the hosting connection
	BeaconHostListener = GetWorld()->SpawnActor<ASteamBeaconListener>(ASteamBeaconListener::StaticClass());
	if (BeaconHostListener && BeaconHostListener->InitHost())
	{
		//Register a class to handle traffic of a specific type
		BeaconHostObject = GetWorld()->SpawnActor<ASteamBeaconHost>(ASteamBeaconHost::StaticClass());
		if (BeaconHostObject)
		{
			BeaconHostObject->Init(PartySessionName);
			BeaconHostListener->RegisterHost(BeaconHostObject);
			BeaconHostObject->SetupLobbyState(MaxPartyCount);
			
			//Set the net driver associated with the hostlistener, fixes standalone code checks
			BeaconHostObject->SetNetDriverName(BeaconHostListener->GetNetDriverName());

			//Allow joining
			BeaconHostListener->PauseBeaconRequests(false);
			
			//Add any Server Players to the Lobby and make them Leader
			InitBeaconClientForServerPlayer();
		}
	}
}

//Creates the Client Beacon for the PartyHost
void USteamBeaconGameInstance::InitBeaconClientForServerPlayer()
{
	BeaconClient = GetWorld()->SpawnActor<ASteamBeaconClient>(ASteamBeaconClient::StaticClass());
	if (BeaconClient && BeaconHostObject && BeaconHostListener)
	{
		//Build our session info
		IOnlineSessionPtr SessionInt = Online::GetSessionInterface(GetWorld());
		FNamedOnlineSession* Session = SessionInt.IsValid() ? SessionInt->GetNamedSession(PartySessionName) : NULL;
		if (Session && Session->SessionInfo.IsValid())
		{
			BeaconClient->SetSessionId(Session->SessionInfo->GetSessionId().ToString());
			BeaconClient->SetBeaconOwner(BeaconHostObject);
			
			//Connected Bypass for Server/Host Player
			//BeaconClient->SetNetConnection(BeaconHostListener->GetNetConnection());
			BeaconClient->OnConnected();
			BeaconHostObject->OnClientConnected(BeaconClient, nullptr);
			
			//Register This BeaconState to the Player Controller to recieve Party Events
			ASteamBeaconPlayerController* const pPlayerController = Cast<ASteamBeaconPlayerController>(GetFirstLocalPlayerController());
			if (pPlayerController)
			{
				pPlayerController->RegisterPartyBeacon(BeaconClient);
			}
			else
			{
				UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::InitBeaconClientForServerPlayer: No Player Controller to Register Party Events"));
			}			
			
			//Add our Custom Data to the SteamBeaconPlayerState
			AddCustomPlayerStateData(Cast<ASteamBeaconPlayerState>(BeaconClient->PlayerState));

			//Trigger update
			BeaconClient->GetPartyState()->OnPlayerLobbyStateAdded().Broadcast(BeaconClient->PlayerState);
			
			//Set us to be Party Leader by default, which it should be
			FUniqueNetIdRepl PartyLeader(LocalPlayers[0]->GetPreferredUniqueNetId());
			BeaconHostObject->UpdatePartyLeader(PartyLeader, PartyLeader);

			BeaconHostObject->GetPartyState()->PartyOwnerUniqueId = PartyLeader;
			PartyStatus = EPartyStatus::PartyLeader;

			//Trigger Party Leader Update for Host
			BeaconClient->PlayerState->OnPartyOwnerChanged().Broadcast(PartyLeader);
		}
	}
}

//Creates the Client Beacon backend and connects the local player to the desired session
void USteamBeaconGameInstance::InitBeaconClient(const FOnlineSessionSearchResult& DesiredHost)
{
	//Save off party join session info
	PartySavedInfo.LastJoinPartySession = DesiredHost;

	//Remove any active beacons
	ClearPartyBeacons();
	
	BeaconClient = GetWorld()->SpawnActor<ASteamBeaconClient>(ASteamBeaconClient::StaticClass());
	if (BeaconClient)
	{
		//Add our Custom Data to the SteamBeaconPlayerState, but after we receive the server replicated actor
		BeaconClient->OnPlayerStateEvent().BindUObject(this, &USteamBeaconGameInstance::OnPlayerStateEvent);
		
		//Add Delegate for Failure
		BeaconClient->OnHostConnectionFailure().BindUObject(this, &USteamBeaconGameInstance::OnSteamBeaconHostConnectionFailureEvent);
		BeaconClient->ConnectToLobby(DesiredHost);
	}
}


void USteamBeaconGameInstance::AddCustomPlayerStateData_Implementation(ASteamBeaconPlayerState* InPlayerState)
{
	//Example on how to override this from a child class for game specific data
	/*if (!InPlayerState) return;
	
	UAbatronLocalPlayer* pPlayer = Cast<UAbatronLocalPlayer>(LocalPlayers[0]);
	if (!pPlayer) return;

	UAbatronPersistentUser* pPlayerData = pPlayer->GetPersistentUser();
	if (!pPlayerData || !pPlayerData->IsOnlineSyncComplete())
	{
		UE_LOG(LogOnlineGame, Verbose, TEXT("UAbatronGameInstance::AddCustomPlayerStateData Failed to get UAbatronPersistentUser"));
		return;
	}

	InPlayerState->PlayerProfileLevel = pPlayerData->ProfileLevel;
	*/
}

//Client: Connection Establish4ed
void USteamBeaconGameInstance::OnPlayerStateEvent(ASteamBeaconPlayerState* InPlayerState)
{
	if (!BeaconClient || !InPlayerState) return;

	//We are officially part of the Party
	PartyStatus = EPartyStatus::PartyMember;

	//Register This BeaconState to the Player Controller to recieve Party Events
	ASteamBeaconPlayerController* const pPlayerController = Cast<ASteamBeaconPlayerController>(GetFirstLocalPlayerController());
	if (pPlayerController)
	{
		pPlayerController->RegisterPartyBeacon(BeaconClient);
	}
	else
	{
		UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::OnPlayerStateEvent: No Player Controller to Register Party Events"));
	}

	//Add our Custom Data to the SteamBeaconPlayerState
	AddCustomPlayerStateData(InPlayerState);
}

//Client: Beacon Fails to Connect to Host 
void USteamBeaconGameInstance::OnSteamBeaconHostConnectionFailureEvent()
{
	UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::OnSteamBeaconHostConnectionFailureEvent: Client Beacon Connection Host Failure"));
	
	//Don't use the direct pointer, as it will be cleared in ClearPartyBeacons
	ASteamBeaconClient* pBeaconClient = BeaconClient;
	if (pBeaconClient)
	{
		//Unhook Delegate from Event
		pBeaconClient->OnHostConnectionFailure().Unbind();
	}

	ClearPartyBeacons();
	
	//Inform Player controller, Connection Failled Event
	ASteamBeaconPlayerController* const pPlayerController = Cast<ASteamBeaconPlayerController>(GetFirstLocalPlayerController());
	if (pPlayerController)
	{
		pPlayerController->OnPartyHostConnectionFailure();
	}
	
	//The Invite could of been directly from listen server lobby instead of from the party system
	//TODO:  Save a the Invite Result, Join Directly

	//Clean up any Online Party Sessions
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		IOnlineSessionPtr SessionInt = OnlineSub->GetSessionInterface();
		if (SessionInt.IsValid())
		{
			//Destroy the Session Record
			SessionInt->DestroySession(PartySessionName);
		}
	}
}

//Dump Party Debug in Log file, use (-log -logcmds="logsteambeacon verbose") on command line
void USteamBeaconGameInstance::DumpDebugParty()
{
	//Server
	if (BeaconHostObject)
	{
		BeaconHostObject->DumpPartyState();
		return;
	}
	else if(BeaconClient) //Clients
	{
		ASteamBeaconState* pSBState = Cast<ASteamBeaconState>(BeaconClient->LobbyState);
		if (pSBState)
		{
			pSBState->DumpPartyState();
			return;
		}
	}
	UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::DumpDebugParty No Active Party Information"));
}

//Send the entire party to the requested steam session information -- Blueprint Callable Version
void USteamBeaconGameInstance::JoinPartyToSessionBP(FBlueprintSessionResult& InSearchResult)
{
	//Send the Unwrapped Version
	JoinPartyToSession(InSearchResult.OnlineResult);
}

//Send the entire party to the requested steam session information
bool USteamBeaconGameInstance::JoinPartyToSession(FOnlineSessionSearchResult& InSearchResult)
{
	if (!BeaconHostObject) return false;
	
	//Save Party Info off
	SavePartyInfo();
	
	FSteamBeaconSessionResult SearchWrapper(InSearchResult);
	
	//Send false here since Party Leader is not the GameServerHost, speeds up connect time
	BeaconHostObject->ConnectPartyToGameServer(SearchWrapper, false);

	return true;
}

//Initiates joining a session from the Party Host, Will start a Session Search to find the requested gameserver/lobby
//This is where the clients have received the SessionInfo from the server
bool USteamBeaconGameInstance::JoinSessionFromSteamBeacon(const FSteamBeaconSessionResult& InSearchResult, bool bIsPartyHostCreatingServer)
{
	//Only Party Members Need to do this route, Party Leader is directly connecting itself
	if (BeaconHostObject) return true;

	//Store for reconnection purposes
	IsPartyHostCreatingServer = bIsPartyHostCreatingServer;
	//If PartyHost becomes the gameserver, we need to give a little time for his world to loadup before connecting, 5 seconds default
	if (IsPartyHostCreatingServer)
	{
		//Save off required info for ResumeJoinSessionFromSteamBeacon()
		SavedSearchResultFromSteamBeacon = InSearchResult;

		//Reset any old retries
		CurrentTravelRetries = 0;
		
		//FOR TIME DELAY ONLY: Save Party Info off... TODO, what if connection issue?
		SavePartyInfo();

		//Set a Timer to refresh in 5 seconds due to packet order for the Party Host to create session ect...
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &USteamBeaconGameInstance::ResumeJoinSessionFromSteamBeacon, PartyHostGameTravelDelay, false);

		//Send a Joining Game Notification
		OnPartyJoiningGameEvent().Broadcast();

		//Destroy All Net Connections here to prevent Overlaps and Handler Crashes for late arriving packets
		//::TODO ClearBeacons Should be enough... 
		ClearPartyBeacons();

		return true;
	}
	else
	{
		//Quickly connect up before loosing spots.  There is no guarantee that every party member will make it before the server
		//Becomes full from other searches from other players. Only workaround is a reservation system... something that will be included in a future update!
		
		// Resolve the address/port,  For Future reference on new system
		//FString SteamIPStr, SteamChannelStr;
		//InSearchResult.HostAddr.Split(":", &SteamIPStr, &SteamChannelStr, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
		
		//First we need to find the Online Session from the SessionId Given
		ASteamBeaconGameSession* pSteamBeaconGameSession = GetSteamBeaconGameSession();
		if (pSteamBeaconGameSession)
		{
			if (!OnSearchSessionsFromSteamBeaconCompletedDelegateHandle.IsValid())
			{
				//Save Party Info off... TODO, what if connection issue?
				SavePartyInfo();

				//Send a Joining Game Notification
				OnPartyJoiningGameEvent().Broadcast();

				//Destroy All Net Connections here to prevent Overlaps and Handler Crashes for late arriving packets
				//::TODO ClearBeacons Should be enough... 
				ClearPartyBeacons();

				SessionIdRequested = InSearchResult.SessionId;
				OnSearchSessionsFromSteamBeaconCompletedDelegateHandle = pSteamBeaconGameSession->FindSessionsCompletedEvent.AddUObject(this, &USteamBeaconGameInstance::OnSearchSessionsFromSteamBeaconCompleted);
				pSteamBeaconGameSession->FindSessionFromSessionId(InSearchResult);
				return true;
			}
			else
			{
				UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::JoinSessionFromSteamBeacon OnSearchSessionsFromSteamBeaconCompletedDelegateHandle Already bound, active search in progress"));
			}
		}

	}
		
	return false;
}

//Delayed joins should no longer be needed, deprecated next version
void USteamBeaconGameInstance::ResumeJoinSessionFromSteamBeacon()
{
	//First we need to find the Online Session from the SessionId Given
	ASteamBeaconGameSession* pSteamBeaconGameSession = GetSteamBeaconGameSession();
	if (pSteamBeaconGameSession)
	{
		if (!OnSearchSessionsFromSteamBeaconCompletedDelegateHandle.IsValid())
		{
			//Save Party Info off... TODO, what if connection issue?
			//SavePartyInfo();

			SessionIdRequested = SavedSearchResultFromSteamBeacon.SessionId;
			OnSearchSessionsFromSteamBeaconCompletedDelegateHandle = pSteamBeaconGameSession->FindSessionsCompletedEvent.AddUObject(this, &USteamBeaconGameInstance::OnSearchSessionsFromSteamBeaconCompleted);
			pSteamBeaconGameSession->FindSessionFromSessionId(SavedSearchResultFromSteamBeacon);
			return;
		}
		else
		{
			UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::JoinSessionFromSteamBeacon OnSearchSessionsFromSteamBeaconCompletedDelegateHandle Already bound, active search in progress"));
		}
	}
}

// Callback upon finding sessions from "JoinSessionFromSteamBeacon"
void USteamBeaconGameInstance::OnSearchSessionsFromSteamBeaconCompleted(bool bWasSuccessful)
{
	ASteamBeaconGameSession* pSteamBeaconGameSession = GetSteamBeaconGameSession();
	if (pSteamBeaconGameSession)
	{
		//Unhook Delegate from Event
		pSteamBeaconGameSession->FindSessionsCompletedEvent.Remove(OnSearchSessionsFromSteamBeaconCompletedDelegateHandle);
		OnSearchSessionsFromSteamBeaconCompletedDelegateHandle.Reset();

		//Get the Search Results
		const TArray<FOnlineSessionSearchResult>& SearchResults = pSteamBeaconGameSession->GetSessionSearchResults();

		//Copy the data to a blueprint friend struct for UMG server lists!!
		if (SearchResults.Num())
		{
			//Find Session of matching SessionId
			for (int i = 0; i < SearchResults.Num(); i++)
			{
				//Find the correct LobbyId or GameServerId
				const FOnlineSessionSearchResult& Result = SearchResults[i];
				
				//Look for custom key set
				FString SessionIdResult;
				Result.Session.SessionSettings.Get(FName(TEXT("SESSIONID")), SessionIdResult);
				if (!Result.Session.SessionSettings.bUsesPresence || (SessionIdResult == SessionIdRequested))
				{
					//Setup callback
					if (!OnJoinGameSessionCompletedDelegateHandle.IsValid())
					{
						OnJoinGameSessionCompletedDelegateHandle = pSteamBeaconGameSession->JoinGameSessionCompleteEvent.AddUObject(this, &USteamBeaconGameInstance::OnJoinGameSessionCompleted);
					}

					//Join The Session
					if (pSteamBeaconGameSession->JoinGameSession(LocalPlayers[0]->GetPreferredUniqueNetId(), SearchResults[i]))
					{
						UE_LOG(LogSteamBeacon, Log, TEXT("USteamBeaconGameInstance::OnSearchSessionsFromSteamBeaconCompleted: Search Found Servers, SessionId %s"), *SessionIdRequested);

						//Send event to playercontroller for cleanup
						OnPartyJoiningGameEvent().Broadcast();
						return;
					}
					else
					{
						UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::OnSearchSessionsFromSteamBeaconCompleted: Search Found Servers, SessionId %s Found, but Can't join Game"), *SessionIdRequested);
					}
				}
			}
			UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::OnSearchSessionsFromSteamBeaconCompleted: Search Found Servers, but SessionId could not be found %s"), *SessionIdRequested);
		}
		else
		{
			//Do a few retries if it was a slow party host game session
			if (IsPartyHostCreatingServer && (CurrentTravelRetries < PartyHostGameTravelMaxRetries))
			{
				//Set a Timer to refresh in 5 seconds due to packet order for the Party Host to create session ect...
				FTimerHandle TimerHandle;
				GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &USteamBeaconGameInstance::ResumeJoinSessionFromSteamBeacon, PartyHostGameTravelDelay, false);
				
				CurrentTravelRetries++;
				//No Search Results
				UE_LOG(LogSteamBeacon, Log, TEXT("USteamBeaconGameInstance::OnSearchSessionsFromSteamBeaconCompleted:: Search Found None Matching Sessions... Retrying...., current retry count = %i"), CurrentTravelRetries);
				return;
			}

			
			//No Search Results
			UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::OnSearchSessionsFromSteamBeaconCompleted:: Search Found None Matching Sessions"));
		}
	}

	//Some error happend here try reconnecting to Party
	RestorePartyInfo();

	//Send out Delegate Broadcast, Blueprints can be assigned
	OnJoinGameSessionComplete.Broadcast(false);
}


//Get the Active SteamBeaconGameSession
ASteamBeaconGameSession* USteamBeaconGameInstance::GetSteamBeaconGameSession()
{
	UWorld* const World = GetWorld();
	if (World)
	{
		AGameModeBase* const Game = World->GetAuthGameMode();
		if (Game)
		{
			return Cast<ASteamBeaconGameSession>(Game->GameSession);
		}
	}
	return nullptr;
}


//Called when player join/leaves lobbies
void USteamBeaconGameInstance::OnLobbyChatUpdate(LobbyChatUpdate_t* CallbackData)
{
	EChatMemberStateChange StateChanged = (EChatMemberStateChange)(CallbackData->m_rgfChatMemberStateChange);
	FUniqueNetIdSteam User(CallbackData->m_ulSteamIDUserChanged);
	FUniqueNetIdSteam Instigator(CallbackData->m_ulSteamIDMakingChange);
	
	//UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::OnLobbyChatUpdate:: User: %s Instigator: %s"), *User.ToDebugString(), *Instigator.ToDebugString());
	switch (StateChanged)
	{
	case k_EChatMemberStateChangeEntered:
		UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::OnLobbyChatUpdate:: User: %s Instigator: %s :: User has joined Lobby"), *User.ToDebugString(), *Instigator.ToDebugString());
		break;
	case k_EChatMemberStateChangeLeft:
		UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::OnLobbyChatUpdate:: User: %s Instigator: %s :: User has left Lobby"), *User.ToDebugString(), *Instigator.ToDebugString());
		break;
	case k_EChatMemberStateChangeDisconnected:
		UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::OnLobbyChatUpdate:: User: %s Instigator: %s :: User has disconnected from Lobby"), *User.ToDebugString(), *Instigator.ToDebugString());
		break;
	case k_EChatMemberStateChangeKicked:
		UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::OnLobbyChatUpdate:: User: %s Instigator: %s :: User kicked from Lobby"), *User.ToDebugString(), *Instigator.ToDebugString());
		break;
	case k_EChatMemberStateChangeBanned:
		UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::OnLobbyChatUpdate:: User: %s Instigator: %s :: User kicked and banned from Lobby"), *User.ToDebugString(), *Instigator.ToDebugString());
		break;
	}

	//If we are a party host, update online data
	if (BeaconHostObject)
	{
		BeaconHostObject->UpdateSessionPartyOnline();
	}

}

//Event triggered by Steam backend when a user receives an invite request (via Steam client)
//@param CallbackData All the valid data from Steam related to this event
void USteamBeaconGameInstance::OnLobbyInviteReceived(LobbyInvite_t* CallbackData)
{
	UE_LOG(LogSteamBeacon, Warning, TEXT(" USteamBeaconGameInstance::OnLobbyInviteReceived:: Invite Received from Steam Friend"));
	
	//Give player In game Option to join
	ASteamBeaconPlayerController* const pPlayerController = Cast<ASteamBeaconPlayerController>(GetFirstLocalPlayerController());
	if (pPlayerController)
	{
		FUniqueNetIdSteam InLobbyId(CallbackData->m_ulSteamIDLobby);
		FUniqueNetIdSteam InPlayerId(CallbackData->m_ulSteamIDUser);

		//Store the Lobby Info so we can join it
		LobbyIdInvite = InLobbyId;
		FriendIdInvite = InPlayerId;

		//Get the Steam Friend info
		IOnlineFriendsPtr FriendsInterface = Online::GetFriendsInterface();
		if (FriendsInterface.IsValid())
		{
			//Request from Steam the friend info, no splitscreen support, only default list supported for steam
			TSharedPtr<FOnlineFriend> FriendInfo = FriendsInterface->GetFriend(0, InPlayerId, EFriendsLists::ToString(EFriendsLists::Default));
			if (FriendInfo.IsValid())
			{
				pPlayerController->OnPartyInviteReceived(FriendInfo->GetDisplayName());
			}
		}
	}
}

//Accept last party invite received
void USteamBeaconGameInstance::AcceptPartyInvite()
{
	//SteamMatchmaking()->JoinLobby(LobbyIdInvite);
	
	ASteamBeaconGameSession* pSteamBeaconGameSession = GetSteamBeaconGameSession();
	if (pSteamBeaconGameSession)
	{
		pSteamBeaconGameSession->FindFriendSession(FriendIdInvite);
	}
}

void USteamBeaconGameInstance::OnSessionUserInviteAccepted(const bool bWasSuccess, const int32 ControllerId, TSharedPtr<const FUniqueNetId > UserId, const FOnlineSessionSearchResult & InviteResult)
{
	UE_LOG(LogSteamBeacon, Verbose, TEXT("USteamBeaconGameInstance::OnSessionUserInviteAccepted: bSuccess: %d, ControllerId: %d, User: %s"), bWasSuccess, ControllerId, UserId.IsValid() ? *UserId->ToString() : TEXT("NULL "));
	if (!bWasSuccess)
	{
		return;
	}

	if (!InviteResult.IsValid())
	{
		UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::OnSessionUserInviteAccepted: Invalid Search Result"));
		return;
	}

	if (!UserId.IsValid())
	{
		UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::OnSessionUserInviteAccepted: Invalid User"));
		return;
	}

	//Make sure we aren't already in the lobby we got accepted the invite for, double invites can come from ingame and steamclient
	if (PartyStatus == EPartyStatus::PartyMember)
	{
		//Check the party ids
		if (InviteResult.GetSessionIdStr() == PartySavedInfo.LastJoinPartySession.GetSessionIdStr())
		{
			UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::OnSessionUserInviteAccepted: Already in Party, ignoring Invite"));
			return;
		}
	}
	
	//TODO:  IF Invite was from a non party session and from game session listen server instead.
	//JoinSessionFromResult(InviteResult);
	
	//Start the ClientBeacon
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		IOnlineSessionPtr SessionInt = OnlineSub->GetSessionInterface();
		if (SessionInt.IsValid())
		{
			InitBeaconClient(InviteResult);
			
			//Add to Party Session Here
			SessionInt->JoinSession(0, PartySessionName, InviteResult);
		}
	}
}

//Get Texture of Steam Avatar, Huge Help From the Forums!
UTexture2D* USteamBeaconGameInstance::GetSteamFriendAvatar(const FUniqueNetIdRepl& FriendUniqueNetId, ESteamAvatarSize InAvatarSize)
{
	if (!FriendUniqueNetId.IsValid() || !FriendUniqueNetId.GetUniqueNetId()->IsValid())
	{
		UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::GetSteamFriendAvatar: Invalid FriendUniqueNetId!"));
		return nullptr;
	}

	uint32 Width = 0;
	uint32 Height = 0;

	if (SteamAPI_Init())
	{
		//Getting the PictureID from the SteamAPI and getting the Size with the ID
		uint64 id = *((uint64*)FriendUniqueNetId.GetUniqueNetId()->GetBytes());
		int Picture = 0;

		switch (InAvatarSize)
		{
			case ESteamAvatarSize::AvatarSmall: Picture = SteamFriends()->GetSmallFriendAvatar(id); break;
			case ESteamAvatarSize::AvatarMedium: Picture = SteamFriends()->GetMediumFriendAvatar(id); break;
			case ESteamAvatarSize::AvatarLarge: Picture = SteamFriends()->GetLargeFriendAvatar(id); break;
			default: break;
		}

		if (Picture == -1)
			return NULL;

		SteamUtils()->GetImageSize(Picture, &Width, &Height);
		if (Width > 0 && Height > 0)
		{
			//Creating the buffer "oAvatarRGBA" and then filling it with the RGBA Stream from the Steam Avatar
			uint8 *oAvatarRGBA = new uint8[Width * Height * 4];

			//Filling the buffer with the RGBA Stream from the Steam Avatar and creating a UTextur2D to parse the RGBA Steam in
			SteamUtils()->GetImageRGBA(Picture, (uint8*)oAvatarRGBA, 4 * Height * Width * sizeof(char));
			UTexture2D* Avatar = UTexture2D::CreateTransient(Width, Height, PF_R8G8B8A8);

			// Switched to a Memcpy instead of byte by byte transer
			uint8* MipData = (uint8*)Avatar->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(MipData, (void*)oAvatarRGBA, Height * Width * 4);
			Avatar->PlatformData->Mips[0].BulkData.Unlock();
			delete[] oAvatarRGBA;

			//Setting some Parameters for the Texture and finally returning it
			Avatar->PlatformData->NumSlices = 1;
			Avatar->NeverStream = true;
			Avatar->UpdateResource();

			return Avatar;
		}
		return nullptr;
	}
	
	UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::GetSteamFriendAvatar: Steam not Initialized"));
	return nullptr;
}


//Save Party Info
void USteamBeaconGameInstance::SavePartyInfo()
{
	switch (PartyStatus)
	{
		case EPartyStatus::PartyLeader:
			PartySavedInfo.IsInParty = true;
			PartySavedInfo.IsPartyLeader = true;
			break;
		case EPartyStatus::PartyMember:
			PartySavedInfo.IsInParty = true;
			PartySavedInfo.IsPartyLeader = false;
			break;
		case EPartyStatus::PartyNone:
		default:
			PartySavedInfo.IsInParty = false;
			PartySavedInfo.IsPartyLeader = false;
			break;
	}

	if (BeaconClient)
	{
		//Unbind ConnectionHost Failure, as we are tranisistion to map or shutting down
		BeaconClient->OnHostConnectionFailure().Unbind();	//.BindUObject(this, &USteamBeaconGameInstance::OnSteamBeaconHostConnectionFailureEvent);
	}
}

//Restore Party Info & Beacons
void USteamBeaconGameInstance::RestorePartyInfo()
{
	//Remove any Active Game Session, Assuming that we don't ever have a restoreparty inside of an active game session, only from main menu... could be changed
	DestroyGameSession();
	
	if (!PartySavedInfo.IsInParty)
	{
		UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::RestorePartyInfo: Not In an Active Party!!"));
		return;
	}

	//Are we host?
	if (PartySavedInfo.IsPartyLeader)
	{
		//Party Session should still be setup here
		InitBeaconHost();
		UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::RestorePartyInfo: Initializing Host....."));
	}
	else
	{
		//We are client
		InitBeaconClient(PartySavedInfo.LastJoinPartySession);
		UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::RestorePartyInfo: Initializing Client....."));

		//Inform Player controller, Reconnecting to Host... 90 second timeout default
		ASteamBeaconPlayerController* const pPlayerController = Cast<ASteamBeaconPlayerController>(GetFirstLocalPlayerController());
		if (pPlayerController)
		{
			pPlayerController->OnPartyHostReconnecting();
		}
	}
}

//Return number of players currently in party
int32 USteamBeaconGameInstance::GetPartyCount()
{
	if (BeaconClient && BeaconClient->GetPartyState())
	{
		return BeaconClient->GetPartyState()->GetNumPlayers();
	}
	
	//Not part of a party, just yourself
	return 1;
}

//Blueprint Accessor to disconnect steam sessions
void USteamBeaconGameInstance::RemoveSteamPlayerSession(APlayerState* InPlayerState)
{
	RemoveSteamP2PSession(InPlayerState);
}

//Directly remove and unregister SteamP2PSessions
void USteamBeaconGameInstance::RemoveSteamP2PSession(CSteamID* SteamPlayer)
{
	QueueRemoveSteamP2PSessions.Add(*SteamPlayer);

	//Create a timer callback in 1 second to let the netconnections finish cleaning up
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &USteamBeaconGameInstance::ProcessRemoveSteamP2PSession, 1.f, false);
}

//Directly remove and unregister SteamP2PSessions
void USteamBeaconGameInstance::RemoveSteamP2PSession(APlayerState* InPlayerState)
{
	if (!InPlayerState) return;

	CSteamID SteamPlayerId(*(uint64*)InPlayerState->UniqueId->GetBytes());
	QueueRemoveSteamP2PSessions.Add(SteamPlayerId);

	//Create a timer callback in 1 second to let the netconnections finish cleaning up
	//FTimerHandle TimerHandle;
	//GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &USteamBeaconGameInstance::ProcessRemoveSteamP2PSession, 1.f, false);
	ProcessRemoveSteamP2PSession();
}

void USteamBeaconGameInstance::ProcessRemoveSteamP2PSession()
{
	if (!QueueRemoveSteamP2PSessions.Num()) return;

	if (QueueRemoveSteamP2PSessions[0].IsValid() && SteamNetworking()->CloseP2PSessionWithUser(QueueRemoveSteamP2PSessions[0]))
	{
		UE_LOG(LogSteamBeacon, Verbose, TEXT("USteamBeaconGameInstance::ProcessRemoveSteamP2PSession: CloseP2PSessionWithUser Success"));
	}
	else
	{
		UE_LOG(LogSteamBeacon, Verbose, TEXT("USteamBeaconGameInstance::ProcessRemoveSteamP2PSession: CloseP2PSessionWithUser Failed!"));
	}

	QueueRemoveSteamP2PSessions.RemoveAt(0);
}




//Create Game and bring in Party
bool USteamBeaconGameInstance::HostGame(int32 MaxPlayers)
{
	//Store New Max Count
	//MaxPartyCount = MaxPlayers;

	ASteamBeaconGameSession* pSteamBeaconGameSession = GetSteamBeaconGameSession();
	if (pSteamBeaconGameSession)
	{
		// add callback delegate for completion
		if (!OnCreatePresenceGameSessionCompleteDelegateHandle.IsValid())
		{
			OnCreatePresenceGameSessionCompleteDelegateHandle = pSteamBeaconGameSession->CreatePresenceGameSessionCompleteEvent.AddUObject(this, &USteamBeaconGameInstance::OnCreatePresenceGameSessionCompleted);
		}

		if (pSteamBeaconGameSession->HostGameSession(LocalPlayers[0]->GetPreferredUniqueNetId(), MaxPlayers))
		{
			//Success at creating the Game Session
			return true;
		}

		UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::HostGame HostGameSession Failed/Already Exists"));

		//short circuit response, if already created
		OnCreatePresenceGameSessionCompleted(GameSessionName, true);
		return true;
	}

	UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::HostGame No GameSession Found"));
	return false;
}

// Callback which is intended to be called upon session creation 
void USteamBeaconGameInstance::OnCreatePresenceGameSessionCompleted(FName SessionName, bool bWasSuccessful)
{
	ASteamBeaconGameSession* pSteamBeaconGameSession = GetSteamBeaconGameSession();
	if (pSteamBeaconGameSession)
	{
		pSteamBeaconGameSession->CreatePresenceGameSessionCompleteEvent.Remove(OnCreatePresenceGameSessionCompleteDelegateHandle);
		OnCreatePresenceGameSessionCompleteDelegateHandle.Reset();

		//Join the party to the game session
		if (bWasSuccessful)
		{
			JoinPartyToHostGameSession();
		}
	}

	//Send out Delegate Broadcast, Blueprints can be assigned
	OnCreateGameSessionCompleted.Broadcast(bWasSuccessful);
}

//Get Current GameSession of Host
FString USteamBeaconGameInstance::GetHostCurrentGameSessionId()
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		IOnlineSessionPtr SessionInt = OnlineSub->GetSessionInterface();
		if (SessionInt.IsValid())
		{
			FNamedOnlineSession* Session = SessionInt.IsValid() ? SessionInt->GetNamedSession(GameSessionName) : NULL;
			if (Session && Session->SessionInfo.IsValid())
			{
				return Session->SessionInfo->GetSessionId().ToString();
			}
		}
	}

	return "";
}

//Send the entire party to the requested steam session information
bool USteamBeaconGameInstance::JoinPartyToHostGameSession()
{
	if (BeaconHostObject && LocalPlayers.Num())// && (GetHostCurrentGameSessionId() != ""))
	{
		//Save Party Info off
		SavePartyInfo();

		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub && GetFirstGamePlayer())
		{
			FSteamBeaconSessionResult SearchWrapper(GetFirstGamePlayer()->GetPreferredUniqueNetId()->ToString());
			FString ServerURL;
			IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
			if (Sessions.IsValid() && Sessions->GetResolvedConnectString(GameSessionName, ServerURL))
			{
				//BeaconHostObject->ConnectPartyToGameServer(ServerURL);
				BeaconHostObject->ConnectPartyToGameServer(SearchWrapper, true);
				return true;
			}
		}
	}
	return false;
}

// Callback which is intended to be called upon session joined 
void USteamBeaconGameInstance::OnJoinGameSessionCompleted(EOnJoinSessionCompleteResult::Type Result)
{
	ASteamBeaconGameSession* pSteamBeaconGameSession = GetSteamBeaconGameSession();
	if (pSteamBeaconGameSession)
	{
		pSteamBeaconGameSession->JoinGameSessionCompleteEvent.Remove(OnJoinGameSessionCompletedDelegateHandle);
		OnJoinGameSessionCompletedDelegateHandle.Reset();
		
		//Join the party to the game session
		if (Result == EOnJoinSessionCompleteResult::Success)
		{
			TravelToGameSession();
		}
		else
		{
			UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::OnJoinGameSessionCompleted JoinSession Failed! Error == %s"), *LexToString(Result));
			//Note: May want to send notification back so Menus can reset states
			//TODO: Also may want to try RestoreParty() on failure here, edge cases where half the party makes it to the new server, but some don't because it got full from external players joining...
		}
	}

	//Send out Delegate Broadcast, Blueprints can be assigned
	OnJoinGameSessionComplete.Broadcast(Result == EOnJoinSessionCompleteResult::Success);
}

//Take player to game server from session data
void USteamBeaconGameInstance::TravelToGameSession()
{
	APlayerController * const PC = GetFirstLocalPlayerController();
	if (PC)
	{
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			FString ServerURL;
			IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
			if (Sessions.IsValid() && Sessions->GetResolvedConnectString(GameSessionName, ServerURL))
			{
				PC->ClientTravel(ServerURL, TRAVEL_Absolute);
				UE_LOG(LogSteamBeacon, Verbose, TEXT("USteamBeaconGameInstance::TravelToGameSession: Travel to Session Success %s"), *ServerURL);
				return;
			}
		}
	}
		
	UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::TravelToGameSession: Travel to Session Failed"));
}

//Take player to game server from session data
void USteamBeaconGameInstance::DirectTravelToGameSession(FString ServerURL)
{
	APlayerController * const PC = GetFirstLocalPlayerController();
	if (PC)
	{
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			//FString ServerURL;
			IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
			if (Sessions.IsValid()) // && Sessions->GetResolvedConnectString(GameSessionName, ServerURL))
			{
				//Save Party off Info
				SavePartyInfo();
				
				PC->ClientTravel(ServerURL, TRAVEL_Absolute);
				UE_LOG(LogSteamBeacon, Verbose, TEXT("USteamBeaconGameInstance::TravelToGameSession: Travel to Session Success %s"), *ServerURL);
				return;
			}
		}
	}

	UE_LOG(LogSteamBeacon, Warning, TEXT("USteamBeaconGameInstance::TravelToGameSession: Travel to Session Failed %s"), *ServerURL);
}

//Destroy the game session
void USteamBeaconGameInstance::DestroyGameSession()
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			EOnlineSessionState::Type SessionState = Sessions->GetSessionState(GameSessionName);
			if (EOnlineSessionState::NoSession != SessionState)
			{
				Sessions->DestroySession(GameSessionName);
			}
		}
	}
}
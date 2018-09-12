// Copyright 2013-2017 W3 Studios, LLC. All Rights Reserved.

#include "CreateSessionGameCallbackProxy.h"
#include "EngineGlobals.h"
#include "Engine/Engine.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "OnlineSubsystemUtils.h"
#include "SteamBeaconGameInstance.h"
#include "TimerManager.h"

//////////////////////////////////////////////////////////////////////////
// UCreateSessionGameCallbackProxy

UCreateSessionGameCallbackProxy::UCreateSessionGameCallbackProxy(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CreateCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateCompleted))
	, StartCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartCompleted))
	, NumPublicConnections(1)
{
}

UCreateSessionGameCallbackProxy* UCreateSessionGameCallbackProxy::CreateSessionGame(UObject* WorldContextObject, class APlayerController* PlayerController, int32 PublicConnections, bool bUseLAN, const FString& InGameType)
{
	UCreateSessionGameCallbackProxy* Proxy = NewObject<UCreateSessionGameCallbackProxy>();
	Proxy->PlayerControllerWeakPtr = PlayerController;
	Proxy->NumPublicConnections = PublicConnections;
	Proxy->bUseLAN = bUseLAN;
	Proxy->WorldContextObject = WorldContextObject;
	Proxy->GameType = InGameType;
	return Proxy;
}

void UCreateSessionGameCallbackProxy::Activate()
{
	if (PlayerControllerWeakPtr.IsValid() && PlayerControllerWeakPtr.Get()->PlayerState && PlayerControllerWeakPtr.Get()->PlayerState->UniqueId.GetUniqueNetId().IsValid())
	{
		auto Sessions = Online::GetSessionInterface();
		if (Sessions.IsValid())
		{
			CreateCompleteDelegateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(CreateCompleteDelegate);

			FOnlineSessionSettings Settings;
			Settings.NumPublicConnections = NumPublicConnections;
			Settings.bShouldAdvertise = true;
			Settings.bAllowJoinInProgress = true;
			Settings.bIsLANMatch = bUseLAN;
			Settings.bUsesPresence = true;
			Settings.bAllowJoinViaPresence = true;

			//Add our custom search key since the default engine steam implementation is broken between the query parameter and created names
			Settings.Set(FName(TEXT("SESSIONID")), PlayerControllerWeakPtr.Get()->PlayerState->UniqueId.GetUniqueNetId()->ToString(), EOnlineDataAdvertisementType::ViaOnlineService);

			//Add Server Tags, This is only for Listen Servers, Dedicated servers use a different API and tags are too long, requires engine modification of the SessionKeys to shorter names
			Settings.Set(SETTING_GAMEMODE, GameType, EOnlineDataAdvertisementType::ViaOnlineService);

			Sessions->CreateSession(*PlayerControllerWeakPtr.Get()->PlayerState->UniqueId.GetUniqueNetId(), GameSessionName, Settings);

			// OnCreateCompleted will get called, nothing more to do now
			return;
		}
		else
		{
			FFrame::KismetExecutionMessage(TEXT("Sessions not supported by Online Subsystem"), ELogVerbosity::Warning);
		}
	}

	// Fail immediately
	OnFailure.Broadcast();
}

void UCreateSessionGameCallbackProxy::OnCreateCompleted(FName SessionName, bool bWasSuccessful)
{
	if (PlayerControllerWeakPtr.IsValid() && PlayerControllerWeakPtr.Get()->PlayerState && PlayerControllerWeakPtr.Get()->PlayerState->UniqueId.GetUniqueNetId().IsValid())
	{
		auto Sessions = Online::GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->ClearOnCreateSessionCompleteDelegate_Handle(CreateCompleteDelegateHandle);
			
			if (bWasSuccessful)
			{
				//Here we need to send our party if we have one
				//Check if you are a Party Leader
				UWorld* CurrentWorld = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
				if (CurrentWorld)
				{
					USteamBeaconGameInstance* GI = Cast<USteamBeaconGameInstance>(CurrentWorld->GetGameInstance());
					if (GI && GI->BeaconHostObject)
					{
						//We call this function specifically when Party Leader Host becomes GameServer
						GI->JoinPartyToHostGameSession();

						//Set a Timer to resume, to allow enough time to send the data down to the beacons before hard disconnecting for level changes
						//FTimerHandle TimerHandle;
						//CurrentWorld->GetTimerManager().SetTimer(TimerHandle, this, &UCreateSessionGameCallbackProxy::ResumeOnCreateCompleted, 1.f, false);
						//return;
					}
				}
				
				StartCompleteDelegateHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(StartCompleteDelegate);
				Sessions->StartSession(GameSessionName);

				// OnStartCompleted will get called, nothing more to do now
				return;
			}
		}
	}

	if (!bWasSuccessful)
	{
		OnFailure.Broadcast();
	}
}

void UCreateSessionGameCallbackProxy::OnStartCompleted(FName SessionName, bool bWasSuccessful)
{
	if (PlayerControllerWeakPtr.IsValid() && PlayerControllerWeakPtr.Get()->PlayerState && PlayerControllerWeakPtr.Get()->PlayerState->UniqueId.GetUniqueNetId().IsValid())
	{
		auto Sessions = Online::GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->ClearOnStartSessionCompleteDelegate_Handle(StartCompleteDelegateHandle);
		}
	}

	if (bWasSuccessful)
	{
		OnSuccess.Broadcast();
	}
	else
	{
		OnFailure.Broadcast();
	}
}


// Resume with Starting the Session now
void UCreateSessionGameCallbackProxy::ResumeOnCreateCompleted()
{
	auto Sessions = Online::GetSessionInterface();
	if (Sessions.IsValid())
	{
		StartCompleteDelegateHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(StartCompleteDelegate);
		Sessions->StartSession(GameSessionName);
	}	
}
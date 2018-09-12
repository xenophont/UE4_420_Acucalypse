// Copyright 2013-2017 W3 Studios, LLC. All Rights Reserved.

#include "JoinSessionGameCallbackProxy.h"
#include "EngineGlobals.h"
#include "Engine/Engine.h"
#include "OnlineSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "SteamBeaconGameInstance.h"


//////////////////////////////////////////////////////////////////////////
// UJoinSessionGameCallbackProxy

UJoinSessionGameCallbackProxy::UJoinSessionGameCallbackProxy(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Delegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCompleted))
{
}

UJoinSessionGameCallbackProxy* UJoinSessionGameCallbackProxy::JoinSessionGame(UObject* WorldContextObject, class APlayerController* PlayerController, const FBlueprintSessionResult& SearchResult)
{
	UJoinSessionGameCallbackProxy* Proxy = NewObject<UJoinSessionGameCallbackProxy>();
	Proxy->PlayerControllerWeakPtr = PlayerController;
	Proxy->OnlineSearchResult = SearchResult.OnlineResult;
	Proxy->WorldContextObject = WorldContextObject;
	return Proxy;
}

void UJoinSessionGameCallbackProxy::Activate()
{
	//Check if you are a Party Leader
	UWorld* CurrentWorld = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (CurrentWorld)
	{
		USteamBeaconGameInstance* GI = Cast<USteamBeaconGameInstance>(CurrentWorld->GetGameInstance());
		if (GI && GI->BeaconHostObject)
		{
			GI->JoinPartyToSession(OnlineSearchResult);
		}
	}
	
	if (PlayerControllerWeakPtr.IsValid() && PlayerControllerWeakPtr.Get()->PlayerState && PlayerControllerWeakPtr.Get()->PlayerState->UniqueId.GetUniqueNetId().IsValid())
	{
		auto Sessions = Online::GetSessionInterface();
		if (Sessions.IsValid())
		{
			DelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(Delegate);
			Sessions->JoinSession(*PlayerControllerWeakPtr.Get()->PlayerState->UniqueId.GetUniqueNetId(), GameSessionName, OnlineSearchResult);

			// OnCompleted will get called, nothing more to do now
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

void UJoinSessionGameCallbackProxy::OnCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (PlayerControllerWeakPtr.IsValid() && PlayerControllerWeakPtr.Get()->PlayerState && PlayerControllerWeakPtr.Get()->PlayerState->UniqueId.GetUniqueNetId().IsValid())
	{
		auto Sessions = Online::GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->ClearOnJoinSessionCompleteDelegate_Handle(DelegateHandle);

			if (Result == EOnJoinSessionCompleteResult::Success)
			{
				// Client travel to the server
				FString ConnectString;
				if (Sessions->GetResolvedConnectString(GameSessionName, ConnectString) && PlayerControllerWeakPtr.IsValid())
				{
					UE_LOG(LogOnline, Log, TEXT("Join Session Game: traveling to %s"), *ConnectString);
					PlayerControllerWeakPtr->ClientTravel(ConnectString, TRAVEL_Absolute);
					OnSuccess.Broadcast();
					return;
				}
			}
		}
	}

	OnFailure.Broadcast();
}

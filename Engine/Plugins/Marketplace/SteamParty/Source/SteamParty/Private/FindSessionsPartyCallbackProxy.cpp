// Copyright 2013-2017 W3 Studios, LLC. All Rights Reserved.

#include "FindSessionsPartyCallbackProxy.h"
#include "EngineGlobals.h"
#include "Engine/Engine.h"
#include "OnlineSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

//////////////////////////////////////////////////////////////////////////
// UFindSessionsPartyCallbackProxy

UFindSessionsPartyCallbackProxy::UFindSessionsPartyCallbackProxy(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Delegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnCompleted))
	, bUseLAN(false)
	, GameType(FString(TEXT("ALL")))
{
}

UFindSessionsPartyCallbackProxy* UFindSessionsPartyCallbackProxy::FindSessionsParty(UObject* WorldContextObject, class APlayerController* PlayerController, int MaxResults, bool bUseLAN, const FString& InGameType)
{
	UFindSessionsPartyCallbackProxy* Proxy = NewObject<UFindSessionsPartyCallbackProxy>();
	Proxy->PlayerControllerWeakPtr = PlayerController;
	Proxy->bUseLAN = bUseLAN;
	Proxy->MaxResults = MaxResults;
	Proxy->WorldContextObject = WorldContextObject;
	Proxy->GameType = InGameType;
	return Proxy;
}

void UFindSessionsPartyCallbackProxy::Activate()
{
	if (PlayerControllerWeakPtr.IsValid() && PlayerControllerWeakPtr.Get()->PlayerState && PlayerControllerWeakPtr.Get()->PlayerState->UniqueId.GetUniqueNetId().IsValid())
	{
		auto Sessions = Online::GetSessionInterface();
		if (Sessions.IsValid())
		{
			DelegateHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(Delegate);
			
			SearchObject = MakeShareable(new FOnlineSessionSearch);
			SearchObject->MaxSearchResults = MaxResults;
			SearchObject->bIsLanQuery = bUseLAN;
			SearchObject->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
			
			//If specified specific gamemode, add the filter
			if (GameType == FString(TEXT("ALL")))
			{
				//Exclude Party Sessions! important as it will hang on the search callback for 15 seconds and timeout
				SearchObject->QuerySettings.Set(SETTING_GAMEMODE, FString(TEXT("PARTY")), EOnlineComparisonOp::Equals);
			}
			else
			{
				SearchObject->QuerySettings.Set(SETTING_GAMEMODE, GameType, EOnlineComparisonOp::Equals);
			}

			Sessions->FindSessions(*PlayerControllerWeakPtr.Get()->PlayerState->UniqueId.GetUniqueNetId(), SearchObject.ToSharedRef());

			// OnQueryCompleted will get called, nothing more to do now
			return;
		}
		else
		{
			FFrame::KismetExecutionMessage(TEXT("Sessions not supported by Online Subsystem"), ELogVerbosity::Warning);
		}
	}

	// Fail immediately
	TArray<FBlueprintSessionResult> Results;
	OnFailure.Broadcast(Results);
}

void UFindSessionsPartyCallbackProxy::OnCompleted(bool bSuccess)
{
	if (PlayerControllerWeakPtr.IsValid() && PlayerControllerWeakPtr.Get()->PlayerState && PlayerControllerWeakPtr.Get()->PlayerState->UniqueId.GetUniqueNetId().IsValid())
	{
		auto Sessions = Online::GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->ClearOnFindSessionsCompleteDelegate_Handle(DelegateHandle);
		}
	}

	TArray<FBlueprintSessionResult> Results;

	if (bSuccess && SearchObject.IsValid())
	{
		for (auto& Result : SearchObject->SearchResults)
		{
			FBlueprintSessionResult BPResult;
			BPResult.OnlineResult = Result;
			Results.Add(BPResult);
		}

		OnSuccess.Broadcast(Results);
	}
	else
	{
		OnFailure.Broadcast(Results);
	}
}

// Get the Game Mode / Type
FString UFindSessionsPartyCallbackProxy::GetGameType(const FBlueprintSessionResult& Result)
{
	FString GameTypeResult;
	Result.OnlineResult.Session.SessionSettings.Get(SETTING_GAMEMODE, GameTypeResult);

	return GameTypeResult;
}



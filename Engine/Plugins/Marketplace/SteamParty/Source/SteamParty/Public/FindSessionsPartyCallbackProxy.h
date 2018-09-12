// Copyright 2013-2017 W3 Studios, LLC. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "OnlineSessionSettings.h"
#include "Net/OnlineBlueprintCallProxyBase.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "FindSessionsCallbackProxy.h"
#include "FindSessionsPartyCallbackProxy.generated.h"

class APlayerController;

UCLASS(MinimalAPI, meta = (ShortTooltip = "Find PartySessions for SteamParty"))
class UFindSessionsPartyCallbackProxy : public UOnlineBlueprintCallProxyBase
{
	GENERATED_UCLASS_BODY()

	// Called when there is a successful query
	UPROPERTY(BlueprintAssignable)
	FBlueprintFindSessionsResultDelegate OnSuccess;

	// Called when there is an unsuccessful query
	UPROPERTY(BlueprintAssignable)
	FBlueprintFindSessionsResultDelegate OnFailure;

	// Searches for advertised sessions with the default online subsystem
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly = "true", WorldContext="WorldContextObject"), Category = "SteamParty|Session")
	static UFindSessionsPartyCallbackProxy* FindSessionsParty(UObject* WorldContextObject, class APlayerController* PlayerController, int32 MaxResults, bool bUseLAN, const FString& InGameType = FString(TEXT("ALL")));

	UFUNCTION(BlueprintPure, Category = "SteamParty|Session")
	static FString GetGameType(const FBlueprintSessionResult& Result);

	// UOnlineBlueprintCallProxyBase interface
	virtual void Activate() override;
	// End of UOnlineBlueprintCallProxyBase interface

private:
	// Internal callback when the session search completes, calls out to the public success/failure callbacks
	void OnCompleted(bool bSuccess);

private:
	// The player controller triggering things
	TWeakObjectPtr<APlayerController> PlayerControllerWeakPtr;

	// The delegate executed by the online subsystem
	FOnFindSessionsCompleteDelegate Delegate;

	// Handle to the registered OnFindSessionsComplete delegate
	FDelegateHandle DelegateHandle;

	// Object to track search results
	TSharedPtr<FOnlineSessionSearch> SearchObject;

	// Whether or not to search LAN
	bool bUseLAN;

	// Maximum number of results to return
	int MaxResults;

	// Game Mode Type to Search for
	FString GameType;

	// The world context object in which this call is taking place
	UObject* WorldContextObject;
};

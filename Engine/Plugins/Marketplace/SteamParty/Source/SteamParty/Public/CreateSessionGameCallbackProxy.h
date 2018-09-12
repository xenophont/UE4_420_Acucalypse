// Copyright 2013-2017 W3 Studios, LLC. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Net/OnlineBlueprintCallProxyBase.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "CreateSessionGameCallbackProxy.generated.h"

class APlayerController;

UCLASS(MinimalAPI, meta = (ShortTooltip = "Create a New GameSession for SteamParty"))
class UCreateSessionGameCallbackProxy : public UOnlineBlueprintCallProxyBase
{
	GENERATED_UCLASS_BODY()

	// Called when the session was created successfully
	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnSuccess;

	// Called when there was an error creating the session
	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnFailure;

	// Creates a session with the default online subsystem
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly = "true", WorldContext="WorldContextObject"), Category = "SteamParty|Session")
	static UCreateSessionGameCallbackProxy* CreateSessionGame(UObject* WorldContextObject, class APlayerController* PlayerController, int32 PublicConnections, bool bUseLAN, const FString& InGameType = FString(TEXT("DEATHMATCH")));

	// UOnlineBlueprintCallProxyBase interface
	virtual void Activate() override;
	// End of UOnlineBlueprintCallProxyBase interface

private:
	// Internal callback when session creation completes, calls StartSession
	void OnCreateCompleted(FName SessionName, bool bWasSuccessful);

	// Delayed callback for ParyHost creating session to allow enough time to transmit info to beacons
	void ResumeOnCreateCompleted();
	
	// Internal callback when session creation completes, calls StartSession
	void OnStartCompleted(FName SessionName, bool bWasSuccessful);

	// The player controller triggering things
	TWeakObjectPtr<APlayerController> PlayerControllerWeakPtr;

	// The delegate executed by the online subsystem
	FOnCreateSessionCompleteDelegate CreateCompleteDelegate;

	// The delegate executed by the online subsystem
	FOnStartSessionCompleteDelegate StartCompleteDelegate;

	// Handles to the registered delegates above
	FDelegateHandle CreateCompleteDelegateHandle;
	FDelegateHandle StartCompleteDelegateHandle;

	// Number of public connections
	int NumPublicConnections;

	// Whether or not to search LAN
	bool bUseLAN;

	// Game Mode Type to Create
	FString GameType;

	// The world context object in which this call is taking place
	UObject* WorldContextObject;
};

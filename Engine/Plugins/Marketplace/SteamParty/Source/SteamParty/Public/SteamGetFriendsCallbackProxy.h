// Copyright 2013-2017 W3 Studios, LLC. All Rights Reserved.
#pragma once

#include "Net/OnlineBlueprintCallProxyBase.h"
#include "SteamBeaconTypes.h"
#include "SteamGetFriendsCallbackProxy.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBlueprintSteamGetFriendsListDelegate, const TArray<FSteamFriendInfo>&, Results);

UCLASS(MinimalAPI)
class USteamGetFriendsCallbackProxy : public UOnlineBlueprintCallProxyBase
{
	GENERATED_UCLASS_BODY()

	//Called when there is a successful Steam Friends List query
	UPROPERTY(BlueprintAssignable)
	FBlueprintSteamGetFriendsListDelegate OnSuccess;

	//Called when there is an unsuccessful Steam Friends List query
	UPROPERTY(BlueprintAssignable)
	FBlueprintSteamGetFriendsListDelegate OnFailure;

	//Request Steam for latest Friends List, then broadcasts the Results into a blueprint readable format
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly = "true", WorldContext="WorldContextObject"), Category = "SteamBeacon")
	static USteamGetFriendsCallbackProxy* RequestSteamFriendsList(UObject* WorldContextObject, class APlayerController* PlayerController);

	//UOnlineBlueprintCallProxyBase interface
	virtual void Activate() override;
	//End of UOnlineBlueprintCallProxyBase interface

private:
	
	//Internal callback when the friends list is retrieved
	void OnReadFriendsListComplete(int32 LocalPlayerNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);

	//The player controller triggering things
	TWeakObjectPtr<APlayerController> PlayerControllerWeakPtr;

	//The delegate executed by the Steam online system
	FOnReadFriendsListComplete ReadFriendsListCompleteDelegate;

	// The world context object in which this call is taking place
	UObject* WorldContextObject;
};


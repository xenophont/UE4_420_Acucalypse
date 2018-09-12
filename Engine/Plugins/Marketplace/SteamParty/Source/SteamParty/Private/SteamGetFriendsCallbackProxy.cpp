// Copyright 2013-2017 W3 Studios, LLC. All Rights Reserved.

#include "SteamGetFriendsCallbackProxy.h"
#include "SteamParty.h"
#include "OnlineSubsystemUtils.h"


//Callback Proxy for blueprint access to the Steam Friends List
USteamGetFriendsCallbackProxy::USteamGetFriendsCallbackProxy(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ReadFriendsListCompleteDelegate(FOnReadFriendsListComplete::CreateUObject(this, &ThisClass::OnReadFriendsListComplete))
{
}

USteamGetFriendsCallbackProxy* USteamGetFriendsCallbackProxy::RequestSteamFriendsList(UObject* WorldContextObject, class APlayerController* PlayerController)
{
	USteamGetFriendsCallbackProxy* Proxy = NewObject<USteamGetFriendsCallbackProxy>();
	Proxy->PlayerControllerWeakPtr = PlayerController;
	Proxy->WorldContextObject = WorldContextObject;
	return Proxy;
}

void USteamGetFriendsCallbackProxy::Activate()
{
	IOnlineFriendsPtr FriendsInterface = Online::GetFriendsInterface();
	if (FriendsInterface.IsValid() && PlayerControllerWeakPtr.IsValid())
	{
		ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PlayerControllerWeakPtr->Player);

		//Request Steam Friends List
		if (LocalPlayer && FriendsInterface->ReadFriendsList(LocalPlayer->GetControllerId(), EFriendsLists::ToString((EFriendsLists::Default)), ReadFriendsListCompleteDelegate))
		{
			return;
		}
	}
	
	//Fail immediately
	TArray<FSteamFriendInfo> Results;
	OnFailure.Broadcast(Results);
}

void USteamGetFriendsCallbackProxy::OnReadFriendsListComplete(int32 LocalPlayerNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
	IOnlineFriendsPtr FriendsInterface = Online::GetFriendsInterface();
	if (FriendsInterface.IsValid() && bWasSuccessful)
	{
		TArray<FSteamFriendInfo> OutSteamFriendsList;
		TArray< TSharedRef<FOnlineFriend> > InSteamFriendsList;
		FriendsInterface->GetFriendsList(LocalPlayerNum, ListName, InSteamFriendsList);

		if (InSteamFriendsList.Num())
		{
			//Copy Out Important Data for blueprints
			for (int32 i = 0; i < InSteamFriendsList.Num(); i++)
			{
				//Get the data
				FSteamFriendInfo NewSteamFriendInfo;
				TSharedRef<FOnlineFriend> OnlineFriend = InSteamFriendsList[i];
				FOnlineUserPresence UserPresence = OnlineFriend->GetPresence();
				
				//Copy the data
				NewSteamFriendInfo.UniqueNetId.SetUniqueNetId(OnlineFriend->GetUserId());
				NewSteamFriendInfo.DisplayName = OnlineFriend->GetDisplayName();
				NewSteamFriendInfo.RealName = OnlineFriend->GetRealName();
				NewSteamFriendInfo.OnlineState = ((ESteamPresenceState)((int32)UserPresence.Status.State));
				NewSteamFriendInfo.bIsOnline = UserPresence.bIsOnline;
				NewSteamFriendInfo.bIsPlaying = UserPresence.bIsPlaying;
				NewSteamFriendInfo.bIsPlayingThisGame = UserPresence.bIsPlayingThisGame;
				NewSteamFriendInfo.bIsJoinable = UserPresence.bIsJoinable;
				NewSteamFriendInfo.bHasVoiceSupport = UserPresence.bHasVoiceSupport;
				
				//Store the data
				OutSteamFriendsList.Add(NewSteamFriendInfo);
			}
		}
		OnSuccess.Broadcast(OutSteamFriendsList);
	}
	else
	{
		TArray<FSteamFriendInfo> Results;
		OnFailure.Broadcast(Results);
	}
}

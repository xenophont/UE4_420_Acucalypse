// Copyright 2013-2017 W3 Studios, LLC. All Rights Reserved.

#pragma once

#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemSteamTypes.h"
#include "OnlineSubsystemTypes.h"
#include "IPAddressSteam.h"
#include "SocketSubsystemSteam.h"
#include "OnlineSubsystemNames.h"
#include "GameFramework/OnlineReplStructs.h"
#include "SteamBeaconTypes.generated.h"


//Wrapper to expose searchresults
USTRUCT(BlueprintType)
struct FSteamBeaconSessionResult
{
	GENERATED_BODY()
public:

	//Will not get replicated!
	FOnlineSessionSearchResult OnlineResult;
	
	//Some future stuff added will be refactored when Steam OSS gets updated by Epic, in the works!!

	// Type of session this is, affects interpretation of id below
	UPROPERTY(BlueprintReadWrite, Category = "SteamBeacon")
		FString SessionType;
	// The ip & port that the host is listening on (valid for LAN/GameServer)
	UPROPERTY(BlueprintReadWrite, Category = "SteamBeacon")
		FString HostAddr;
	// The Steam P2P address that the host is listening on (valid for GameServer/Lobby)
	UPROPERTY(BlueprintReadWrite, Category = "SteamBeacon")
		FString SteamP2PAddr;
	// Steam Lobby Id or Gameserver Id if applicable
	UPROPERTY(BlueprintReadWrite, Category = "SteamBeacon")
		FString SessionId;

	// Lobby Or GameServer
	UPROPERTY(BlueprintReadWrite, Category = "SteamBeacon")
		bool bIsPresence = false;

	// Lan or Steam
	UPROPERTY(BlueprintReadWrite, Category = "SteamBeacon")
		bool bIsLan = false;
	
	//Constructors
	FSteamBeaconSessionResult() {}
	FSteamBeaconSessionResult(FOnlineSessionSearchResult& Result)
	{
		OnlineResult = Result;
				
		//Copy out important Steam Connection Info
		bIsPresence = Result.Session.SessionSettings.bUsesPresence;
		bIsLan = Result.Session.SessionSettings.bIsLANMatch;

		FOnlineSessionInfoSteam* SessionInfo = (FOnlineSessionInfoSteam*)(Result.Session.SessionInfo.Get());
		SessionType = ESteamSession::ToString(SessionInfo->SessionType);
		
		//Typically Dedicated Server Session
		if (SessionInfo->HostAddr.IsValid() && SessionInfo->HostAddr->IsValid())
		{
			HostAddr = SessionInfo->HostAddr->ToString(true);
		}
		
		//Typically Presence Server Session,  ListenServer
		if (SessionInfo->SteamP2PAddr.IsValid() && SessionInfo->SteamP2PAddr->IsValid())
		{
			SteamP2PAddr = SessionInfo->SteamP2PAddr->ToString(true);
		}

		//SessionId are derived differently for Presence versus Dedicated servers
		if (bIsPresence)
		{
			//OwnerId for Presence
			SessionId = Result.Session.OwningUserId->ToString();
		}
		else
		{
			//LobbyId for Dedicated
			SessionId = SessionInfo->SessionId.ToString();		
			//SessionId = Result.GetSessionIdStr();
		}
	}
	FSteamBeaconSessionResult(FString InSessionId)
	{
		bIsPresence = true;
		SessionId = InSessionId;
	}
};

//Struct To Store Party Info
USTRUCT(BlueprintType)
struct FPartySaveInfo
{
	GENERATED_USTRUCT_BODY()

public:

	//Is Currently in a Party
	UPROPERTY(BlueprintReadWrite, Category = "Party")
		bool IsInParty = false;

	//Is Currently Party Leader
	UPROPERTY(BlueprintReadWrite, Category = "Party")
		bool IsPartyLeader = false;

	//Last Joined Party Session Info -- This can not be replicated or accessed by blueprints
	FOnlineSessionSearchResult LastJoinPartySession;
};

//Steam Friend Log on State Taken straight from EOnlinePresenceState and exposed to blueprints
UENUM(BlueprintType)
enum class ESteamPresenceState : uint8
{
	Online			UMETA(DisplayName = "Online"),
	Offline			UMETA(DisplayName = "Offline"),
	Away			UMETA(DisplayName = "Away"),
	ExtendedAway	UMETA(DisplayName = "Extended Away"),
	DoNotDisturb	UMETA(DisplayName = "Do Not Disturb"),	
	Chat			UMETA(DisplayName = "Chat")
	
};

//Struct To Online Friend data to Blueprints
USTRUCT(BlueprintType)
struct FSteamFriendInfo
{
	GENERATED_USTRUCT_BODY()

public:
	
	//Unique Player Id, Steam Player ID
	UPROPERTY(BlueprintReadWrite, Category = "Steam Friend")
		FUniqueNetIdRepl UniqueNetId;
	//Displayed Steam Name
	UPROPERTY(BlueprintReadWrite, Category = "Steam Friend")
		FString DisplayName;
	//Real Name
	UPROPERTY(BlueprintReadWrite, Category = "Steam Friend")
		FString RealName;
	//Online State of Friend, Online-Offline ect..
	UPROPERTY(BlueprintReadWrite, Category = "Steam Friend")
		ESteamPresenceState OnlineState;
	
	//Means Friend is in any other state than Offline
	UPROPERTY(BlueprintReadWrite, Category = "Steam Friend")
		bool bIsOnline;
	//Is playing a game on Steam, may not be your game
	UPROPERTY(BlueprintReadWrite, Category = "Steam Friend")
		bool bIsPlaying;
	//Is playing your game also
	UPROPERTY(BlueprintReadWrite, Category = "Steam Friend")
		bool bIsPlayingThisGame;
	//Are they in a joinable lobby session
	UPROPERTY(BlueprintReadWrite, Category = "Steam Friend")
		bool bIsJoinable;
	//If the friend has voice support
	UPROPERTY(BlueprintReadWrite, Category = "Steam Friend")
		bool bHasVoiceSupport;

};

//Steam Avatar Sizes
UENUM(Blueprintable)
enum class ESteamAvatarSize : uint8
{
	AvatarSmall		= 1,
	AvatarMedium	= 2,
	AvatarLarge		= 3
};

//Party States
UENUM(Blueprintable)
enum class EPartyStatus : uint8
{
	PartyNone			UMETA(DisplayName = "Party None"),
	PartyLeader			UMETA(DisplayName = "Party Leader"),
	PartyMember			UMETA(DisplayName = "Party Member")
};

//Party Chat Message type
UENUM(Blueprintable)
enum class EPartyChatType : uint8
{
	Global			UMETA(DisplayName = "Global"),
	Whisper			UMETA(DisplayName = "Whisper"),
};

//Struct For Party Messages
USTRUCT(BlueprintType)
struct FPartyMessage
{
	GENERATED_USTRUCT_BODY()

public:

	//Time Message was sent
	UPROPERTY(BlueprintReadWrite, Category = "Chat")
		float TimeSent;

	//Sender Name
	UPROPERTY(BlueprintReadWrite, Category = "Chat")
		FString SenderName;

	//Unique Player Id, Steam Player ID for Sender
	UPROPERTY(BlueprintReadWrite, Category = "Chat")
		FUniqueNetIdRepl SenderId;

	//Chat Message
	UPROPERTY(BlueprintReadWrite, Category = "Chat")
		FString Message;

	//Message Type
	UPROPERTY(BlueprintReadWrite, Category = "Chat")
		EPartyChatType ChatType;

	//Unique Player Id, Steam Player ID for target message, if whisper
	UPROPERTY(BlueprintReadWrite, Category = "Chat")
		FUniqueNetIdRepl ReceiverId;
};
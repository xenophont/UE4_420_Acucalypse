// Copyright 2013-2017 W3 Studios, LLC. All Rights Reserved.

#pragma once

#include "GameFramework/PlayerController.h"
#include "DummyPlayerController.generated.h"

//Dummy Player Controller class not networked to represent Players For each SteamBeaconClient
//Not meant to be used except for satisfying internal engine constraints as coded especially for Voice Chat, thats why no API & Privatized
UCLASS()
class ADummyPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	
	//Overridden so that we can route the OnNetCleanup to the SteamBeaconClient also since it no longer owns the Connection
	virtual void OnNetCleanup(class UNetConnection* Connection) override;

	//Don't spawn in Camera Managers for dummy player controllers, it also spawns in audio junk we don't and messes with volumes
	//virtual void SpawnPlayerCameraManager() override {}
};

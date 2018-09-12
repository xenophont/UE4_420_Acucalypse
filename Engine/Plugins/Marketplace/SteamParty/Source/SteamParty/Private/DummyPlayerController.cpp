// Copyright 2013-2017 W3 Studios, LLC. All Rights Reserved.

#include "DummyPlayerController.h"
#include "SteamBeaconGameInstance.h"
#include "Engine/World.h"

//Clean up when netconnection is closed down
void ADummyPlayerController::OnNetCleanup(UNetConnection* Connection)
{
	//We need to relay the NetCleanup back to any existing BeaconClients
	USteamBeaconGameInstance* pGI = Cast<USteamBeaconGameInstance>(GetWorld()->GetGameInstance());
	if (pGI && pGI->BeaconHostObject)
	{
		pGI->BeaconHostObject->ProcessBeaconNetCleanup(Connection);
	}
	
	Super::OnNetCleanup(Connection);
}
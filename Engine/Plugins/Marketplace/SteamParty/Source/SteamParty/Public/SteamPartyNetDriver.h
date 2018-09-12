// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

//
// Steam Party implementation of the SteamNet driver 
//

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "IpNetDriver.h"
//#include "SteamNetDriver.h"

#include "SteamPartyNetDriver.generated.h"

class Error;
class FNetworkNotify;

UCLASS(transient, config = Engine)
class STEAMPARTY_API USteamPartyNetDriver : public UIpNetDriver
{
	GENERATED_BODY()
		
	//~ Begin UNetDriver Interface
	virtual int32 ServerReplicateActors(float DeltaSeconds) override;
	//~ End UNetDriver Interface
};

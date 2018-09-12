// Copyright 2013-2017 W3 Studios, LLC. All Rights Reserved.

#pragma once

#include "OnlineBeaconHost.h"
#include "SteamBeaconTypes.h"
#include "SteamBeaconListener.generated.h"

/**
* Main actor that listens for side channel communication from another Unreal Engine application
*
* The ASteamBeaconListener listens for connections to route to a registered AOnlineBeaconHostObject
* The ASteamBeaconHost (AOnlineBeaconHostObject) is responsible for spawning the server version of the AOnlineBeaconClient
* The ASteamBeaconListener pairs the two client actors, verifies the validity of the exchange, and accepts/continues the connection
*/

//Steam Party Beacon Listener Server, listens for connections to route to a registered AOnlineBeaconHostObject 
UCLASS()
class STEAMPARTY_API ASteamBeaconListener : public AOnlineBeaconHost
{
	GENERATED_BODY()
	
public:
	
	//~ Begin FNetworkNotify Interface
	//virtual EAcceptConnection::Type NotifyAcceptingConnection() override;
	virtual void NotifyAcceptedConnection(UNetConnection* Connection) override;
	virtual bool NotifyAcceptingChannel(UChannel* Channel) override;
	virtual void NotifyControlMessage(UNetConnection* Connection, uint8 MessageType, FInBunch& Bunch) override;
	//~ End FNetworkNotify Interface
		
};

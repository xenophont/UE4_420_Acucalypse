// Copyright 2013-2017 W3 Studios, LLC. All Rights Reserved.

#include "SteamBeaconListener.h"
#include "SteamBeaconClient.h"
#include "Net/DataChannel.h"
#include "Misc/NetworkVersion.h"
#include "Engine/World.h"
#include "DummyPlayerController.h"


bool ASteamBeaconListener::NotifyAcceptingChannel(UChannel* Channel)
{
	//Base Class will perform checks on the data for validity, no need to do it again
	return Super::NotifyAcceptingChannel(Channel);
}


void ASteamBeaconListener::NotifyAcceptedConnection(UNetConnection* Connection)
{
	Super::NotifyAcceptedConnection(Connection);
}

void ASteamBeaconListener::NotifyControlMessage(UNetConnection* Connection, uint8 MessageType, class FInBunch& Bunch)
{
	Super::NotifyControlMessage(Connection, MessageType, Bunch);
	
	/*if (NetDriver->ServerConnection == nullptr)
	{
		switch (MessageType)
		{
			//We receive this control message on an incoming request from a client connection, First Communication
			case NMT_BeaconJoin:
			{
				break;
			}
			//We receive the Ack(Callback) when the client successfully accepts the NetGUI sent, Beacon CLient is now functional
			case NMT_BeaconNetGUIDAck:
			{
				break;
			}
		}
	}*/
}


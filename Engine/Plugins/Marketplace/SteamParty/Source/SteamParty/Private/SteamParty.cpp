// Copyright 2013-2017 W3 Studios, LLC. All Rights Reserved.

#include "SteamParty.h"

#define LOCTEXT_NAMESPACE "FSteamPartyModule"

DEFINE_LOG_CATEGORY(LogSteamBeacon);

void FSteamPartyModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	UE_LOG(LogSteamBeacon, Log, TEXT("Plugin SteamParty Module Started Up"));
}

void FSteamPartyModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSteamPartyModule, SteamParty)
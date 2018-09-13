// Crated by dorgon, All Rights Reserved.
// email: dorgonman@hotmail.com
// blog: dorgon.horizon-studio.net

#pragma once
//engine
//Monolithic headers should not be used by this module. Please change it to explicitly include the headers it needs.
//#include "UMG.h"

#include "Misc/AssertionMacros.h"
//thirdparty plugin
//#include "VictoryBPFunctionLibrary.h"









//General Log
//CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogTemp, Log, All);
//HORIZONLIBRARY_API 
DECLARE_LOG_CATEGORY_EXTERN(HorizonUI, Log, All);
#define UE_HORIZONUI_FATAL(msg, ...) UE_LOG(HorizonUI, Fatal, TEXT(msg), ##__VA_ARGS__)
#define UE_HORIZONUI_ERROR(msg, ...) UE_LOG(HorizonUI, Error, TEXT(msg), ##__VA_ARGS__)
#define UE_HORIZONUI_WARNING(msg, ...) UE_LOG(HorizonUI, Warning, TEXT(msg), ##__VA_ARGS__)
#define UE_HORIZONUI_DISPLAY(msg, ...) UE_LOG(HorizonUI, Display, TEXT(msg), ##__VA_ARGS__)
#define UE_HORIZONUI_LOG(msg, ...) UE_LOG(HorizonUI, Log, TEXT(msg), ##__VA_ARGS__)
#define UE_HORIZONUI_VERBOSE(msg, ...) UE_LOG(HorizonUI, Verbose, TEXT(msg), ##__VA_ARGS__)
#define UE_HORIZONUI_VERY_VERBOSE(msg, ...) UE_LOG(HorizonUI, VeryVerbose, TEXT(msg), ##__VA_ARGS__)


FName GName_HorizonUIPlugin = "HorizonUIPlugin";
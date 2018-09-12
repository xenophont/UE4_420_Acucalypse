// Copyright 2013-2017 W3 Studios, LLC. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class SteamParty : ModuleRules
{
	public SteamParty(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
       
	   //Changed from AddRange which creates relative to engine, EPIC fix implemented here
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));

        PrivateIncludePaths.AddRange(
            new string[] {
                "SteamParty/Private",
                // ... add other private include paths required here ...
			}
            );

        //Expose Pack_Scope of Steam to Public
        PublicDefinitions.Add("ONLINESUBSYSTEMSTEAM_PACKAGE=1");

        //Required for some private headers needed for steam sockets support.
        //var EngineDir = Path.GetFullPath(BuildConfiguration.RelativeEnginePath);
        var EngineDir = Path.GetFullPath(Target.RelativeEnginePath);
        PrivateIncludePaths.AddRange(
            new string[] {
                            Path.Combine(EngineDir, @"Plugins\Online\OnlineSubsystemSteam\Source\Private")
                        });
               
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "Lobby",
                "OnlineSubsystem",
                "OnlineSubsystemUtils",
                "OnlineSubsystemSteam",
                "Sockets",
                "Steamworks",
                //"CoreUObject",
                //"Engine",
                // ... add other public dependencies that you statically link with here ...
			}
            );
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
               	//"Slate",
				//"SlateCore",
                // ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
            }
		);

        PrivateIncludePathModuleNames.AddRange(
            new string[] 
            {
                
            }
        );

    }
}

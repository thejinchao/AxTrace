// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

namespace UnrealBuildTool.Rules
{
    public class AxTrace : ModuleRules
    {
    	public AxTrace(ReadOnlyTargetRules Target) : base(Target)
    	{
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

    		PrivateIncludePaths.AddRange(
    			new string[] {
    				"AxTrace/Private",
    				// ... add other private include paths required here ...
    			}
    		);
    			
    		
    		PublicDependencyModuleNames.AddRange(
    			new string[]
    			{
                    "Engine",
                    "Core",
                    "CoreUObject",
                    "Sockets",
                    "Networking",
					"DeveloperSettings"
				}
			);
    	}
    }

}

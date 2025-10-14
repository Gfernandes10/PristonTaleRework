// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PristonTaleRework : ModuleRules
{
	public PristonTaleRework(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"Niagara",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"PristonTaleRework",
			"PristonTaleRework/Variant_Strategy",
			"PristonTaleRework/Variant_Strategy/UI",
			"PristonTaleRework/Variant_TwinStick",
			"PristonTaleRework/Variant_TwinStick/AI",
			"PristonTaleRework/Variant_TwinStick/Gameplay",
			"PristonTaleRework/Variant_TwinStick/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}

using UnrealBuildTool;

public class Afterlight : ModuleRules
{
	public Afterlight(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicIncludePaths.Add(ModuleDirectory);
		PrivateIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameplayTags",
			"UMG",
			"Slate",
			"SlateCore",
			"CinematicCamera",
			"LevelSequence",
			"MovieScene",
			"MovieSceneTracks"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"DeveloperSettings"
		});
	}
}

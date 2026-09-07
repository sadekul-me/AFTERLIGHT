using UnrealBuildTool;

public class AFTERLIGHTTarget : TargetRules
{
	public AFTERLIGHTTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("Afterlight");

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			WindowsPlatform.CompilerVersion = "14.44.35207";
		}
	}
}

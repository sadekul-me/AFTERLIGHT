using UnrealBuildTool;

public class AFTERLIGHTEditorTarget : TargetRules
{
	public AFTERLIGHTEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("Afterlight");

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			WindowsPlatform.CompilerVersion = "14.44.35207";
		}
	}
}

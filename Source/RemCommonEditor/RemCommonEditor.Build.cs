// Copyright RemRemRemRe. All Rights Reserved.

using UnrealBuildTool;

public class RemCommonEditor : ModuleRules
{
	public RemCommonEditor(ReadOnlyTargetRules target) : base(target)
	{
		CppCompileWarningSettings.ShadowVariableWarningLevel = WarningLevel.Error;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		CppStandard = CppStandardVersion.EngineDefault;
		CppCompileWarningSettings.UnsafeTypeCastWarningLevel = WarningLevel.Warning;
		
		CppCompileWarningSettings.NonInlinedGenCppWarningLevel = WarningLevel.Warning;
		bUseUnity = false;
		
		PrivateDependencyModuleNames.AddRange(
			[
				"Core",
				"CoreUObject",
				"Engine",
				
				"DeveloperSettings",
				"PropertyEditor",
				"GameplayTags",
				"Slate",
				"SlateCore",
				"InputCore",
				
				"RemCommon",
				"RemEditorUtilities",
			]
		);
	}
}

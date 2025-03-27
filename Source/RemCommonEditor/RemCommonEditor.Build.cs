// Copyright RemRemRemRe. All Rights Reserved.

using UnrealBuildTool;

public class RemCommonEditor : ModuleRules
{
	public RemCommonEditor(ReadOnlyTargetRules target) : base(target)
	{
		ShadowVariableWarningLevel = WarningLevel.Error;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		CppStandard = CppStandardVersion.EngineDefault;
		UnsafeTypeCastWarningLevel = WarningLevel.Warning;
		
		bEnableNonInlinedGenCppWarnings = true;
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

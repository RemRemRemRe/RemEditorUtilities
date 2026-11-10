// Copyright RemRemRemRe. 2026. All Rights Reserved.

using UnrealBuildTool;
using Rem.BuildRule;

public class RemEditorUtilitiesTest : ModuleRules
{
	public RemEditorUtilitiesTest(ReadOnlyTargetRules target) : base(target)
	{
		RemSharedModuleRules.Apply(this);

		PrivateDependencyModuleNames.AddRange(
			[
				"Core",
				"CoreUObject",
				"Engine",

				"ClassViewer",
				"GameplayTags",
				"PropertyEditor",
				"UMG",
				"UnrealEd",

				"RemCommon",
				"RemCommonTest",
				"RemEditorUtilities",
				"RemCommonEditor",
			]
		);
	}
}

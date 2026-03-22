// Copyright RemRemRemRe. All Rights Reserved.

using UnrealBuildTool;
using Rem.BuildRule;

public class RemCommonEditor : ModuleRules
{
	public RemCommonEditor(ReadOnlyTargetRules target) : base(target)
	{
        RemSharedModuleRules.Apply(this);
		
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
                "UnrealEd",
				
				"RemCommon",
				"RemEditorUtilities",
			]
		);
	}
}

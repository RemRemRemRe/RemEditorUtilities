// Copyright RemRemRemRe. All Rights Reserved.

using UnrealBuildTool;
using Rem.BuildRule;

public class RemEditorUtilities : ModuleRules
{
	public RemEditorUtilities(ReadOnlyTargetRules target) : base(target)
	{
        RemSharedModuleRules.Apply(this);
		
		PrivateDependencyModuleNames.AddRange(
			[
				"Core",
				"CoreUObject",
				"Engine",
				
				"Slate",
				"SlateCore",
				"UnrealEd",
				"UMG",
				"ClassViewer",
				
				"RemCommon",
			]
		);
	}
}

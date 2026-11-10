// Copyright RemRemRemRe. All Rights Reserved.

using UnrealBuildTool;
using Rem.BuildRule;

public class RemEditorUtilities : ModuleRules
{
	public RemEditorUtilities(ReadOnlyTargetRules target) : base(target)
	{
        RemSharedModuleRules.Apply(this);

        // the public headers expose PropertyEditor types and the Rem concepts
        PublicDependencyModuleNames.AddRange(
        	[
        		"RemCommon",
        		"PropertyEditor",
        	]
        );
		
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

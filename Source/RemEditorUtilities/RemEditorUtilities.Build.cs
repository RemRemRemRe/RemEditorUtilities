// Copyright Epic Games, Inc. All Rights Reserved.


namespace UnrealBuildTool.Rules
{
	public class RemEditorUtilities : ModuleRules
	{
		public RemEditorUtilities(ReadOnlyTargetRules target) : base(target)
		{
			PCHUsage					= PCHUsageMode.UseExplicitOrSharedPCHs;
			bLegacyPublicIncludePaths	= false;
			ShadowVariableWarningLevel	= WarningLevel.Error;
			CppStandard 				= CppStandardVersion.Cpp20;

			PrivateDependencyModuleNames.AddRange(
				new[]
				{
					"Core",
					"CoreUObject",
					"Engine",
					
					"Slate",
					"SlateCore",
					"UMG",
					"PropertyEditor",

					"RemCommon",
				}
			);
		}
	}
}


#pragma once

#include "Editor.h"
#include "Subsystems/AssetEditorSubsystem.h"

namespace Rem::Editor
{
	template<typename T>
	requires std::is_base_of_v<IAssetEditorInstance, T>
	auto GetAssetEditorInstance(UClass* Class) -> decltype(auto)
	{
		if (auto* Blueprint = UBlueprint::GetBlueprintFromClass(Class))
		{
			if (IAssetEditorInstance* AssetEditorInstance =
				GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(Blueprint, false))
			{
				auto* Editor = static_cast<T*>(AssetEditorInstance);
				RemCheckVariable(Editor);

				return Editor;
			}
		}

		return static_cast<T*>(nullptr);
	}
}
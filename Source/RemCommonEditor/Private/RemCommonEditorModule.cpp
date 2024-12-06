// Copyright RemRemRemRe, All Rights Reserved.

#include "RemCommonEditorModule.h"

#include "Modules/ModuleManager.h"
#include "RemConcepts.h"
#include "GameplayTagsManager.h"
#include "GameplayTag/RemGameplayTagWithCategory.h"
#include "PropertyHandle.h"
#include "Details/RemReflectedFunctionCallDataDetails.h"
#include "Details/RemReflectedFunctionDataDetails.h"
#include "Macro/RemAssertionMacros.h"
#include "Struct/RemReflectedFunctionCallData.h"
#include "Struct/RemReflectedFunctionData.h"

IRemCommonEditorModule& IRemCommonEditorModule::Get()
{
	return FModuleManager::LoadModuleChecked< IRemCommonEditorModule >( "RemCommonEditor" );
}

bool IRemCommonEditorModule::IsAvailable()
{
	return FModuleManager::Get().IsModuleLoaded( "RemCommonEditor" );
}


class FRemCommonEditorModule final : public IRemCommonEditorModule
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	using ThisClass = FRemCommonEditorModule;
	FDelegateHandle DelegateHandle;
	static void OnGetCategoriesMetaFromPropertyHandle(const TSharedPtr<IPropertyHandle> PropertyHandle, FString& OutCategoryString);
};

IMPLEMENT_MODULE(FRemCommonEditorModule, RemCommonEditor)

void FRemCommonEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
	IRemCommonEditorModule::StartupModule();

	DelegateHandle = UGameplayTagsManager::Get().OnGetCategoriesMetaFromPropertyHandle.AddStatic(
		&ThisClass::OnGetCategoriesMetaFromPropertyHandle);

	auto& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout(FRemReflectedFunctionCallData::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FRemReflectedFunctionCallDataDetails::MakeInstance));
	PropertyModule.RegisterCustomPropertyTypeLayout(FRemReflectedFunctionData::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FRemReflectedFunctionDataDetails::MakeInstance));
}

void FRemCommonEditorModule::ShutdownModule()
{
	auto& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.UnregisterCustomPropertyTypeLayout(FRemReflectedFunctionData::StaticStruct()->GetFName());
	PropertyModule.UnregisterCustomPropertyTypeLayout(FRemReflectedFunctionCallData::StaticStruct()->GetFName());

	if (auto* GameplayTagsManager = UGameplayTagsManager::GetIfAllocated())
	{
		GameplayTagsManager->OnGetCategoriesMetaFromPropertyHandle.Remove(DelegateHandle);
	}

	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	IRemCommonEditorModule::ShutdownModule();
}

// ReSharper disable once CppPassValueParameterByConstReference
void FRemCommonEditorModule::OnGetCategoriesMetaFromPropertyHandle(const TSharedPtr<IPropertyHandle> PropertyHandle,
	FString& OutCategoryString)
{
	//static_assert(Rem::Concepts::has_tag<FRemGameplayTagWithCategory>, "Tag member of FRemGameplayTagWithCategory is missing!");
	const auto TagPropertyName = FName{TEXTVIEW("Tag")};

	if (const auto* Property = PropertyHandle->GetProperty();
		Property->GetFName() == TagPropertyName && Property->GetOwnerStruct() == FRemGameplayTagWithCategory::StaticStruct())
	{
		void* OutAddress = nullptr;
		RemCheckCondition(PropertyHandle->GetParentHandle()->GetValueData(OutAddress) == FPropertyAccess::Success, return;)

		if (const auto* GameplayTagWithCategory = static_cast<const FRemGameplayTagWithCategory*>(OutAddress);
			GameplayTagWithCategory && GameplayTagWithCategory->GetCategory().IsValid())
		{
			OutCategoryString = GameplayTagWithCategory->GetCategory().GetTagName().ToString();
		}
	}
}

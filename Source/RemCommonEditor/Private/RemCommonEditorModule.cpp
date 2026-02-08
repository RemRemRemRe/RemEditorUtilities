// Copyright RemRemRemRe, All Rights Reserved.

#include "RemCommonEditorModule.h"

#include "Modules/ModuleManager.h"
#include "GameplayTagsManager.h"
#include "GameplayTag/RemGameplayTagWithCategory.h"
#include "PropertyHandle.h"
#include "RemCommonEditorLog.h"
#include "Details/RemReflectedFunctionCallDataDetails.h"
#include "Details/RemReflectedFunctionDataDetails.h"
#include "GameplayTag/RemGameplayTagArray.h"
#include "Macro/RemAssertionMacros.h"
#include "Macro/RemLogMacros.h"
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
    auto* PropertyModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor");
    RemCheckVariable(PropertyModule, return;, REM_NO_LOG_OR_ASSERTION);
    
	PropertyModule->UnregisterCustomPropertyTypeLayout(FRemReflectedFunctionData::StaticStruct()->GetFName());
	PropertyModule->UnregisterCustomPropertyTypeLayout(FRemReflectedFunctionCallData::StaticStruct()->GetFName());

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
	static_assert(sizeof(FRemGameplayTagWithCategory::Tag), "Tag member of FRemGameplayTagWithCategory is missing!");

    RemCheckVariable(PropertyHandle, return;);
    
    const auto* Property = PropertyHandle->GetProperty();
    RemCheckVariable(Property, return;);

	if (Property->GetFName() == GET_MEMBER_NAME_STRING_VIEW_CHECKED(FRemGameplayTagWithCategory, Tag)
		&& Property->GetOwnerStruct() == FRemGameplayTagWithCategory::StaticStruct())
	{
		void* OutAddress = nullptr;
		RemCheckCondition(PropertyHandle->GetParentHandle()->GetValueData(OutAddress) == FPropertyAccess::Success, return;)

		if (const auto* GameplayTagWithCategory = static_cast<const FRemGameplayTagWithCategory*>(OutAddress);
			GameplayTagWithCategory && GameplayTagWithCategory->GetCategory().IsValid())
		{
			OutCategoryString = GameplayTagWithCategory->GetCategory().GetTagName().ToString();
		}
	}
	else if (const FStructProperty* Field = CastField<FStructProperty>(Property);
	    Field && Field->Struct == FGameplayTag::StaticStruct()
	    && Field->GetFName() == GET_MEMBER_NAME_STRING_VIEW_CHECKED(FRemGameplayTagArray, Tags))
    {
        auto Parent{PropertyHandle};
        const FRemGameplayTagArray* GameplayTagWithCategory{};
        do
        {
            Parent = Parent->GetParentHandle();
            if (!Parent.IsValid())
            {
                break;
            }

            if (const auto* StructProperty = CastField<FStructProperty>(Parent->GetProperty());
                StructProperty)
            {
                if (StructProperty->Struct == FRemGameplayTagArray::StaticStruct())
                {
                    void* OutAddress = nullptr;
                    RemCheckCondition(Parent->GetValueData(OutAddress) == FPropertyAccess::Success, return;)
                    
                    GameplayTagWithCategory = static_cast<const FRemGameplayTagArray*>(OutAddress);
                    break;
                }
                
                if (StructProperty->Struct == FInstancedStruct::StaticStruct())
                {
                    void* OutAddress = nullptr;
                    RemCheckCondition(Parent->GetValueData(OutAddress) == FPropertyAccess::Success, return;)
                    
                    if (auto* InstancedStruct = static_cast<const FInstancedStruct*>(OutAddress))
                    {
                        if (GameplayTagWithCategory = InstancedStruct->GetPtr<const FRemGameplayTagArray>();
                            GameplayTagWithCategory)
                        {
                            break;
                        }
                    }
                }
            }
        }
        while (true);

        RemCheckVariable(GameplayTagWithCategory, return;);
	
        if (GameplayTagWithCategory->OptionalCategory.IsValid())
        {
            OutCategoryString = GameplayTagWithCategory->OptionalCategory.GetTagName().ToString();
        }
    }
}

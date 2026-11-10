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
#include "RemCommonEditorGameplayTag.h"
#include "RemEditorUtilitiesStatics.inl"
#include "Struct/RemReflectedFunctionCallData.h"
#include "Struct/RemReflectedFunctionData.h"

IRemCommonEditorModule& IRemCommonEditorModule::Get()
{
    return FModuleManager::LoadModuleChecked<IRemCommonEditorModule>("RemCommonEditor");
}

bool IRemCommonEditorModule::IsAvailable()
{
    return FModuleManager::Get().IsModuleLoaded("RemCommonEditor");
}

namespace
{

class FRemCommonEditorModule final : public IRemCommonEditorModule
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    using ThisClass = FRemCommonEditorModule;
    FDelegateHandle DelegateHandle;
    static void OnGetCategoriesMetaFromPropertyHandle(const TSharedPtr<IPropertyHandle> PropertyHandle,
        FString& OutCategoryString);
};

}

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
    RemCheckVariable(PropertyModule, return;);

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
    // This callback is broadcast for every tag property of every details panel in the editor, so everything that is
    // not provably one of our members is a silent pass-through: another struct's tag member is not an error of ours
    // to report, and neither is a member whose value cannot be read (a multi-selection has no single value).
    const auto* const Property = PropertyHandle.IsValid()
                                     ? PropertyHandle->GetProperty()
                                     : nullptr;
    if (!Rem::CommonEditor::IsTagCategoryProperty(Property))
    {
        return;
    }

    if (Property->GetOwnerStruct() == FRemGameplayTagWithCategory::StaticStruct())
    {
        if (const auto* TagWithCategory =
                Rem::Editor::GetDeclaringStructValue<FRemGameplayTagWithCategory>(PropertyHandle);
            TagWithCategory && TagWithCategory->GetCategory().IsValid())
        {
            OutCategoryString = TagWithCategory->GetCategory().GetTagName().ToString();
        }

        return;
    }

    if (const auto* GameplayTagArray = Rem::Editor::GetDeclaringStructValue<FRemGameplayTagArray>(PropertyHandle);
        GameplayTagArray && GameplayTagArray->OptionalCategory.IsValid())
    {
        OutCategoryString = GameplayTagArray->OptionalCategory.GetTagName().ToString();
    }
}

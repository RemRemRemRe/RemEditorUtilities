// Copyright RemRemRemRe. 2026. All Rights Reserved.

#include "Test/RemEditorUtilitiesTestTypes.h"

#include "ClassFilter/RemEditorUtilitiesClassFilter.h"
#include "ClassViewerFilter.h"
#include "ClassViewerModule.h"
#include "Engine/DataTable.h"
#include "GameFramework/Character.h"
#include "Misc/AutomationTest.h"
#include "UObject/Class.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
using Rem::Editor::Private::URemTestAbstractAsset;
}

DEFINE_SPEC(FRemEditorUtilitiesClassFilterTest, "Rem.EditorUtilities.ClassFilter",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter);

void FRemEditorUtilitiesClassFilterTest::Define()
{
    Describe(TEXT("IsClassAllowed"), [this]
    {
        It(TEXT("should allow everything with an empty filter"), [this]
        {
            FRemEditorUtilitiesClassFilter Filter{};
            const auto FilterFuncs = MakeShared<FClassViewerFilterFuncs>();
            FClassViewerInitializationOptions InitOptions{};

            TestTrue(TEXT("actor allowed"), Filter.IsClassAllowed(InitOptions, AActor::StaticClass(), FilterFuncs));
            TestTrue(TEXT("object allowed"), Filter.IsClassAllowed(InitOptions, UObject::StaticClass(), FilterFuncs));
        });

        It(TEXT("should only allow classes derived from the allowed set"), [this]
        {
            FRemEditorUtilitiesClassFilter Filter{};
            Filter.AllowedClasses.Add(AActor::StaticClass());
            const auto FilterFuncs = MakeShared<FClassViewerFilterFuncs>();
            FClassViewerInitializationOptions InitOptions{};

            TestTrue(TEXT("derived allowed"), Filter.IsClassAllowed(InitOptions, ACharacter::StaticClass(), FilterFuncs));
            TestTrue(TEXT("exact class allowed"), Filter.IsClassAllowed(InitOptions, AActor::StaticClass(), FilterFuncs));
            TestFalse(TEXT("unrelated denied"), Filter.IsClassAllowed(InitOptions, UObject::StaticClass(), FilterFuncs));
        });

        It(TEXT("should deny classes derived from the disallowed set"), [this]
        {
            FRemEditorUtilitiesClassFilter Filter{};
            Filter.DisallowedClasses.Add(AActor::StaticClass());
            const auto FilterFuncs = MakeShared<FClassViewerFilterFuncs>();
            FClassViewerInitializationOptions InitOptions{};

            TestFalse(TEXT("derived denied"), Filter.IsClassAllowed(InitOptions, ACharacter::StaticClass(),
                FilterFuncs));
            TestTrue(TEXT("unrelated allowed"), Filter.IsClassAllowed(InitOptions, UObject::StaticClass(), FilterFuncs));
        });

        It(TEXT("should deny classes with disallowed class flags"), [this]
        {
            FRemEditorUtilitiesClassFilter Filter{};
            Filter.DisallowedClassFlags = CLASS_Abstract;
            const auto FilterFuncs = MakeShared<FClassViewerFilterFuncs>();
            FClassViewerInitializationOptions InitOptions{};

            TestFalse(TEXT("abstract denied"), Filter.IsClassAllowed(InitOptions, URemTestAbstractAsset::StaticClass(),
                FilterFuncs));
            // UDataAsset is abstract too; UDataTable is concrete
            TestTrue(TEXT("concrete allowed"), Filter.IsClassAllowed(InitOptions, UDataTable::StaticClass(),
                FilterFuncs));
        });

        It(TEXT("should let the disallowed set win over the allowed set"), [this]
        {
            FRemEditorUtilitiesClassFilter Filter{};
            Filter.AllowedClasses.Add(AActor::StaticClass());
            Filter.DisallowedClasses.Add(APawn::StaticClass());
            const auto FilterFuncs = MakeShared<FClassViewerFilterFuncs>();
            FClassViewerInitializationOptions InitOptions{};

            // ACharacter is an AActor but also an APawn: the disallowed set wins
            TestFalse(TEXT("disallowed wins"), Filter.IsClassAllowed(InitOptions, ACharacter::StaticClass(),
                FilterFuncs));
            TestTrue(TEXT("allowed outside disallowed"), Filter.IsClassAllowed(InitOptions, AActor::StaticClass(),
                FilterFuncs));
        });
    });
}

#endif // WITH_DEV_AUTOMATION_TESTS

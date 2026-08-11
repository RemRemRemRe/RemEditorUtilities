// Copyright RemRemRemRe. 2026. All Rights Reserved.

#include "Test/RemEditorUtilitiesTestTypes.h"

#include "Components/Image.h"
#include "Misc/AutomationTest.h"
#include "PropertyEditorModule.h"
#include "RemEditorUtilitiesStatics.h"
#include "StructUtils/InstancedStruct.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
using Rem::Editor::Private::FRemTestEditorData;
using Rem::Editor::Private::FRemTestEditorNested;
using Rem::Editor::IsInstancedStruct;
using Rem::Editor::GetPropertyPath;
using Rem::Editor::TryGetText;
using Rem::Editor::GetWidgetName;
}

DEFINE_SPEC(FRemEditorUtilitiesStaticsTest, "Rem.EditorUtilities.Statics",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter);

void FRemEditorUtilitiesStaticsTest::Define()
{
    Describe(TEXT("IsInstancedStruct"), [this]
    {
        It(TEXT("should only accept the FInstancedStruct script struct"), [this]
        {
            TestTrue(TEXT("instanced struct"), IsInstancedStruct(FInstancedStruct::StaticStruct()));
            TestFalse(TEXT("plain struct"), IsInstancedStruct(FRemTestEditorData::StaticStruct()));
            TestFalse(TEXT("null"), IsInstancedStruct(nullptr));
        });
    });

    Describe(TEXT("GetPropertyPath"), [this]
    {
        It(TEXT("should return the member path after the struct name"), [this]
        {
            const auto* NumberProperty = FindFProperty<FIntProperty>(FRemTestEditorData::StaticStruct(),
                FName{GET_MEMBER_NAME_ANSI_STRING_VIEW_CHECKED(FRemTestEditorData, Number)});

            TestEqual(TEXT("path"), GetPropertyPath(NumberProperty), FString{TEXT("Number")});
        });

        It(TEXT("should include outer properties for nested members"), [this]
        {
            const auto* NestedProperty = FindFProperty<FStructProperty>(FRemTestEditorData::StaticStruct(),
                FName{GET_MEMBER_NAME_ANSI_STRING_VIEW_CHECKED(FRemTestEditorData, Nested)});
            const auto* ValueProperty = NestedProperty->Struct->FindPropertyByName(
                FName{GET_MEMBER_NAME_ANSI_STRING_VIEW_CHECKED(FRemTestEditorNested, Value)});

            TestEqual(TEXT("nested path"), GetPropertyPath(ValueProperty), FString{TEXT("Value")});
        });
    });

    Describe(TEXT("TryGetText"), [this]
    {
        It(TEXT("should evaluate the predicate on success"), [this]
        {
            const auto Result = TryGetText(FPropertyAccess::Success,
                []()
                {
                    return FText::FromString(TEXT("Resolved"));
                });

            TestEqual(TEXT("text"), Result.ToString(), FString{TEXT("Resolved")});
        });

        It(TEXT("should report multiple values"), [this]
        {
            const auto Result = TryGetText(FPropertyAccess::MultipleValues,
                []()
                {
                    return FText::FromString(TEXT("Resolved"));
                });

            TestEqual(TEXT("text"), Result.ToString(), FString{TEXT("Multiple Values")});
        });

        It(TEXT("should return empty text on failure"), [this]
        {
            const auto Result = TryGetText(FPropertyAccess::Fail,
                []()
                {
                    return FText::FromString(TEXT("Resolved"));
                });

            TestTrue(TEXT("empty"), Result.IsEmpty());
        });
    });

    Describe(TEXT("Constants"), [this]
    {
        It(TEXT("should expose the shared editor values"), [this]
        {
            TestEqual(TEXT("index format"), Rem::Editor::IndexFormat, FString{TEXT("Index [ {0} ]")});
            TestEqual(TEXT("asset combo style"), Rem::Editor::AssetComboStyleName,
                FName{TEXT("PropertyEditor.AssetComboStyle")});
            TestEqual(TEXT("asset name color"), Rem::Editor::AssetNameColorName,
                FName{TEXT("PropertyEditor.AssetName.ColorAndOpacity")});
            TestEqual(TEXT("property padding"), Rem::Editor::PropertyPadding, FVector4f{2.0f, 0.0f, 2.0f, 0.0f});
        });
    });

    Describe(TEXT("GetWidgetName"), [this]
    {
        It(TEXT("should return an empty text for a null widget"), [this]
        {
            TestTrue(TEXT("empty"), GetWidgetName(static_cast<const UWidget*>(nullptr)).IsEmpty());
        });

        It(TEXT("should return a non-empty name for a widget"), [this]
        {
            auto* Image = NewObject<UImage>();

            TestFalse(TEXT("non-empty"), GetWidgetName(Image).IsEmpty());
        });
    });
}

#endif // WITH_DEV_AUTOMATION_TESTS

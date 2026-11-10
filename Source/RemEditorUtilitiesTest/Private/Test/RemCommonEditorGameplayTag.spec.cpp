// Copyright RemRemRemRe, 2026. All Rights Reserved.

#include "Test/RemEditorUtilitiesTestTypes.h"

#include "GameplayTag/RemGameplayTagArray.h"
#include "GameplayTag/RemGameplayTagWithCategory.h"
#include "Misc/AutomationTest.h"
#include "RemCommonEditorGameplayTag.h"
#include "UObject/UnrealType.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
using Rem::CommonEditor::IsTagCategoryProperty;
using Rem::Editor::Private::FRemTestTagLookalike;

/**
 * Property a details panel hands to the categories listener for the given member: a container member is reached
 * through its elements, and an element reports the container's inner property.
 */
const FProperty* GetMemberProperty(const UStruct* OwnerStruct, const FName MemberName)
{
    const FProperty* MemberProperty = OwnerStruct->FindPropertyByName(MemberName);
    if (const auto* ArrayProperty = CastField<FArrayProperty>(MemberProperty);
        ArrayProperty)
    {
        return ArrayProperty->Inner;
    }

    return MemberProperty;
}
}

DEFINE_SPEC(FRemCommonEditorGameplayTagTest, "Rem.CommonEditor.GameplayTag",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter);

void FRemCommonEditorGameplayTagTest::Define()
{
    Describe(TEXT("IsTagCategoryProperty"), [this]
    {
        It(TEXT("should accept the Tag member of FRemGameplayTagWithCategory"), [this]
        {
            const auto* Property = GetMemberProperty(FRemGameplayTagWithCategory::StaticStruct(),
                FName{GET_MEMBER_NAME_ANSI_STRING_VIEW_CHECKED(FRemGameplayTagWithCategory, Tag)});

            TestTrue(TEXT("Tag"), IsTagCategoryProperty(Property));
        });

        It(TEXT("should accept the Tags elements of FRemGameplayTagArray"), [this]
        {
            const auto* Property = GetMemberProperty(FRemGameplayTagArray::StaticStruct(),
                FName{GET_MEMBER_NAME_ANSI_STRING_VIEW_CHECKED(FRemGameplayTagArray, Tags)});

            TestNotNull(TEXT("Tags element property"), Property);
            TestTrue(TEXT("Tags"), IsTagCategoryProperty(Property));
        });

        It(TEXT("should reject a Tags member of an unrelated struct"), [this]
        {
            const auto* Property = GetMemberProperty(FRemTestTagLookalike::StaticStruct(),
                FName{GET_MEMBER_NAME_ANSI_STRING_VIEW_CHECKED(FRemTestTagLookalike, Tags)});

            TestNotNull(TEXT("Tags element property"), Property);
            TestFalse(TEXT("lookalike"), IsTagCategoryProperty(Property));
        });

        It(TEXT("should reject the Category member that filters its sibling Tag"), [this]
        {
            // Category is private, so the name is spelled out; TestNotNull reports a rename as a failure.
            const auto* Property = GetMemberProperty(FRemGameplayTagWithCategory::StaticStruct(),
                FName{TEXT("Category")});

            TestNotNull(TEXT("Category property"), Property);
            TestFalse(TEXT("Category"), IsTagCategoryProperty(Property));
        });

        It(TEXT("should reject a missing property"), [this]
        {
            TestFalse(TEXT("null"), IsTagCategoryProperty(nullptr));
        });
    });
}

#endif

// Copyright RemRemRemRe, All Rights Reserved.


#include "RemCommonEditorGameplayTag.h"

#include "GameplayTagContainer.h"
#include "GameplayTag/RemGameplayTagArray.h"
#include "GameplayTag/RemGameplayTagWithCategory.h"
#include "UObject/UnrealType.h"

namespace Rem::CommonEditor
{
bool IsTagCategoryProperty(const FProperty* Property)
{
    const auto* StructProperty = CastField<FStructProperty>(Property);
    if (!StructProperty || StructProperty->Struct != FGameplayTag::StaticStruct())
    {
        return false;
    }

    if (Property->GetOwnerStruct() == FRemGameplayTagWithCategory::StaticStruct())
    {
        return Property->GetFName() == GET_MEMBER_NAME_CHECKED(FRemGameplayTagWithCategory, Tag);
    }

    return Property->GetOwnerStruct() == FRemGameplayTagArray::StaticStruct()
           && Property->GetFName() == GET_MEMBER_NAME_CHECKED(FRemGameplayTagArray, Tags);
}
}

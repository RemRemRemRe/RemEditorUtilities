// Copyright RemRemRemRe, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FProperty;

namespace Rem::CommonEditor
{
/**
 * @brief Whether the property is one of the two gameplay-tag members that carry a category
 *
 * The gameplay-tag categories listener is broadcast for every tag property of every details panel, so this is the
 * identity check it answers before reading a value: a member name alone is shared with unrelated types, and their
 * handles are not an error of ours. Only the member that a category filters is accepted, never the category itself.
 *
 * @param Property property of the member, or the property of one of the elements of its container - an array
 * element reports the array's inner property, which carries the array's name and the element type
 * @return true for the FRemGameplayTagWithCategory::Tag and FRemGameplayTagArray::Tags members of those structs
 */
REMCOMMONEDITOR_API bool IsTagCategoryProperty(const FProperty* Property);
}

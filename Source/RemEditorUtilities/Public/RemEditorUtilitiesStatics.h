// Copyright RemRemRemRe, All Rights Reserved.

#pragma once

#include "Enum/RemContainerCombination.h"
#include "Math/Vector4.h"

class FDetailWidgetRow;
class IDetailGroup;
class IPropertyHandle;
class IAssetEditorInstance;
class UWidget;
class SWidget;
template<class T>
struct TSoftObjectPtr;
namespace FPropertyAccess
{
	enum Result;
}

namespace Rem::Editor
{
	REMEDITORUTILITIES_API extern const FString IndexFormat;

	REMEDITORUTILITIES_API extern const FName AssetComboStyleName;
	REMEDITORUTILITIES_API extern const FName AssetNameColorName;

	REMEDITORUTILITIES_API extern const FVector4f PropertyPadding;
	
	REMEDITORUTILITIES_API FText GetWidgetName(const UWidget* Widget);
	REMEDITORUTILITIES_API FText GetWidgetName(const TSoftObjectPtr<const UWidget>& Widget);
	REMEDITORUTILITIES_API bool IsInstancedStruct(const UScriptStruct* Struct);

	/**
	 * @brief A valid array element property handle would have children num equal to 1
	 * @param ElementHandle an array element property handle
	 * @return true if valid
	 */
	REMEDITORUTILITIES_API bool IsContainerElementValid(const TSharedRef<IPropertyHandle>& ElementHandle);

	/**
	 * @brief  Build the ChildGroupLayerMapping with given PropertyGroupName, and return the corresponding IDetailGroup pointer
	 * 
	 * eg: a property with category name " ParentCategory | SubCategory | ThisCategory ", would end up with ChildGroupLayerMapping
	 * like this:
	 *			[0] "None"				GroupDefault,
	 *				"ParentCategory"	GroupA
	 *				
	 *			[1] "SubCategory"		GroupB
	 *			
	 *			[2] "ThisCategory"		GroupC
	 *			
	 *	three "layer"(array element), each layer contains all groups in that layer.
	 *	
	 *	the relation between group A, B, C should be "The former is the parent of the latter", in that way, they could be
	 *	correctly (hierarchically) displayed in editor.
	 *
	 *	Pointer of GroupC would be returned
	 * 
	 * @param ChildGroupLayerMapping layered group name to IDetailGroup mapping.
	 * Note it must contain the "start point (no category group)" --- "ChildGroupLayerMapping[0][NAME_None]" element
	 * 
	 * @param PropertyGroupName the group (category) name of a property
	 * 
	 * @return the IDetailGroup pointer of given PropertyGroupName, so you could "AddPropertyRow" under it.
	 * ChildGroupLayerMapping would be properly populated
	 */
	REMEDITORUTILITIES_API IDetailGroup* MakePropertyGroups(TArray<TMap<FName, IDetailGroup*>>& ChildGroupLayerMapping,
		const FName PropertyGroupName);

	using FMakePropertyWidgetFunctor = TFunctionRef<TSharedRef<SWidget>(TSharedRef<IPropertyHandle> PropertyHandle)>;

	/**
	 * @brief Make a custom widget for the property no matter whether if it is a container
	 * @param PropertyHandle handle of the property to customize
	 * @param DetailPropertyRow the row to put our custom widget in
	 * @param ContainerType container type of the Property
	 * @param Functor functor to make custom widget using the PropertyHandle
	 */
	REMEDITORUTILITIES_API void MakeCustomWidgetForProperty(const TSharedRef<IPropertyHandle>& PropertyHandle,
		FDetailWidgetRow& DetailPropertyRow, Enum::EContainerCombination ContainerType, FMakePropertyWidgetFunctor Functor);

	/**
	 * @brief Make a property path used for query property handle (using property path name)
	 * @param Property 
	 * @return 
	 */
	REMEDITORUTILITIES_API FString GetPropertyPath(const FProperty* Property);

	REMEDITORUTILITIES_API FText TryGetText(const FPropertyAccess::Result Result, const TFunctionRef<FText()>& Predicate);
}

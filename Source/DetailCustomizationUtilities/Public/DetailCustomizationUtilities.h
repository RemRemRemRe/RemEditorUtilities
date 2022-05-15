
#pragma once

#include "Components/Widget.h"
#include "IDetailGroup.h"
#include "IDetailPropertyRow.h"
#include "Macro/AssertionMacros.h"
#include "ObjectEditorUtils.h"
#include "PropertyHandle.h"
#include "Templates/PropertyHelper.h"
#include "Templates/IsInstance.h"
#include "Enum/ContainerCombination.h"

using namespace Common;

class UWidget;

#define LOCTEXT_NAMESPACE "DetailCustomizationUtilities"

namespace DetailCustomizationUtilities
{
	inline const FString IndexFormat = TEXT("Index [ {0} ]");

	inline const FName AssetComboStyleName = TEXT("PropertyEditor.AssetComboStyle");
	inline const FName AssetNameColorName = TEXT("PropertyEditor.AssetName.ColorAndOpacity");

	inline const FMargin PropertyPadding(2.0f, 0.0f, 2.0f, 0.0f);
	
	DETAILCUSTOMIZATIONUTILITIES_API FText GetWidgetName(const UWidget* Widget);
	DETAILCUSTOMIZATIONUTILITIES_API FText GetWidgetName(const TSoftObjectPtr<const UWidget>& Widget);

	template<typename ReturnType>
	ReturnType GetCurrentValue(const TSharedPtr<IPropertyHandle> ChildHandle, int32& OutResult)
	{
		if (ChildHandle.IsValid())
		{
			switch (TArray<FString> PerObjectValues;
				OutResult = ChildHandle->GetPerObjectValues(PerObjectValues))
			{
				case FPropertyAccess::Success:
				{
					if (PerObjectValues.Num() > 0)
					{
						if constexpr (TIsInstance<ReturnType, TSoftObjectPtr>::value)
						{
							return ReturnType(PerObjectValues[0]);
						}
						else if constexpr (TIsDerivedFrom<typename TDecay<std::remove_pointer_t<ReturnType>>::Type, UObject>::Value)
						{
							return TSoftObjectPtr<typename TDecay<std::remove_pointer_t<ReturnType>>::Type>(PerObjectValues[0]).Get();
						}
					}
					break;		
				}
				default:
					break;
			}
		}

		return ReturnType{};
	}

	template<typename ReturnType>
	ReturnType GetCurrentValue(const TSharedPtr<IPropertyHandle> ChildHandle)
	{
		int32 Result;
		return GetCurrentValue<ReturnType>(ChildHandle, Result);
	}

	template<typename ValueType>
	FText GetCurrentValueText(const TSharedPtr<IPropertyHandle> ChildHandle, const TFunctionRef<FText (const ValueType&)>& Predicate)
	{
		if (ChildHandle.IsValid())
		{
			int32 Result;
			const ValueType Value = GetCurrentValue<ValueType>(ChildHandle, Result);
			switch (Result)
			{
			case FPropertyAccess::MultipleValues:
				return LOCTEXT("MultipleValues", "Multiple Values");
			case FPropertyAccess::Success:
				return Predicate(Value);
			default:
				break;
			}
		}

		return FText::GetEmpty();
	}

	template <typename ObjectType>
	bool SetObjectValue(const ObjectType* Object, const TSharedPtr<IPropertyHandle> PropertyHandle)
	{
		if (PropertyHandle.IsValid())
		{
			TArray<FString> References;
			for (int32 Index = 0; Index < PropertyHandle->GetNumPerObjectValues(); Index++)
			{
				// using soft object to get the object path string
				TSoftObjectPtr<ObjectType> SoftObject(Object);
				References.Add(SoftObject.ToString());
			}

			// can't use this to set value from UWidgetBlueprintGeneratedClass::WidgetTree(of UClass property i guess),
			// PPF_ParsingDefaultProperties is needed but that is hard coded
			// @see FPropertyValueImpl::ImportText of @line 402 : FPropertyTextUtilities::PropertyToTextHelper
			const bool bResult = PropertyHandle->SetPerObjectValues(References) == FPropertyAccess::Result::Success;
			
			EnsureCondition(bResult);
			
			return bResult;
		}
		
		return {};
	}

	DETAILCUSTOMIZATIONUTILITIES_API
	/**
	 * @brief An valid array element property handle would have children num equal to 1
	 * @param ElementHandle an array element property handle
	 * @return true if valid
	 */
	bool IsContainerElementValid(const TSharedPtr<IPropertyHandle> ElementHandle);
	
	DETAILCUSTOMIZATIONUTILITIES_API
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
	IDetailGroup* MakePropertyGroups(TArray<TMap<FName, IDetailGroup*>>& ChildGroupLayerMapping,
		const FName PropertyGroupName);

	/**
	 * @brief Generate container header widget
	 * @tparam TGroupBuilder any type has "AddGroup" function to add a group
	 * @param ContainerHandle property handle of the container
	 * @param GroupBuilder group builder object reference
	 * @param OnPropertyValueChanged optional call back when container value changed
	 * @return the reference of  container element group
	 */
	template<typename TGroupBuilder>
	IDetailGroup& GenerateContainerHeader(const TSharedPtr<IPropertyHandle> ContainerHandle, TGroupBuilder& GroupBuilder,
										  const FSimpleDelegate& OnPropertyValueChanged = {})
	{
		const FProperty* ContainerProperty = ContainerHandle->GetProperty();
		IDetailGroup& ContainerGroup = GroupBuilder.AddGroup(FObjectEditorUtils::GetCategoryFName(ContainerProperty),
													FObjectEditorUtils::GetCategoryText(ContainerProperty));
	
		ContainerGroup.HeaderProperty(ContainerHandle.ToSharedRef());

		if (OnPropertyValueChanged.IsBound())
		{
			// eg: refresh detail view when value changed, element added, deleted, inserted, etc...
			ContainerHandle->SetOnPropertyValueChanged(OnPropertyValueChanged);

			// eg: child value changed, element added...
			ContainerHandle->SetOnChildPropertyValueChanged(OnPropertyValueChanged);
		}
	
		return ContainerGroup;
	}

	typedef TFunctionRef<void(TSharedPtr<IPropertyHandle> Handle, FDetailWidgetRow& WidgetPropertyRow, EContainerCombination)>
	FPropertyCustomizationFunctor;
	
	// forward declaration
	template<typename PropertyType, typename PropertyBaseClass>
	typename TEnableIf<TIsDerivedFrom<PropertyType, FObjectPropertyBase>::Value, void>
	::Type GenerateWidgetForContainerElement(IDetailGroup& ParentGroup, const TSharedPtr<IPropertyHandle> ElementHandle,
		const FPropertyCustomizationFunctor Predicate,
		const EContainerCombination ContainerType);
	
	/**
	 * @brief Generate widget for container content (elements)
	 * @tparam PropertyType the property type you want to customize with 
	 * @tparam PropertyBaseClass property base class
	 * @param ContainerHandle container property handle
	 * @param ContainerGroup container group
	 * @param Predicate property customization predicate
	 * @param ContainerType container type of PropertyHandle.
	 * use it to identify whether the PropertyHandle is the container itself or one of the child handle of the original container and its container type
	 */
	template<typename PropertyType, typename PropertyBaseClass>
	typename TEnableIf<TIsDerivedFrom<PropertyType, FObjectPropertyBase>::Value, void>
	::Type GenerateWidgetForContainerContent(const TSharedPtr<IPropertyHandle> ContainerHandle, IDetailGroup& ContainerGroup,
		const FPropertyCustomizationFunctor Predicate,
		const EContainerCombination ContainerType)
	{
		uint32 ContainerNum;
		ContainerHandle->GetNumChildren(ContainerNum);

		// traverse the container
		for (uint32 Index = 0; Index < ContainerNum; ++Index)
		{
			const TSharedPtr<IPropertyHandle> ElementHandle = ContainerHandle->GetChildHandle(Index);
			CheckCondition(ElementHandle.IsValid(), continue;);

			// Generate widget for container element
			DetailCustomizationUtilities::GenerateWidgetForContainerElement<PropertyType, PropertyBaseClass>(
				ContainerGroup, ElementHandle, Predicate, ContainerType);
		}
	}

	// forward declaration
	template<typename PropertyType, typename PropertyBaseClass>
	typename TEnableIf<TIsDerivedFrom<PropertyType, FObjectPropertyBase>::Value, void>
	::Type GenerateWidgetsForNestedElement(const TSharedPtr<IPropertyHandle> ElementHandle, const uint32 NumChildren,
		TArray<TMap<FName, IDetailGroup*>>& ChildGroupLayerMapping, const uint32 Layer,
		const FPropertyCustomizationFunctor Predicate,
			const EContainerCombination ContainerType);
	
	/**
	 * @brief Generate widget for a container element
	 * @tparam PropertyType the property type you want to customize with 
	 * @tparam PropertyBaseClass property base class 
	 * @param ParentGroup container group
	 * @param ElementHandle element property handle
	 * @param Predicate property customization predicate
	 * @param ContainerType container type of PropertyHandle.
	 * use it to identify whether the PropertyHandle is the container itself or one of the child handle of the original container and its container type
	 */
	template<typename PropertyType, typename PropertyBaseClass>
	typename TEnableIf<TIsDerivedFrom<PropertyType, FObjectPropertyBase>::Value, void>
	::Type GenerateWidgetForContainerElement(IDetailGroup& ParentGroup, const TSharedPtr<IPropertyHandle> ElementHandle,
		const FPropertyCustomizationFunctor Predicate,
		const EContainerCombination ContainerType)
	{
		// add index[] group
		const FName ElementGroupName = *FString::Format(*IndexFormat,{ElementHandle->GetIndexInArray()});
		IDetailGroup& ElementGroup = ParentGroup.AddGroup(ElementGroupName, FText::FromName(ElementGroupName));
		
		if (IDetailPropertyRow& ElementGroupPropertyRow = ElementGroup.HeaderProperty(ElementHandle.ToSharedRef());
			ContainerType != EContainerCombination::ContainerItself)
		{
			// generate widget for TMap / TSet / TArray element
			FDetailWidgetRow& DetailWidgetRow = ElementGroupPropertyRow.CustomWidget();
			Predicate(ElementHandle, DetailWidgetRow, ContainerType);
			return;
		}
	
		// add element properties and groups

		if (!IsContainerElementValid(ElementHandle))
		{
			// invalid element that don't have any children
			return;
		}
	
		const TSharedPtr<IPropertyHandle> ElementValueHandle = ElementHandle->GetChildHandle(0);
		uint32 NumChildren;
		ElementValueHandle->GetNumChildren(NumChildren);

		if (NumChildren <= 0)
		{
			return;
		}

		TArray<TMap<FName, IDetailGroup*>> ChildGroupLayerMapping{ { {NAME_None, &ElementGroup} } };
	
		GenerateWidgetsForNestedElement<PropertyType, PropertyBaseClass>(ElementValueHandle, NumChildren,
			ChildGroupLayerMapping, 0, Predicate, ContainerType);
	}

	/**
	 * @brief Generate widgets for nested element (properties of an array element)
	 * @tparam PropertyType the property type you want to customize with 
	 * @tparam PropertyBaseClass property base class
	 * @param ElementHandle element property handle
	 * @param NumChildren children num of element property handle
	 * @param ChildGroupLayerMapping layered group name to IDetailGroup mapping.
	 * Note it must contain the "start point (no category group)" --- "ChildGroupLayerMapping[0][NAME_None]" element
	 * @param Layer Index of ChildGroupLayerMapping indicates which layer it should resides
	 * @param Predicate property customization predicate
	 * @param ContainerType container type of PropertyHandle.
	 * use it to identify whether the PropertyHandle is the container itself or one of the child handle of the original container and its container type
	 */
	template<typename PropertyType, typename PropertyBaseClass>
	typename TEnableIf<TIsDerivedFrom<PropertyType, FObjectPropertyBase>::Value, void>
	::Type GenerateWidgetsForNestedElement(const TSharedPtr<IPropertyHandle> ElementHandle, const uint32 NumChildren,
		TArray<TMap<FName, IDetailGroup*>>& ChildGroupLayerMapping, const uint32 Layer,
		const FPropertyCustomizationFunctor Predicate,
		const EContainerCombination ContainerType)
	{
		const UStruct* Base = PropertyBaseClass::StaticClass();
		
		for (uint32 Index = 0; Index < NumChildren; ++Index)
		{
			TSharedPtr<IPropertyHandle> ChildHandle = ElementHandle->GetChildHandle(Index);
			CheckCondition(ChildHandle.IsValid(), continue;);

			// if this child is a property
			if (const FProperty* Property = ChildHandle->GetProperty())
			{
				const FName PropertyGroupName = FObjectEditorUtils::GetCategoryFName(ChildHandle->GetProperty());
				
				IDetailGroup* PropertyGroup = MakePropertyGroups(ChildGroupLayerMapping, PropertyGroupName);

				// PropertyGroup need to be valid from now on
				CheckPointer(PropertyGroup, continue;);

				using namespace PropertyHelper;
				bool bNeedCustomWidget = false;
				if (const PropertyType* ObjectPropertyBase = CastField<PropertyType>(Property))
				{
					if (ObjectPropertyBase->PropertyClass->IsChildOf(Base))
					{
						bNeedCustomWidget = true;
					}
				}
				else if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
				{
					if (IsPropertyClassChildOf<PropertyType>(ArrayProperty->Inner, Base))
					{
						IDetailGroup& ContainerGroup = GenerateContainerHeader(ChildHandle, *PropertyGroup);
						GenerateWidgetForContainerContent<PropertyType, PropertyBaseClass>(ChildHandle, ContainerGroup, Predicate, EContainerCombination::Array);
						continue;
					}
				}
				else if (const FMapProperty* MapProperty = CastField<FMapProperty>(Property))
				{
					if (const bool IsPropertyClassChildOfResult[2] =
						{
							IsPropertyClassChildOf<PropertyType>(MapProperty->KeyProp, Base), // bMapKey
							IsPropertyClassChildOf<PropertyType>(MapProperty->ValueProp, Base) // bMapValue
						};
						IsPropertyClassChildOfResult[0] || IsPropertyClassChildOfResult[1])
					{
						IDetailGroup& ContainerGroup = GenerateContainerHeader(ChildHandle, *PropertyGroup);
						
						EContainerCombination MapContainerType{};
						if (IsPropertyClassChildOfResult[0] && IsPropertyClassChildOfResult[1])
						{
							MapContainerType = EContainerCombination::Map;
						}
						else if (IsPropertyClassChildOfResult[0])
						{
							MapContainerType = EContainerCombination::MapKey;
						}
						else if (IsPropertyClassChildOfResult[1])
						{
							MapContainerType = EContainerCombination::MapValue;
						}
						
						GenerateWidgetForContainerContent<PropertyType, PropertyBaseClass>(ChildHandle, ContainerGroup, Predicate, MapContainerType);
						continue;
					}
				}
				else if (const FSetProperty* SetProperty = CastField<FSetProperty>(Property))
				{
					if (IsPropertyClassChildOf<PropertyType>(SetProperty->ElementProp, Base))
					{
						IDetailGroup& ContainerGroup = GenerateContainerHeader(ChildHandle, *PropertyGroup);
						GenerateWidgetForContainerContent<PropertyType, PropertyBaseClass>(ChildHandle, ContainerGroup, Predicate, EContainerCombination::Set);
						continue;
					}
				}
				
				// add property row
				IDetailPropertyRow& WidgetPropertyRow = PropertyGroup->AddPropertyRow(ChildHandle.ToSharedRef());
				WidgetPropertyRow.EditCondition(ChildHandle->IsEditable(), {});

				if (bNeedCustomWidget)
				{
					Predicate(ChildHandle, WidgetPropertyRow.CustomWidget(), ContainerType);
				}
			}
			// if this child is a category
			else
			{
				// prevent duplicate adding on same layer but different category
				if (ChildGroupLayerMapping.Num() <= static_cast<int32>(Layer + 1))
				{
					ChildGroupLayerMapping.AddDefaulted();
				}

				uint32 NumChildrenOfChildHandle;
				ChildHandle->GetNumChildren(NumChildrenOfChildHandle);
				if (NumChildrenOfChildHandle != 0)
				{
					// generate property group and nested property widgets
					GenerateWidgetsForNestedElement<PropertyType, PropertyBaseClass>(ChildHandle, NumChildrenOfChildHandle,
						ChildGroupLayerMapping, Layer+ 1, Predicate, ContainerType);
				}
			}
		}
	}

	typedef TFunctionRef<TSharedRef<SWidget>(TSharedPtr<IPropertyHandle> PropertyHandle)> FMakePropertyWidgetFunctor;

	DETAILCUSTOMIZATIONUTILITIES_API
	/**
	 * @brief Make a custom widget for the property no matter whether if it is a container
	 * @param PropertyHandle handle of the property to customize
	 * @param DetailPropertyRow the row to put our custom widget in
	 * @param ContainerType container type of the Property
	 * @param Functor functor to make custom widget using the PropertyHandle
	 */
	void MakeCustomWidgetForProperty(TSharedPtr<IPropertyHandle> PropertyHandle, FDetailWidgetRow& DetailPropertyRow,
		EContainerCombination ContainerType, FMakePropertyWidgetFunctor Functor);

	DETAILCUSTOMIZATIONUTILITIES_API
	/**
	 * @brief Make a property path used for query property handle (using property path name)
	 * @param Property 
	 * @return 
	 */
	FString GetPropertyPath(const FProperty* Property);

}

#undef LOCTEXT_NAMESPACE
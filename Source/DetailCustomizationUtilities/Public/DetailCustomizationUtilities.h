
#pragma once

#include "Components/Widget.h"
#include "IDetailGroup.h"
#include "IDetailPropertyRow.h"
#include "Macro/AssertionMacros.h"
#include "ObjectEditorUtils.h"
#include "PropertyHandle.h"

class UWidget;

#define LOCTEXT_NAMESPACE "DetailCustomizationUtilities"

namespace FDetailCustomizationUtilities
{
	DETAILCUSTOMIZATIONUTILITIES_API
	enum class EMemberContainerType : uint8
	{
		NotAContainer,
		Array,
		Set,
		Map,
		MapKey,
		MapValue,
	};
	
	DETAILCUSTOMIZATIONUTILITIES_API FText GetWidgetName(const UWidget* Widget);

	template<typename ReturnType>
	ReturnType* GetCurrentValue(const TSharedPtr<IPropertyHandle> ChildHandle, int32& OutResult)
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
						// TODO template this
						return TSoftObjectPtr<ReturnType>(PerObjectValues[0]).Get();
					}
					break;		
				}
				default:
					break;
			}
		}

		return {};
	}

	template<typename ValueType>
	FText GetCurrentValueText(const TSharedPtr<IPropertyHandle> ChildHandle, const TFunction<FText (const ValueType*)>& Predicate)
	{
		if (ChildHandle.IsValid())
		{
			int32 Result;
			const ValueType* Value = GetCurrentValue<ValueType>(ChildHandle, Result);
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

	DETAILCUSTOMIZATIONUTILITIES_API
	bool IsArrayElementValid(const TSharedPtr<IPropertyHandle> ElementHandle);

	template<typename TGroupBuilder>
	IDetailGroup& GenerateContainerHeader(const TSharedPtr<IPropertyHandle> PropertyHandle, TGroupBuilder& GroupBuilder,
										  const FSimpleDelegate& OnComponentsChanged = {})
	{
		const FProperty* ComponentsProperty = PropertyHandle->GetProperty();
		IDetailGroup& ComponentsGroup = GroupBuilder.AddGroup(FObjectEditorUtils::GetCategoryFName(ComponentsProperty),
													FObjectEditorUtils::GetCategoryText(ComponentsProperty));
	
		ComponentsGroup.HeaderProperty(PropertyHandle.ToSharedRef());

		if (OnComponentsChanged.IsBound())
		{
			// refresh detail view when value changed, element added, deleted, inserted, etc...
			PropertyHandle->SetOnPropertyValueChanged(OnComponentsChanged);
		}
	
		return ComponentsGroup;
	}

	template<typename PropertyType>
	void GenerateWidgetForContainerElement(IDetailGroup& ParentGroup, const TSharedPtr<IPropertyHandle> ElementHandle,
		const TFunction<void(const TSharedPtr<IPropertyHandle>, IDetailPropertyRow&, const EMemberContainerType)> Predicate,
		const EMemberContainerType ContainerType);
	
	template<typename PropertyType>
	void GenerateWidgetForContainerContent(const TSharedPtr<IPropertyHandle> PropertyHandle, IDetailGroup& ComponentsGroup,
		const TFunction<void(const TSharedPtr<IPropertyHandle> Handle, IDetailPropertyRow& WidgetPropertyRow, const EMemberContainerType)>& Predicate,
		const EMemberContainerType ContainerType)
	{
		uint32 ComponentNum;
		PropertyHandle->GetNumChildren(ComponentNum);

		// traverse components array
		for (uint32 Index = 0; Index < ComponentNum; ++Index)
		{
			const TSharedPtr<IPropertyHandle> ComponentHandle = PropertyHandle->GetChildHandle(Index);
			CheckCondition(ComponentHandle, continue;);

			// Generate widget for component
			FDetailCustomizationUtilities::GenerateWidgetForContainerElement<PropertyType>(
				ComponentsGroup, ComponentHandle, Predicate, ContainerType);
		}
	}
	
	template<typename PropertyType>
	void GenerateWidgetsForNestedElement(const TSharedPtr<IPropertyHandle> PropertyHandle, const uint32 NumChildren,
		TArray<TMap<FName, IDetailGroup*>>& ChildGroupLayerMapping, const uint32 Layer,
		const TFunction<void (const TSharedPtr<IPropertyHandle>, IDetailPropertyRow&, const EMemberContainerType)> Predicate,
			const EMemberContainerType ContainerType);
	
	template<typename PropertyType>
	void GenerateWidgetForContainerElement(IDetailGroup& ParentGroup, const TSharedPtr<IPropertyHandle> ElementHandle,
		const TFunction<void(const TSharedPtr<IPropertyHandle>, IDetailPropertyRow&, const EMemberContainerType)> Predicate,
		const EMemberContainerType ContainerType)
	{
		// add index[] group
		const FName ElementGroupName = *FString::Format(TEXT("Index[{0}]"),{ElementHandle->GetIndexInArray()});
		IDetailGroup& ElementGroup = ParentGroup.AddGroup(ElementGroupName, FText::FromName(ElementGroupName));
		
		ElementGroup
		.HeaderProperty(ElementHandle.ToSharedRef())
		.EditCondition(ElementHandle->IsEditable(), {});

		if (!IsArrayElementValid(ElementHandle))
		{
			return;
		}
	
		// add element property and groups
	
		const TSharedPtr<IPropertyHandle> ElementValueHandle = ElementHandle->GetChildHandle(0);
		uint32 NumChildren;
		ElementValueHandle->GetNumChildren(NumChildren);

		if (NumChildren <= 0)
		{
			return;
		}

		TArray<TMap<FName, IDetailGroup*>> ChildGroupLayerMapping{ { {NAME_None, &ElementGroup} } };
	
		GenerateWidgetsForNestedElement<PropertyType>(ElementValueHandle, NumChildren,
			ChildGroupLayerMapping, 0, Predicate, ContainerType);
	}
	
	template<typename PropertyType>
	void GenerateWidgetsForNestedElement(const TSharedPtr<IPropertyHandle> PropertyHandle, const uint32 NumChildren,
		TArray<TMap<FName, IDetailGroup*>>& ChildGroupLayerMapping, const uint32 Layer,
		const TFunction<void (const TSharedPtr<IPropertyHandle>, IDetailPropertyRow&, const EMemberContainerType)> Predicate,
		const EMemberContainerType ContainerType)
	{
		for (uint32 Index = 0; Index < NumChildren; ++Index)
		{
			TSharedPtr<IPropertyHandle> ChildHandle = PropertyHandle->GetChildHandle(Index);
			CheckCondition(ChildHandle.IsValid(), continue;);

			// if this child is a property
			if (const FProperty* Property = ChildHandle->GetProperty())
			{
				// add property group
				IDetailGroup* PropertyGroup = ChildGroupLayerMapping[Layer - 1].FindRef(NAME_None);
				
				if (FName PropertyGroupName = FObjectEditorUtils::GetCategoryFName(ChildHandle->GetProperty());
					PropertyGroupName != NAME_None)
				{
					if (PropertyGroup = ChildGroupLayerMapping[Layer].FindRef(PropertyGroupName);
						!PropertyGroup)
					{
						FText InLocalizedDisplayName = FObjectEditorUtils::GetCategoryText(ChildHandle->GetProperty());

						IDetailGroup* ParentGroup = ChildGroupLayerMapping[Layer - 1][NAME_None];
						PropertyGroup = &ParentGroup->AddGroup(PropertyGroupName, InLocalizedDisplayName);

						ChildGroupLayerMapping[Layer].Add(PropertyGroupName, PropertyGroup);
					}
				}

				// PropertyGroup need to be valid from now on
				CheckPointer(PropertyGroup, continue;);

				const UStruct* Base = UWidget::StaticClass();

				bool bNeedCustomWidget = true;
				if (const PropertyType* ObjectPropertyBase = CastField<PropertyType>(Property))
				{
					if (!ObjectPropertyBase->PropertyClass->IsChildOf(Base))
					{
						continue;
					}
				}
				else if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
				{
					if (!Common::IsPropertyClassChildOf<PropertyType>(ArrayProperty->Inner, Base))
					{
						continue;
					}

					IDetailGroup& ContainerGroup = GenerateContainerHeader(ChildHandle, *PropertyGroup);
					GenerateWidgetForContainerContent<PropertyType>(ChildHandle, ContainerGroup, Predicate, EMemberContainerType::Array);
				}
				else if (const FMapProperty* MapProperty = CastField<FMapProperty>(Property))
				{
					const bool bMapKey		= Common::IsPropertyClassChildOf<PropertyType>(MapProperty->KeyProp, Base); 
					const bool bMapValue	= Common::IsPropertyClassChildOf<PropertyType>(MapProperty->ValueProp, Base); 
				
					if (!bMapKey && !bMapValue)
					{
						continue;
					}

					IDetailGroup& ContainerGroup = GenerateContainerHeader(ChildHandle, *PropertyGroup);
					
					EMemberContainerType MapContainerType{};
					if (bMapKey && bMapValue)
					{
						MapContainerType = EMemberContainerType::Map;
					}
					else if (bMapKey)
					{
						MapContainerType = EMemberContainerType::MapKey;
					}
					else if (bMapValue)
					{
						MapContainerType = EMemberContainerType::MapValue;
					}
					
					GenerateWidgetForContainerContent<PropertyType>(ChildHandle, ContainerGroup, Predicate, MapContainerType);
				}
				else if (const FSetProperty* SetProperty = CastField<FSetProperty>(Property))
				{
					if (!Common::IsPropertyClassChildOf<PropertyType>(SetProperty->ElementProp, Base))
					{
						continue;
					}

					IDetailGroup& ContainerGroup = GenerateContainerHeader(ChildHandle, *PropertyGroup);
					GenerateWidgetForContainerContent<PropertyType>(ChildHandle, ContainerGroup, Predicate, EMemberContainerType::Set);
				}
				else
				{
					bNeedCustomWidget = false;
				}
				
				// add property row
				IDetailPropertyRow& WidgetPropertyRow = PropertyGroup->AddPropertyRow(ChildHandle.ToSharedRef());
				WidgetPropertyRow.EditCondition(ChildHandle->IsEditable(), {});

				if (bNeedCustomWidget)
				{
					Predicate(ChildHandle, WidgetPropertyRow, ContainerType);
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
					GenerateWidgetsForNestedElement<PropertyType>(ChildHandle, NumChildrenOfChildHandle,
						ChildGroupLayerMapping, Layer+ 1, Predicate, ContainerType);
				}
			}
		}
	}

}

#undef LOCTEXT_NAMESPACE
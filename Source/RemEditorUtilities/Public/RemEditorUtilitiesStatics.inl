
#pragma once

#include "Editor.h"
#include "Subsystems/AssetEditorSubsystem.h"

#include "IDetailGroup.h"
#include "IDetailPropertyRow.h"
#include "Macro/RemAssertionMacros.h"
#include "ObjectEditorUtils.h"
#include "PropertyHandle.h"
#include "Templates/RemPropertyHelper.h"
#include "Templates/RemIsInstance.h"

namespace Rem::Editor
{
	template<typename T>
	requires std::is_base_of_v<IAssetEditorInstance, T>
	T* GetAssetEditorInstance(UClass* Class)
	{
		auto* Blueprint = UBlueprint::GetBlueprintFromClass(Class);
		RemCheckVariable(Blueprint, return {});

		RemCheckVariable(GEditor, return {});

		auto* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
		RemCheckVariable(AssetEditorSubsystem, return {});

		if (auto* AssetEditorInstance = AssetEditorSubsystem->FindEditorForAsset(Blueprint, false))
		{
			auto* Editor = static_cast<T*>(AssetEditorInstance);
			return Editor;
		}

		return nullptr;
	}

	template<typename ReturnType>
	ReturnType GetCurrentValue(const TSharedPtr<IPropertyHandle>& ChildHandle, int32& OutResult)
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
						using RawType = std::remove_pointer_t<ReturnType>;
						if constexpr (is_instance_v<ReturnType, TSoftObjectPtr>)
						{
							return ReturnType(FSoftObjectPath{PerObjectValues[0]});
						}
						else if constexpr (std::is_base_of_v<UObject, RawType>)
						{
							return TSoftObjectPtr<RawType>(FSoftObjectPath{PerObjectValues[0]}).Get();
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
	ReturnType GetCurrentValue(const TSharedPtr<IPropertyHandle>& ChildHandle)
	{
		int32 Result;
		return GetCurrentValue<ReturnType>(ChildHandle, Result);
	}

	template<typename ValueType>
	FText GetCurrentValueText(const TSharedPtr<IPropertyHandle>& ChildHandle, const TFunctionRef<FText (const ValueType&)>& Predicate)
	{
		if (ChildHandle.IsValid())
		{
			int32 Result;
			const ValueType Value = GetCurrentValue<ValueType>(ChildHandle, Result);
			switch (Result)
			{
			case FPropertyAccess::MultipleValues:
				return NSLOCTEXT("PropertyEditor", "MultipleValues", "Multiple Values");
			case FPropertyAccess::Success:
				return Predicate(Value);
			default:
				break;
			}
		}

		return FText::GetEmpty();
	}

	template <typename ObjectType>
	bool SetObjectValue(const ObjectType* Object, const TSharedPtr<IPropertyHandle>& PropertyHandle)
	{
		if (PropertyHandle.IsValid())
		{
			TArray<FString> References;
			for (int32 Index = 0; Index < PropertyHandle->GetNumPerObjectValues(); Index++)
			{
				// using soft object to get the object path string
				TSoftObjectPtr<const ObjectType> SoftObject(Object);
				References.Add(SoftObject.ToString());
			}

			// can't use this to set value from UWidgetBlueprintGeneratedClass::WidgetTree(of UClass property I guess),
			// PPF_ParsingDefaultProperties is needed but that is hard coded
			// @see FPropertyValueImpl::ImportText of @line 402 : FPropertyTextUtilities::PropertyToTextHelper
			const bool bResult = PropertyHandle->SetPerObjectValues(References) == FPropertyAccess::Result::Success;

			RemEnsureCondition(bResult);

			return bResult;
		}

		return {};
	}

	/**
	 * @brief Generate container header widget
	 * @tparam TGroupBuilder any type has "AddGroup" function to add a group
	 * @param ContainerHandle property handle of the container
	 * @param GroupBuilder group builder object reference
	 * @param OnPropertyValueChanged optional call back when container value changed
	 * @return the reference of  container element group
	 */
	template<typename TGroupBuilder>
	IDetailGroup& GenerateContainerHeader(const TSharedPtr<IPropertyHandle>& ContainerHandle, TGroupBuilder& GroupBuilder,
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

	using FPropertyCustomizationFunctor =
		TFunction<void(TSharedPtr<IPropertyHandle> Handle, FDetailWidgetRow& WidgetPropertyRow, Enum::EContainerCombination)>;

	// forward declaration
	template<Concepts::is_object_property_base PropertyType, typename PropertyBaseClass>
	void GenerateWidgetForContainerElement(IDetailGroup& ParentGroup, const TSharedPtr<IPropertyHandle>& ElementHandle,
		const FPropertyCustomizationFunctor Predicate,
		const Enum::EContainerCombination ContainerType);

	// forward declaration
	template<Concepts::is_object_property_base PropertyType, typename PropertyBaseClass>
	void GenerateWidgetsForNestedElement(const TSharedPtr<IPropertyHandle>& ElementHandle, const uint32 NumChildren,
		TArray<TMap<FName, IDetailGroup*>>& ChildGroupLayerMapping, const uint32 Layer,
		const FPropertyCustomizationFunctor Predicate,
			const Enum::EContainerCombination ContainerType);
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
	template<Concepts::is_object_property_base PropertyType, typename PropertyBaseClass>
	void GenerateWidgetForContainerContent(const TSharedPtr<IPropertyHandle>& ContainerHandle, IDetailGroup& ContainerGroup,
		// ReSharper disable once CppPassValueParameterByConstReference
		const FPropertyCustomizationFunctor Predicate,
		const Enum::EContainerCombination ContainerType)
	{
		uint32 NumChildren;
		ContainerHandle->GetNumChildren(NumChildren);

		if (ContainerType != Enum::EContainerCombination::Struct)
		{
			// traverse the container
			for (uint32 Index = 0; Index < NumChildren; ++Index)
			{
				const TSharedPtr<IPropertyHandle> ElementHandle = ContainerHandle->GetChildHandle(Index);
				RemCheckCondition(ElementHandle.IsValid(), continue;);

				// Generate widget for container element
				Editor::GenerateWidgetForContainerElement<PropertyType, PropertyBaseClass>(
					ContainerGroup, ElementHandle, Predicate, ContainerType);
			}
		}
		else
		{
			const FName StructTypeName = CastFieldChecked<FStructProperty>(ContainerHandle->GetProperty())->Struct->GetFName();

			// member of USTRUCT with no category specified will default to the category of "type name of the USTRUCT",
			// so we add extra mapping here to redirect it
			TArray<TMap<FName, IDetailGroup*>> ChildGroupLayerMapping{ { {NAME_None, &ContainerGroup},
				{StructTypeName, &ContainerGroup} } };

			GenerateWidgetsForNestedElement<PropertyType, PropertyBaseClass>(ContainerHandle, NumChildren,
				ChildGroupLayerMapping, 0, Predicate, ContainerType);
		}
	}

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
	template<Concepts::is_object_property_base PropertyType, typename PropertyBaseClass>
	void GenerateWidgetForContainerElement(IDetailGroup& ParentGroup, const TSharedPtr<IPropertyHandle>& ElementHandle,
		// ReSharper disable once CppPassValueParameterByConstReference
		const FPropertyCustomizationFunctor Predicate,
		const Enum::EContainerCombination ContainerType)
	{
		// add index[] group
		const FName ElementGroupName = *FString::Format(*IndexFormat,{ElementHandle->GetIndexInArray()});
		IDetailGroup& ElementGroup = ParentGroup.AddGroup(ElementGroupName, FText::FromName(ElementGroupName));

		const auto* StructProperty = CastField<FStructProperty>(ElementHandle->GetProperty());

		if (IDetailPropertyRow& ElementGroupPropertyRow = ElementGroup.HeaderProperty(ElementHandle.ToSharedRef());
			ContainerType != Enum::EContainerCombination::ContainerItself && !StructProperty)
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

		const TSharedPtr<IPropertyHandle> ElementValueHandle = StructProperty ? ElementHandle : ElementHandle->GetChildHandle(0);
		uint32 NumChildren;
		ElementValueHandle->GetNumChildren(NumChildren);

		if (NumChildren <= 0)
		{
			return;
		}

		using FNameToDetailGroupMap = TMap<FName, IDetailGroup*>;

		TArray ChildGroupLayerMapping =
			!!StructProperty
				? TArray{
					// member of USTRUCT with no category specified will default to the category of "type name of the USTRUCT",
					// so we add extra mapping here to redirect it
					FNameToDetailGroupMap{{NAME_None, &ElementGroup}, {StructProperty->Struct->GetFName(), &ElementGroup}},
				}
				: TArray{
					FNameToDetailGroupMap{{NAME_None, &ElementGroup}}
				};

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
	 * @param Layer Index of ChildGroupLayerMapping indicates which layer it should reside
	 * @param Predicate property customization predicate
	 * @param ContainerType container type of PropertyHandle.
	 * use it to identify whether the PropertyHandle is the container itself or one of the child handle of the original container and its container type
	 */
	template<Concepts::is_object_property_base PropertyType, typename PropertyBaseClass>
	void GenerateWidgetsForNestedElement(const TSharedPtr<IPropertyHandle>& ElementHandle, const uint32 NumChildren,
		TArray<TMap<FName, IDetailGroup*>>& ChildGroupLayerMapping, const uint32 Layer,
		// ReSharper disable once CppPassValueParameterByConstReference
		const FPropertyCustomizationFunctor Predicate,
		const Enum::EContainerCombination ContainerType)
	{
		for (uint32 Index = 0; Index < NumChildren; ++Index)
		{
			TSharedPtr<IPropertyHandle> ChildHandle = ElementHandle->GetChildHandle(Index);
			RemCheckCondition(ChildHandle.IsValid(), continue;);

			// if this child is a property
			if (const auto* Property = ChildHandle->GetProperty())
			{
				const FName PropertyGroupName = FObjectEditorUtils::GetCategoryFName(ChildHandle->GetProperty());

				IDetailGroup* PropertyGroup = MakePropertyGroups(ChildGroupLayerMapping, PropertyGroupName);

				// PropertyGroup need to be valid from now on
				RemCheckVariable(PropertyGroup, continue;);

				const UStruct* Base = PropertyBaseClass::StaticClass();

				bool bNeedCustomWidget = false;
				if (const auto* ObjectPropertyBase = CastField<PropertyType>(Property))
				{
					if (ObjectPropertyBase->PropertyClass->IsChildOf(Base))
					{
						bNeedCustomWidget = true;
					}
				}
				else if (const auto* ArrayProperty = CastField<FArrayProperty>(Property))
				{
					if (Property::IsPropertyClassChildOf<PropertyType>(ArrayProperty->Inner, Base) || CastField<FStructProperty>(ArrayProperty->Inner))
					{
						IDetailGroup& ContainerGroup = GenerateContainerHeader(ChildHandle, *PropertyGroup);
						GenerateWidgetForContainerContent<PropertyType, PropertyBaseClass>(ChildHandle, ContainerGroup, Predicate, Enum::EContainerCombination::Array);
						continue;
					}
				}
				else if (const auto* MapProperty = CastField<FMapProperty>(Property))
				{
					if (const bool IsPropertyClassChildOfResult[2] =
						{
							Property::IsPropertyClassChildOf<PropertyType>(MapProperty->KeyProp, Base), // bMapKey
							Property::IsPropertyClassChildOf<PropertyType>(MapProperty->ValueProp, Base) || CastField<FStructProperty>(MapProperty->ValueProp) // bMapValue
						};
						IsPropertyClassChildOfResult[0] || IsPropertyClassChildOfResult[1])
					{
						IDetailGroup& ContainerGroup = GenerateContainerHeader(ChildHandle, *PropertyGroup);

						Enum::EContainerCombination MapContainerType{};
						if (IsPropertyClassChildOfResult[0] && IsPropertyClassChildOfResult[1])
						{
							MapContainerType = Enum::EContainerCombination::Map;
						}
						else if (IsPropertyClassChildOfResult[0])
						{
							MapContainerType = Enum::EContainerCombination::MapKey;
						}
						else if (IsPropertyClassChildOfResult[1])
						{
							MapContainerType = Enum::EContainerCombination::MapValue;
						}

						GenerateWidgetForContainerContent<PropertyType, PropertyBaseClass>(ChildHandle, ContainerGroup, Predicate, MapContainerType);
						continue;
					}
				}
				else if (const auto* SetProperty = CastField<FSetProperty>(Property))
				{
					if (Property::IsPropertyClassChildOf<PropertyType>(SetProperty->ElementProp, Base) || CastField<FStructProperty>(SetProperty->ElementProp))
					{
						IDetailGroup& ContainerGroup = GenerateContainerHeader(ChildHandle, *PropertyGroup);
						GenerateWidgetForContainerContent<PropertyType, PropertyBaseClass>(ChildHandle, ContainerGroup, Predicate, Enum::EContainerCombination::Set);
						continue;
					}
				}
				else if (const auto* StructProperty = CastField<FStructProperty>(Property);
					StructProperty)
				{
					// skip instanced struct, or it can't show up in details panel
					if (!IsInstancedStruct(StructProperty->Struct))
					{
						IDetailGroup& ContainerGroup = GenerateContainerHeader(ChildHandle, *PropertyGroup);

						// TODO this will cause inner properties of customized struct type being shown up redundantly.
						// eg: FGameplayTag::TagName, we need a way to identify whether a struct type has a detail customization
						// FPropertyEditorModule::IsCustomizedStruct looks not exposed at the moment
						GenerateWidgetForContainerContent<PropertyType, PropertyBaseClass>(ChildHandle, ContainerGroup, Predicate, Enum::EContainerCombination::Struct);
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
}

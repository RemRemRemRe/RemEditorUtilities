
#include "RemDetailCustomizationUtilities.h"

#include "DetailWidgetRow.h"
#include "Components/Widget.h"
#include "InstancedStruct.h"

namespace Rem::Editor
{

FAutoConsoleVariable CVarWidgetObjectPathAsWidgetName(
	TEXT("Rem.Editor.WidgetObjectPathAsWidgetName"), false,
	TEXT("Show widget object path as widget name"));

FText GetWidgetName(const UWidget* Widget)
{
	if (!Widget)
	{
		return FText::GetEmpty();
	}
	
	return Widget->IsGeneratedName() ? FText::FromName(Widget->GetFName()) : Widget->GetLabelText();;
}

FText GetWidgetName(const TSoftObjectPtr<const UWidget>& Widget)
{
	if (CVarWidgetObjectPathAsWidgetName->GetBool())
	{
		return FText::FromString(Widget.ToString());
	}
	
	return GetWidgetName(Widget.Get());
}

bool IsInstancedStruct(const UScriptStruct* Struct)
{
	return Struct == FInstancedStruct::StaticStruct();
}

bool IsContainerElementValid(const TSharedPtr<IPropertyHandle>& ElementHandle)
{
	// whether the element has valid value
	uint32 IsElementValid;
	ElementHandle->GetNumChildren(IsElementValid);
	return IsElementValid > 0;
}

IDetailGroup* MakePropertyGroups(TArray<TMap<FName, IDetailGroup*>>& ChildGroupLayerMapping, const FName PropertyGroupName)
{
	IDetailGroup* PropertyGroup = nullptr;
	
	if (PropertyGroupName == NAME_None)
	{
		// no group property only show up at first layer
		PropertyGroup = ChildGroupLayerMapping[0][NAME_None];
	}
	else
	{
		// add extra property group
		const FString PropertyGroupString = PropertyGroupName.ToString();
					
		TArray<FString> SplitCategoryString;
		const int32 NumCategory = PropertyGroupString.ParseIntoArray(SplitCategoryString, TEXT("|"));

		// make sure size is big enough
		if (ChildGroupLayerMapping.Num() < NumCategory)
		{
			ChildGroupLayerMapping.AddDefaulted(NumCategory - ChildGroupLayerMapping.Num());
		}

		// build the group hierarchy from top(left) to bottom(right)
		IDetailGroup* LastGroup = nullptr;
		for (int32 CategoryLayer = 0; CategoryLayer < NumCategory; ++CategoryLayer)
		{
			// remove space from start and end, ensuring category is properly retrieved
			const FString CurrentCategoryString = SplitCategoryString[CategoryLayer].TrimStartAndEnd();
				
			const FName CurrentCategoryName(*CurrentCategoryString);
						
			if (PropertyGroup = ChildGroupLayerMapping[CategoryLayer].FindRef(CurrentCategoryName);
				!PropertyGroup)
			{
				IDetailGroup* ParentGroup = CategoryLayer == 0 ? ChildGroupLayerMapping[0][NAME_None] : LastGroup;
				
				const FText InLocalizedDisplayName = FText::FromName(CurrentCategoryName);
				PropertyGroup = &ParentGroup->AddGroup(CurrentCategoryName, InLocalizedDisplayName);

				ChildGroupLayerMapping[CategoryLayer].Add(CurrentCategoryName, PropertyGroup);
			}
						
			LastGroup = PropertyGroup;
		}
	}
	
	return PropertyGroup;
}

void MakeCustomWidgetForProperty(const TSharedPtr<IPropertyHandle>& PropertyHandle, FDetailWidgetRow& DetailPropertyRow,
	// ReSharper disable once CppPassValueParameterByConstReference
	const EContainerCombination ContainerType, const FMakePropertyWidgetFunctor Functor)
{
	using namespace BitOperation;
	
	DetailPropertyRow
	.NameContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.Padding(PropertyPadding)
		.AutoWidth()
		[
			ContainerType == EContainerCombination::ContainerItself
			|| ContainerType == EContainerCombination::Struct
			|| EnumHasAnyFlags(ContainerType, EContainerCombination::Array)
			? PropertyHandle->CreatePropertyNameWidget()

			: ContainerType == EContainerCombination::Set
			? PropertyHandle->CreatePropertyNameWidget(FText::AsNumber(PropertyHandle->GetIndexInArray()))
			:

			EnumHasAllFlags(ContainerType, EContainerCombination::MapKey)
			? Functor(PropertyHandle->GetKeyHandle())
			: PropertyHandle->GetKeyHandle()->CreatePropertyValueWidget()
		]
	]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.Padding(PropertyPadding)
		.AutoWidth()
		[
			ContainerType == EContainerCombination::MapKey
			// pass bDisplayDefaultPropertyButtons as false to prevent delete button get doubled
			? PropertyHandle->CreatePropertyValueWidget(false)
			: Functor(PropertyHandle)
		]			
	];
}

FString GetPropertyPath(const FProperty* Property)
{
	RemCheckVariable(Property, return {});

	const FString PropertyPath = Property->GetPathName();

	int32 Index;
	if (!PropertyPath.FindChar(TEXT(':'), Index))
	{
		return {};
	}
	return PropertyPath.RightChop(Index + 1)/*.Replace(TEXT(":"), TEXT("."))*/;
}
	
}

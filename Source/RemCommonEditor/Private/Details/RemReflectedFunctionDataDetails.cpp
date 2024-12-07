// Copyright RemRemRemRe. 2024. All Rights Reserved.


#include "Details/RemReflectedFunctionDataDetails.h"

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "RemEditorUtilitiesComboButton.inl"
#include "RemEditorUtilitiesStatics.h"
#include "RemEditorUtilitiesStatics.inl"
#include "Macro/RemAssertionMacros.h"
#include "Misc/AssertionMacros.h"
#include "Struct/RemReflectedFunctionCallData.h"

TSharedRef<IPropertyTypeCustomization> FRemReflectedFunctionDataDetails::MakeInstance()
{
	return MakeShared<FRemReflectedFunctionDataDetails>();
}

void FRemReflectedFunctionDataDetails::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	HeaderRow
	.NameContent()
	[
		StructPropertyHandle->CreatePropertyNameWidget()
	];
}

void FRemReflectedFunctionDataDetails::CustomizeChildren(const TSharedRef<IPropertyHandle> StructPropertyHandle,
	IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	FunctionDataPropertyHandle = StructPropertyHandle;

	const auto FunctionOwnerClassPropertyHandle = StructPropertyHandle->GetChildHandle(
		FName{GET_MEMBER_NAME_STRING_VIEW_CHECKED(FRemReflectedFunctionData, FunctionOwnerClass)}, false);

	StructBuilder.AddProperty(FunctionOwnerClassPropertyHandle.ToSharedRef());

	const auto FunctionNameClassPropertyHandle = StructPropertyHandle->GetChildHandle(
		FName{GET_MEMBER_NAME_STRING_VIEW_CHECKED(FRemReflectedFunctionData, FunctionName)}, false);

	const auto FunctionNameClassPropertyHandleRef = FunctionNameClassPropertyHandle.ToSharedRef();
	auto& FunctionNameRow = StructBuilder.AddProperty(FunctionNameClassPropertyHandleRef);

	using namespace Rem::Editor;
	MakeCustomWidgetForProperty(FunctionNameClassPropertyHandleRef, FunctionNameRow.CustomWidget(), Rem::Enum::EContainerCombination::None,
		[this] (const TSharedRef<IPropertyHandle>& WidgetPropertyHandle)
		{
			return MakeComboButton(WidgetPropertyHandle,
				[this, WidgetPropertyHandle] (TSharedRef<SComboButton>& ComboButton)
				{
					return FOnGetContent::CreateStatic(&GetPopupContent<FListViewItemType>,
						ComboButton,
						&ListViewItems,
						SListView<FListViewItemType>::FOnSelectionChanged::CreateLambda(
						[WidgetPropertyHandle, ComboButton](const FListViewItemType& InItem, const ESelectInfo::Type SelectionInfo)
						{
							using namespace Rem::Editor;

							if (SelectionInfo != ESelectInfo::Direct)
							{
								// value should set successfully
								RemCheckCondition(InItem, return);

								RemCheckCondition(WidgetPropertyHandle->SetValue(*InItem) == FPropertyAccess::Success);

								ComboButton->SetIsOpen(false);
							}
						}),
						SListView<FListViewItemType>::FOnGenerateRow::CreateStatic(&OnGenerateListItem<FListViewItemType>,
						[] (const FListViewItemType& InItem) -> FText
						{
							return FText::AsCultureInvariant(FText::FromName(InItem.IsValid() ? *InItem : NAME_None));
						}),
						[WidgetPropertyHandle]() -> FListViewItemType
						{
							FName FunctionName;
							RemCheckCondition(WidgetPropertyHandle->GetValue(FunctionName) == FPropertyAccess::Success);
							return FListViewItemType{MakeShared<FName>(FunctionName)};
						},
						[this, WidgetPropertyHandle](TSharedRef<SListView<FListViewItemType>> ListView)
						{
							return FOnTextChanged::CreateRaw(this, &ThisClass::OnFilterTextChanged, WidgetPropertyHandle, ListView);
						}
					);
				},
				TAttribute<FText>::CreateLambda([WidgetPropertyHandle]() -> FText
				{
					FName FunctionName;
					RemCheckCondition(WidgetPropertyHandle->GetValue(FunctionName) == FPropertyAccess::Success);
					return FText::AsCultureInvariant(FText::FromName(FunctionName));
				}));
		});
}

void FRemReflectedFunctionDataDetails::OnFilterTextChanged(const FText& InFilterText,
	const TSharedRef<IPropertyHandle> FilterTextPropertyHandle, const TSharedRef<SListView<FListViewItemType>> WidgetListView)
{
	auto* FunctionData{Rem::Editor::GetStructPtr<FRemReflectedFunctionData>(FunctionDataPropertyHandle.ToSharedRef())};
	RemCheckVariable(FunctionData, return;);

	if (!FunctionData->FunctionOwnerClass)
	{
		return;
	}

	TArray<FName> FunctionNames;
	FunctionData->FunctionOwnerClass->GenerateFunctionList(FunctionNames);

	const auto& CurrentFilterString = InFilterText.ToString();

	ListViewItems.Reset();
	for (const auto& FunctionName : FunctionNames)
	{
		if (!CurrentFilterString.IsEmpty() && !FunctionName.ToString().Contains(CurrentFilterString))
		{
			continue;
		}

		ListViewItems.Add(MakeShared<FName>(FunctionName));
	}

	ListViewItems.Shrink();
	WidgetListView->RequestListRefresh();
}

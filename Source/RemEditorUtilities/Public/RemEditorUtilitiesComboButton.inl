// Copyright RemRemRemRe, All Rights Reserved.

#pragma once

#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SSearchBox.h"
#include "PropertyHandle.h"
#include "RemEditorUtilitiesStatics.h"
#include "DetailLayoutBuilder.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Views/SListView.h"

namespace Rem::Editor
{

static TSharedRef<SWidget> MakeComboButton(const TSharedRef<IPropertyHandle>& PropertyHandle,
	const TFunctionRef<FOnGetContent(TSharedRef<SComboButton>& ComboButton)>& GetOnGetMenuContent, const TAttribute<FText>& ComboButtonTextAttribute)
{
	using namespace Rem::Editor;

	auto ComboButton = SNew(SComboButton)
		.ButtonStyle(FAppStyle::Get(), AssetComboStyleName)
		.ForegroundColor(FAppStyle::GetColor(AssetNameColorName))
		.ContentPadding(2.0f)
		.IsEnabled(!PropertyHandle->IsEditConst())
		.ButtonContent()
		[
			SNew(STextBlock)
			.Text(ComboButtonTextAttribute)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		];

	ComboButton->SetOnGetMenuContent(GetOnGetMenuContent(ComboButton));

	return ComboButton;
}

template <typename ItemType, typename FunctorGetFOnTextChanged, typename FunctorGetFGetCurrentValue>
static TSharedRef<SWidget> GetPopupContent(const TSharedRef<SComboButton> WidgetListComboButton,
	TArray<ItemType>* ListItemsSource,
	typename SListView<ItemType>::FOnSelectionChanged OnSelectionChanged,
	typename SListView<ItemType>::FOnGenerateRow OnGenerateRow,
	FunctorGetFGetCurrentValue /*TFunction<ItemType()>*/ MakeCurrentListItem,
	FunctorGetFOnTextChanged /*TFunction<FOnTextChanged(TSharedRef<SListView<ItemType>> ListView)>*/ GetOnTextChanged)
{
	using namespace Rem::Editor;

	constexpr bool bInShouldCloseWindowAfterMenuSelection = true;
	constexpr bool bCloseSelfOnly = true;
	FMenuBuilder MenuBuilder(bInShouldCloseWindowAfterMenuSelection, nullptr, nullptr, bCloseSelfOnly);

	MenuBuilder.BeginSection(NAME_None, NSLOCTEXT("PropertyEditor", "BrowseHeader", "Browse"));
	{
		const auto WidgetListView =
		SNew(SListView<ItemType>)
			.ListItemsSource(ListItemsSource)
			.OnSelectionChanged(OnSelectionChanged)
			.OnGenerateRow(OnGenerateRow)
			.SelectionMode(ESelectionMode::Single);

		const FOnTextChanged OnTextChanged = GetOnTextChanged(WidgetListView);

		// Ensure no filter is applied at the time the menu opens
		OnTextChanged.Execute(FText::GetEmpty());

		WidgetListView->SetSelection(MakeCurrentListItem());

		TSharedPtr<SSearchBox> SearchBox;

		const auto MenuContent =
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(SearchBox, SSearchBox)
				.OnTextChanged(OnTextChanged)
			]
			+ SVerticalBox::Slot()
			.Padding(0, 2.0f, 0, 0)
			.AutoHeight()
			[
				SNew(SBox)
				.MaxDesiredHeight(300.0f)
				[
					WidgetListView
				]
			];

		MenuBuilder.AddWidget(MenuContent, FText::GetEmpty(), true);

		WidgetListComboButton->SetMenuContentWidgetToFocus(SearchBox);
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

// why typename FunctorGetText ?
// @see https://stackoverflow.com/a/52508715
template <typename ItemType, typename FunctorGetText>
static TSharedRef<ITableRow> OnGenerateListItem(const ItemType InItem,
	const TSharedRef<STableViewBase>& OwnerTable, FunctorGetText/*TFunction<FText(const ItemType& Item)>*/ GetText)
{
	using namespace Rem::Editor;

	return
		SNew(STableRow<ItemType>, OwnerTable)
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(GetText(InItem))
		];
}

}

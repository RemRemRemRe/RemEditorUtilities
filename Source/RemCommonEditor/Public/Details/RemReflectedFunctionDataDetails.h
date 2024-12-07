// Copyright RemRemRemRe. 2024. All Rights Reserved.

#pragma once

#include "IPropertyTypeCustomization.h"

template <typename ItemType>
class SListView;

class REMCOMMONEDITOR_API FRemReflectedFunctionDataDetails : public IPropertyTypeCustomization
{
	TSharedPtr<IPropertyHandle> FunctionDataPropertyHandle;
	TArray<TSharedPtr<FName>> ListViewItems;

public:
	using ThisClass = FRemReflectedFunctionDataDetails;
	using FListViewItemType = decltype(ListViewItems)::ElementType;

	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

protected:
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder,
		IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

	virtual void OnFilterTextChanged(const FText& InFilterText, const TSharedRef<IPropertyHandle> FilterTextPropertyHandle,
		const TSharedRef<SListView<FListViewItemType>> WidgetListView);
};

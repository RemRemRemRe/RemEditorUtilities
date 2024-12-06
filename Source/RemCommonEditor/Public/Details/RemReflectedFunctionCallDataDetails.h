// Copyright RemRemRemRe. 2024. All Rights Reserved.

#pragma once

#include "IPropertyTypeCustomization.h"

class IPropertyHandleStruct;

class REMCOMMONEDITOR_API FRemReflectedFunctionCallDataDetails : public IPropertyTypeCustomization
{
	TSharedPtr<IPropertyHandle> FunctionCallDataPropertyHandle;

public:
	using ThisClass = FRemReflectedFunctionCallDataDetails;

	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

protected:
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder,
		IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

	void OnFunctionNameChanged(const FPropertyChangedEvent& PropertyChangedEvent) const;
};

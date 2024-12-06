// Copyright RemRemRemRe. 2024. All Rights Reserved.


#include "Details/RemReflectedFunctionCallDataDetails.h"

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "Macro/RemAssertionMacros.h"
#include "Struct/RemReflectedFunctionCallData.h"

TSharedRef<IPropertyTypeCustomization> FRemReflectedFunctionCallDataDetails::MakeInstance()
{
	return MakeShared<FRemReflectedFunctionCallDataDetails>();
}

void FRemReflectedFunctionCallDataDetails::CustomizeHeader(const TSharedRef<IPropertyHandle> StructPropertyHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	HeaderRow
	.NameContent()
	[
		StructPropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		StructPropertyHandle->CreatePropertyValueWidget()
	];

	FunctionCallDataPropertyHandle = StructPropertyHandle;

	const auto FunctionDataPropertyHandle = StructPropertyHandle->GetChildHandle(
		FName{GET_MEMBER_NAME_STRING_VIEW_CHECKED(FRemReflectedFunctionCallData, FunctionData)}, false);

	const auto FunctionNamePropertyHandle = FunctionDataPropertyHandle->GetChildHandle(
		FName{GET_MEMBER_NAME_STRING_VIEW_CHECKED(FRemReflectedFunctionData, FunctionName)}, false);
	FunctionNamePropertyHandle->SetOnPropertyValueChangedWithData(TDelegate<void(const FPropertyChangedEvent&)>::CreateSP(this, &ThisClass::OnFunctionNameChanged));
}

void FRemReflectedFunctionCallDataDetails::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle,
	IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	uint32 ChildrenNum{};
	StructPropertyHandle->GetNumChildren(ChildrenNum);
	for (int Index = 0; Index < ChildrenNum; ++Index)
	{
		auto ChildHandle = StructPropertyHandle->GetChildHandle(Index);
		StructBuilder.AddProperty(ChildHandle.ToSharedRef());
	}
}

void FRemReflectedFunctionCallDataDetails::OnFunctionNameChanged(const FPropertyChangedEvent& PropertyChangedEvent) const
{
	RemCheckVariable(FunctionCallDataPropertyHandle, return;);

	const auto* StructProperty = CastField<FStructProperty>(FunctionCallDataPropertyHandle ->GetProperty());
	RemCheckCondition(StructProperty && StructProperty->Struct == FRemReflectedFunctionCallData::StaticStruct(), return;);

	void* StructPtr{};
	FunctionCallDataPropertyHandle->GetValueData(StructPtr);
	RemCheckVariable(StructPtr, return;);

	const TStructView<FRemReflectedFunctionCallData> DataView{static_cast<uint8*>(StructPtr)};
	RemCheckVariable(DataView, return;);

	DataView.Get().TryFillParameters();
}

// Copyright RemRemRemRe. 2024. All Rights Reserved.


#include "Details/RemReflectedFunctionCallDataDetails.h"

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyRestriction.h"
#include "RemEditorUtilitiesStatics.inl"
#include "ClassFilter/RemEditorUtilitiesClassFilter.h"
#include "Macro/RemAssertionMacros.h"
#include "Struct/RemReflectedFunctionCallData.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Macro/RemLogMacros.h"
#include "Widgets/Notifications/SNotificationList.h"

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

	const auto FunctionOwnerClassPropertyHandle = FunctionDataPropertyHandle->GetChildHandle(
		FName{GET_MEMBER_NAME_STRING_VIEW_CHECKED(FRemReflectedFunctionData, FunctionOwnerClass)}, false);

	// sync useful meta data from outer
	const auto AllowedClassesKey{FName{TEXTVIEW("AllowedClasses")}};
	const auto DisallowedClassesKey{FName{TEXTVIEW("DisallowedClasses")}};
	FunctionOwnerClassPropertyHandle->SetInstanceMetaData(DisallowedClassesKey, FunctionCallDataPropertyHandle->GetMetaData(DisallowedClassesKey));

	static auto RestrictReason = NSLOCTEXT("RemReflectedFunctionCallData", "PassingClassFilter", "Passing meta data of class filter to FunctionOwnerClass");

	const auto Restriction = MakeShared<FPropertyRestriction>(RestrictReason);
	const auto ClassFilter = MakeShared<FRemEditorUtilitiesClassFilter>();
	ClassFilter->AllowedClasses.Append({PropertyCustomizationHelpers::GetClassesFromMetadataString(
		FunctionCallDataPropertyHandle->GetMetaData(AllowedClassesKey))});
	ClassFilter->DisallowedClasses.Append({PropertyCustomizationHelpers::GetClassesFromMetadataString(
		FunctionCallDataPropertyHandle->GetMetaData(DisallowedClassesKey))});

	Restriction->AddClassFilter(ClassFilter);
	FunctionOwnerClassPropertyHandle->AddRestriction(Restriction);
}

void FRemReflectedFunctionCallDataDetails::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle,
	IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	uint32 ChildrenNum{};
	StructPropertyHandle->GetNumChildren(ChildrenNum);
	for (uint32 Index = 0; Index < ChildrenNum; ++Index)
	{
		auto ChildHandle = StructPropertyHandle->GetChildHandle(Index);
		StructBuilder.AddProperty(ChildHandle.ToSharedRef());
	}
}

void FRemReflectedFunctionCallDataDetails::OnFunctionNameChanged(const FPropertyChangedEvent& PropertyChangedEvent) const
{
	RemCheckVariable(FunctionCallDataPropertyHandle, return;);

	auto* FunctionCallData{Rem::Editor::GetStructPtr<FRemReflectedFunctionCallData>(FunctionCallDataPropertyHandle.ToSharedRef())};
	RemCheckVariable(FunctionCallData, return;);

	const auto SavedFunctionName{FunctionCallData->FunctionData.FunctionName};
	FunctionCallData->TryFillParameters();

	if (bool bFunctionNameGotRest = !SavedFunctionName.IsNone() && FunctionCallData->FunctionData.FunctionName.IsNone())
	{
		FNotificationInfo Info(NSLOCTEXT("RemReflectedFunctionCallData", "FunctionIsNotSupported", "Parameter of selected function is not supported"));
		Info.bUseThrobber = true;
		Info.Image = FAppStyle::GetBrush(FName{TEXTVIEW("MessageLog.Warning")});
		Info.FadeOutDuration = 4.0f;
		Info.ExpireDuration = Info.FadeOutDuration;
		FSlateNotificationManager::Get().AddNotification(Info);
	}
}

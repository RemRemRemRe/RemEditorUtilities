// Copyright RemRemRemRe. 2024. All Rights Reserved.


#include "ClassFilter/RemEditorUtilitiesClassFilter.h"

bool FRemEditorUtilitiesClassFilter::IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass,
	TSharedRef< FClassViewerFilterFuncs > InFilterFuncs)
{
	return !InClass->HasAnyClassFlags(DisallowedClassFlags) && InFilterFuncs->IfInChildOfClassesSet(DisallowedClasses , InClass) != EFilterReturn::Passed
		&& InFilterFuncs->IfInChildOfClassesSet(AllowedClasses, InClass) != EFilterReturn::Failed;
}

bool FRemEditorUtilitiesClassFilter::IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions,
	const TSharedRef< const IUnloadedBlueprintData > InUnloadedClassData, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs)
{
	return !InUnloadedClassData->HasAnyClassFlags(DisallowedClassFlags) && InFilterFuncs->IfInChildOfClassesSet(DisallowedClasses , InUnloadedClassData) != EFilterReturn::Passed
		&& InFilterFuncs->IfInChildOfClassesSet(AllowedClasses, InUnloadedClassData) != EFilterReturn::Failed;
}

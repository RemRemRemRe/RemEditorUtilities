// Copyright RemRemRemRe. 2024. All Rights Reserved.


#include "ClassFilter/RemEditorUtilitiesClassFilter.h"

bool FRemEditorUtilitiesClassFilter::IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions,
    const UClass* InClass,
    const TSharedRef<FClassViewerFilterFuncs> InFilterFunc)
{
    return !InClass->HasAnyClassFlags(DisallowedClassFlags) && InFilterFunc->IfInChildOfClassesSet(DisallowedClasses,
               InClass) != EFilterReturn::Passed
           && InFilterFunc->IfInChildOfClassesSet(AllowedClasses, InClass) != EFilterReturn::Failed;
}

bool FRemEditorUtilitiesClassFilter::IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions,
    const TSharedRef<const IUnloadedBlueprintData> InUnloadedClassData,
    const TSharedRef<FClassViewerFilterFuncs> InFilterFunc)
{
    return !InUnloadedClassData->HasAnyClassFlags(DisallowedClassFlags) && InFilterFunc->IfInChildOfClassesSet(
               DisallowedClasses, InUnloadedClassData) != EFilterReturn::Passed
           && InFilterFunc->IfInChildOfClassesSet(AllowedClasses, InUnloadedClassData) != EFilterReturn::Failed;
}

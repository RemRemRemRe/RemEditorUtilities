// Copyright RemRemRemRe. 2024. All Rights Reserved.

#pragma once

#include "ClassViewerFilter.h"

#include "UObject/Class.h"

struct REMEDITORUTILITIES_API FRemEditorUtilitiesClassFilter : IClassViewerFilter
{
    TSet<const UClass*> AllowedClasses;
    TSet<const UClass*> DisallowedClasses;
    EClassFlags DisallowedClassFlags{};

    virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass,
        TSharedRef<FClassViewerFilterFuncs> InFilterFunc) override;

    virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions,
        const TSharedRef<const IUnloadedBlueprintData> InUnloadedClassData,
        TSharedRef<FClassViewerFilterFuncs> InFilterFunc) override;
};

// Copyright Epic Games, Inc. All Rights Reserved.

#include "DetailCustomizationUtilitiesModule.h"

class FDetailCustomizationUtilitiesModule : public IDetailCustomizationUtilitiesModule
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FDetailCustomizationUtilitiesModule, DetailCustomizationUtilitiesModule)

void FDetailCustomizationUtilitiesModule::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
	IDetailCustomizationUtilitiesModule::StartupModule();
}

void FDetailCustomizationUtilitiesModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	IDetailCustomizationUtilitiesModule::ShutdownModule();
}
// Copyright RemRemRemRe, All Rights Reserved.

#include "RemEditorUtilitiesModule.h"

class FRemEditorUtilitiesModule : public IRemEditorUtilitiesModule
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FRemEditorUtilitiesModule, RemEditorUtilities)

void FRemEditorUtilitiesModule::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
	IRemEditorUtilitiesModule::StartupModule();
}

void FRemEditorUtilitiesModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	IRemEditorUtilitiesModule::ShutdownModule();
}
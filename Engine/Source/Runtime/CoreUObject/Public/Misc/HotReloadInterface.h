// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModuleManager.h"
#include "CoreUObject.h"
#include "CompilationResult.h"

/**
* HotReload module interface
*/
class IHotReloadInterface : public IModuleInterface
{
public:
	/**
	 * Tries to gets a pointer to the active HotReload implementation. 
	 */
	static inline IHotReloadInterface* GetPtr()
	{
		static FName HotReload("HotReload");
		return FModuleManager::GetModulePtr<IHotReloadInterface>(HotReload);
	}

	/**
	 * Module manager ticking is only used to check for asynchronously compiled modules that may need to be reloaded
	 */
	virtual void Tick() = 0;

	/**
	 * Save the current state to disk before quitting.
	 */
	virtual void SaveConfig() = 0;

	/**
	 * Queries the compilation method for a given module.
	 *
	 * @param InModuleName Module to query the name of
	 * @return A string describing the method used to compile the module.
	 */
	virtual FString GetModuleCompileMethod(FName InModuleName) = 0;

	/**
	 * Recompiles a single module
	 */
	virtual bool RecompileModule( const FName InModuleName, const bool bReloadAfterRecompile, FOutputDevice &Ar ) = 0;

	/**
	 * Returns whether modules are currently being compiled
	 */
	virtual bool IsCurrentlyCompiling() const = 0;

	/**
	 * Request that current compile be stopped
	 */
	virtual void RequestStopCompilation() = 0;

	/**
	* Adds a function to re-map after hot-reload.
	*/
	virtual void AddHotReloadFunctionRemap(Native NewFunctionPointer, Native OldFunctionPointer) = 0;

	/**
	* Performs hot reload from the editor
	*/
	virtual void DoHotReloadFromEditor() = 0;

	/**
	* HotReload: Reloads the DLLs for given packages.
	* @param	Package				Packages to reload.
	* @param	DependentModules	Additional modules that don't contain UObjects, but rely on them
	* @param	bWaitForCompletion	True if RebindPackages should not return until the recompile and reload has completed
	* @param	Ar					Output device for logging compilation status
	*/
	virtual void RebindPackages(TArray<UPackage*> Packages, TArray<FName> DependentModules, const bool bWaitForCompletion, FOutputDevice &Ar) = 0;

	/** Called when a Hot Reload event has completed. 
	 * 
	 * @param	bWasTriggeredAutomatically	True if the hot reload was invoked automatically by the hot reload system after detecting a changed DLL
	 */
	DECLARE_MULTICAST_DELEGATE_OneParam(FHotReloadEvent, bool /* bWasTriggeredAutomatically */ );
	virtual FHotReloadEvent& OnHotReload() = 0;

	/**
	 * Gets an event delegate that is executed when compilation of a module has started.
	 *
	 * @return The event delegate.
	 */
	DECLARE_MULTICAST_DELEGATE(FModuleCompilerStartedEvent);
	virtual FModuleCompilerStartedEvent& OnModuleCompilerStarted() = 0;

	/**
	 * Gets an event delegate that is executed when compilation of a module has finished.
	 *
	 * The first parameter is the result of the compilation operation.
	 * The second parameter determines whether the log should be shown.
	 *
	 * @return The event delegate.
	 */
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FModuleCompilerFinishedEvent, const FString&, ECompilationResult::Type, bool);
	virtual FModuleCompilerFinishedEvent& OnModuleCompilerFinished() = 0;

	/**
	 * Checks if there's any game modules currently loaded
	 */
	virtual bool IsAnyGameModuleLoaded() const = 0;
};

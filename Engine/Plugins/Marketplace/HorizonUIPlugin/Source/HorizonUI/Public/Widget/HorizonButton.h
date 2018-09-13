// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/Button.h"
#include "HorizonButton.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHorizonButtonEvent, UHorizonButton*, pButton);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnHorizonButtonEventNative, UHorizonButton*);
/**
 * 
 */
UCLASS()
class HORIZONUI_API UHorizonButton : public UButton
{
	GENERATED_BODY()


public: //delegate

	// for blueprint
	UPROPERTY(BlueprintAssignable, Category = "HorizonPlugin|Delegate")
	FOnHorizonButtonEvent OnClickedDelegate;
	UPROPERTY(BlueprintAssignable, Category = "HorizonPlugin|Delegate")
	FOnHorizonButtonEvent OnPressedDelegate;
	UPROPERTY(BlueprintAssignable, Category = "HorizonPlugin|Delegate")
	FOnHorizonButtonEvent OnReleasedDelegate;
	UPROPERTY(BlueprintAssignable, Category = "HorizonPlugin|Delegate")
	FOnHorizonButtonEvent OnHoveredDelegate;
	UPROPERTY(BlueprintAssignable, Category = "HorizonPlugin|Delegate")
	FOnHorizonButtonEvent OnUnhoveredDelegate;


	// for c++ callback binding
	FOnHorizonButtonEventNative OnClickedDelegateNative;
	FOnHorizonButtonEventNative OnPressedDelegateNative;
	FOnHorizonButtonEventNative OnReleasedDelegateNative;
	FOnHorizonButtonEventNative OnHoveredDelegateNative;
	FOnHorizonButtonEventNative OnUnhoveredDelegateNative;
public:

	virtual void OnWidgetRebuilt() override;
#if WITH_EDITOR
		//~ Begin UWidget Interface
		//#if (ENGINE_MAJOR_VERSION <= 4) && (ENGINE_MINOR_VERSION <= 12)
		//virtual const FSlateBrush* GetEditorIcon() override;
		//#endif
	virtual const FText GetPaletteCategory() override;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
protected:  //dynamic delegate
	UFUNCTION()
	virtual void OnPressedButton();
	UFUNCTION()
	virtual void OnReleasedButton();
	UFUNCTION()
	virtual void OnClickedButton();
	UFUNCTION()
	virtual void OnHoveredButton();
	UFUNCTION()
	virtual void OnUnhoveredButton();

	
	
};

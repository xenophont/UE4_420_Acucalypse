// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SlateWrapperTypes.h"

#include "OverlaySlot.generated.h"

/** Slot for the UOverlay panel.  Allows content to be hover above other content. */
UCLASS()
class UMG_API UOverlaySlot : public UPanelSlot
{
	GENERATED_UCLASS_BODY()
	
	/** The padding area between the slot and the content it contains. */
	UPROPERTY(EditDefaultsOnly, Category=Layout)
	FMargin Padding;

	/** The alignment of the object horizontally. */
	UPROPERTY(EditDefaultsOnly, Category=Layout)
	TEnumAsByte<EHorizontalAlignment> HorizontalAlignment;

	/** The alignment of the object vertically. */
	UPROPERTY(EditDefaultsOnly, Category=Layout)
	TEnumAsByte<EVerticalAlignment> VerticalAlignment;

	UFUNCTION(BlueprintCallable, Category="Appearance")
	void SetPadding(FMargin InPadding);

	UFUNCTION(BlueprintCallable, Category="Appearance")
	void SetHorizontalAlignment(EHorizontalAlignment InHorizontalAlignment);

	UFUNCTION(BlueprintCallable, Category="Appearance")
	void SetVerticalAlignment(EVerticalAlignment InVerticalAlignment);

	void BuildSlot(TSharedRef<SHorizontalBox> HorizontalBox);

	// UPanelSlot interface
	virtual void SyncronizeProperties() override;
	// End of UPanelSlot interface

	/** Builds the underlying FSlot for the Slate layout panel. */
	void BuildSlot(TSharedRef<SOverlay> InOverlay);

private:
	//TODO UMG Slots should hold weak or shared refs to slots.

	/** A raw pointer to the slot to allow us to adjust the size, padding...etc at runtime. */
	SOverlay::FOverlaySlot* Slot;
};
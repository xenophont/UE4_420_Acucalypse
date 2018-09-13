// Created by dorgon, All Rights Reserved.
// email: dorgonman@hotmail.com
// blog: dorgon.horizon-studio.net


#include "Widget/HorizonFlipbookWidget.h"
#include "Widget/HorizonWidgetFunctionLibrary.h"

#include "PaperFlipbook.h"
#include "PaperFlipbookComponent.h"
#include "PaperSprite.h"


//UMG
#include "UMGStyle.h"







void UHorizonFlipbookWidget::Tick(float DeltaTime) {

	if (PaperFlipbook) {
		if (NumOfLoop <= 0 || CurrentNumOfLoop < NumOfLoop) {   //if NumOfLoop <= 0, then it will loop forever
			CurrentDuration += DeltaTime;

			SetCurrentImage(CurrentDuration);
			if (CurrentDuration > PaperFlipbook->GetTotalDuration()) {
				CurrentDuration = 0;
				//prevent overflow crash
				if (CurrentNumOfLoop < TNumericLimits<int32>::Max()) {
					CurrentNumOfLoop += 1;
				}
			
			}
		}
	
	}

	
}

TSharedRef<SWidget> UHorizonFlipbookWidget::RebuildWidget() {
	return Super::RebuildWidget();
}


void UHorizonFlipbookWidget::SetCurrentImage(float currentDuration) {
	if (PaperFlipbook) {
		if (ImageWeakPtr.IsValid()) {
			ImageWeakPtr->SetVisibility(ESlateVisibility::HitTestInvisible);
			int32 spriterIndex = PaperFlipbook->GetKeyFrameIndexAtTime(currentDuration);

			UPaperSprite* sprite = PaperFlipbook->GetSpriteAtTime(currentDuration);
			if (sprite) {
				UTexture2D* pTexture = sprite->GetBakedTexture();
				ImageWeakPtr->SetBrushFromTexture(pTexture);
				if (spriterIndex < SourceUV.Num() && spriterIndex < SourceSize.Num()) {
					FVector2D Min = SourceUV[spriterIndex];
					FVector2D textureSize = pTexture->GetImportedSize();
					FVector2D Max = Min + SourceSize[spriterIndex];
					FBox2D UVCoordinates(Min / textureSize, Max / textureSize);
					UVCoordinates.bIsValid = true;
					ImageWeakPtr->Brush.SetUVRegion(UVCoordinates);
				}
			}
		}
	}
}

//virtual TSharedRef<SWidget> RebuildWidget() override;
void UHorizonFlipbookWidget::OnWidgetRebuilt() {
	Super::OnWidgetRebuilt();



	RemoveChild(ImageWeakPtr.Get());
	if (PaperFlipbook) {
		//FName name = *FString::Printf(TEXT("%s.image"), *FString(GetName()));
		ImageWeakPtr = NewObject<UHorizonImage>(GetOuter());
		AddChild(ImageWeakPtr.Get());
		ImageWeakPtr->SetVisibility(ESlateVisibility::HitTestInvisible);

		UCanvasPanelSlot* pCanvasPanelSlot = UHorizonWidgetFunctionLibrary::GetParentCanvasPanelSlot(this);
		verifyf(pCanvasPanelSlot, TEXT("oops! something error"));

		UCanvasPanelSlot* imageCanvasPanelSlot = Cast<UCanvasPanelSlot>(ImageWeakPtr->Slot);
		if (imageCanvasPanelSlot) {
				imageCanvasPanelSlot->SetSize(pCanvasPanelSlot->GetSize());
		}

		ImageWeakPtr->SetColorAndOpacity(ColorAndOpacity.GetSpecifiedColor());

		if (!bIsStartTick) {
			SetCurrentImage(CurrentDuration);
		}
	}
	ResetAnimation();

}


void UHorizonFlipbookWidget::ResetAnimation() {
	CurrentDuration = 0;
	CurrentNumOfLoop = 0;
}


void UHorizonFlipbookWidget::PlayAnimation()
{
	ResetAnimation();
	bIsStartTick = true;
}

void UHorizonFlipbookWidget::StopAnimation()
{
	ResetAnimation();
	bIsStartTick = false;	
}


void UHorizonFlipbookWidget::PauseAnimation()
{
	bIsStartTick = false;
}

void UHorizonFlipbookWidget::ResumeAnimation()
{
	bIsStartTick = true;
}


float UHorizonFlipbookWidget::GetCurrentAnimationDuration() { 
	return CurrentDuration; 
}
void UHorizonFlipbookWidget::SetCurrentAnimationDuration(float duration) 
{
	CurrentDuration = duration;
}
void UHorizonFlipbookWidget::OnBindingChanged(const FName& Property) {
	Super::OnBindingChanged(Property);
}
//~ End UWidget Interface



void UHorizonFlipbookWidget::SetColorAndOpacity(FSlateColor InColorAndOpacity)
{
	if (ImageWeakPtr.IsValid()) {
		ImageWeakPtr->SetColorAndOpacity(InColorAndOpacity.GetSpecifiedColor());
	}
}

void UHorizonFlipbookWidget::SetOpacity(float InOpacity)
{
	FLinearColor CurrentColor = ColorAndOpacity.GetSpecifiedColor();
	CurrentColor.A = InOpacity;

	SetColorAndOpacity(FSlateColor(CurrentColor));
}

//~ Begin UWidget Interface
void UHorizonFlipbookWidget::SynchronizeProperties() {
	Super::SynchronizeProperties();
	OnWidgetRebuilt();
}
//~ End UWidget Interface

//~ Begin UVisual Interface
void UHorizonFlipbookWidget::ReleaseSlateResources(bool bReleaseChildren) {
	Super::ReleaseSlateResources(bReleaseChildren);
}
//~ End UVisual Interface


void UHorizonFlipbookWidget::SetFlipbook(UPaperFlipbook* flipbook) {
	PaperFlipbook = flipbook;
}




void UHorizonFlipbookWidget::SetFlipbookSource(const TArray<FVector2D>& sourceUV,
	const TArray<FVector2D>& sourceSize)
{
	SetFlipbookSourceUV(sourceUV);
	SetFlipbookSourceSize(sourceSize);
};
// Created by dorgon, All Rights Reserved.
// email: dorgonman@hotmail.com
// blog: dorgon.horizon-studio.net
#include "Widget/HorizonDialogueMsgTextBlock.h"
#include "Widget/HorizonFlipbookWidget.h"
#include "Widget/HorizonTextBlock.h"
#include "Widget/HorizonWidgetFunctionLibrary.h"


#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"




void UHorizonDialogueMsgTextBlock::SetSegmentColor(int32 segmentIndex, int32 lineIndex, const FSlateColor& InColor)
{
	for (auto& lineInfoIt : DialogueLineInfoList)
	{
		for (auto& blockInfoIt : lineInfoIt.DialogueBlockInfoList)
		{
			if (blockInfoIt.SegmentReferenceIndex == segmentIndex)
			{
				UHorizonTextBlock* pTextBlock = Cast<UHorizonTextBlock>(blockInfoIt.WidgetWeakPtr.Get());
				if (pTextBlock)
				{
					pTextBlock->SetColorAndOpacity(InColor);
				}
				UHorizonImage* pImage = Cast<UHorizonImage>(blockInfoIt.WidgetWeakPtr.Get());
				if (pImage)
				{
					pImage->SetColorAndOpacity(InColor.GetSpecifiedColor());
				}
				UHorizonFlipbookWidget* pFlipbook = Cast<UHorizonFlipbookWidget>(blockInfoIt.WidgetWeakPtr.Get());
				if (pFlipbook)
				{
					pFlipbook->SetColorAndOpacity(InColor);

				}
			}

		}
	}
}

void UHorizonDialogueMsgTextBlock::AddBackgroundButton(
	int32 segmentIndex,
	FHorizonDialogueBlockInfo& blockInfo,
	UCanvasPanelSlot* pCanvasPanelSlot, 
	const FVector2D& paddingPos)
{

	if (pCanvasPanelSlot)
	{
		auto& segInfo = DialogueSegmentInfoList[segmentIndex];
		if (!blockInfo.WidgetBackgroundWeakPtr.IsValid())
		{
			if (segInfo.FilePath.IsSet())
			{

				auto filePath = UHorizonWidgetFunctionLibrary::NormalizeBlueprintClassFilePath(segInfo.FilePath.GetValue());
				FStringClassReference clsRef = filePath;
				UClass* blueprintClass = clsRef.TryLoadClass<UHorizonButton>();
				if (blueprintClass)
				{
					segInfo.BackgroundButtonClass = blueprintClass;
				}
			}
			
			


			if (segInfo.BackgroundButtonClass)
			{
				blockInfo.WidgetBackgroundWeakPtr = NewObject<UHorizonButton>(GetOuter(),
					segInfo.BackgroundButtonClass, NAME_None, EObjectFlags::RF_NoFlags, 
					nullptr, true);
				AddChild(blockInfo.WidgetBackgroundWeakPtr.Get());
			}
		}
		auto pButton = Cast<UHorizonButton>(blockInfo.WidgetBackgroundWeakPtr.Get());
		if (pButton)
		{
			FHorizonDialogueHypertextResult hyperTextResult;
			hyperTextResult.DialogueMsgTextBlock = this;
			hyperTextResult.SegmentIndex = segmentIndex;
			hyperTextResult.LineIndex = DialogueLineInfoList.Num() - 1;
			hyperTextResult.BlockIndex = DialogueLineInfoList.Last().DialogueBlockInfoList.Num();
			hyperTextResult.HypertextReference = segInfo.HypertextReference.Get(FString());

			FString Pair, PairKey, PairValue;
			auto tempStr = hyperTextResult.HypertextReference;
			while (UGameplayStatics::GrabOption(tempStr, Pair))
			{
				UGameplayStatics::GetKeyValue(Pair, PairKey, PairValue);
				hyperTextResult.UrlEncodeMap.Emplace(PairKey, PairValue);
			}
			//UGameplayStatics::GetKeyValue();
			//FString NewPlayerName = UGameplayStatics::ParseOption(Options, TEXT("Name")).Left(20);
			//
			
			pButton->OnClickedDelegateNative.AddLambda(
				[this, hyperTextResult](UHorizonButton* InButton){
				OnHypertextClickedDelegate.Broadcast(hyperTextResult);
				OnHypertextClickedDelegateNative.Broadcast(hyperTextResult);
				}
			);

			pButton->OnPressedDelegateNative.AddLambda(
				[this, hyperTextResult](UHorizonButton* InButton) {

				OnHypertextPressedDelegate.Broadcast(hyperTextResult);
				OnHypertextPressedDelegateNative.Broadcast(hyperTextResult);
			}
			);

			pButton->OnReleasedDelegateNative.AddLambda(
				[this, hyperTextResult](UHorizonButton* InButton) {

				OnHypertextReleasedDelegate.Broadcast(hyperTextResult);
				OnHypertextReleasedDelegateNative.Broadcast(hyperTextResult);

				if (hyperTextResult.SegmentIndex < DialogueSegmentInfoList.Num()) 
				{
					auto& tmpSegInfo = DialogueSegmentInfoList[hyperTextResult.SegmentIndex];
					tmpSegInfo.bHypertextVisited = true;
					SetSegmentColor(hyperTextResult.SegmentIndex, hyperTextResult.LineIndex, tmpSegInfo.HypertextVisitedColor);
				}
			}
			);
			pButton->OnHoveredDelegateNative.AddLambda(
				[this, hyperTextResult](UHorizonButton* InButton) {
				OnHypertextHoveredDelegate.Broadcast(hyperTextResult);
				OnHypertextHoveredDelegateNative.Broadcast(hyperTextResult);
				if (hyperTextResult.SegmentIndex < DialogueSegmentInfoList.Num())
				{
					auto& tmpSegInfo = DialogueSegmentInfoList[hyperTextResult.SegmentIndex];
					SetSegmentColor(hyperTextResult.SegmentIndex, hyperTextResult.LineIndex, tmpSegInfo.HypertextHoveredColor);
				}
								
			}
			);
			pButton->OnUnhoveredDelegateNative.AddLambda(
				[this, hyperTextResult](UHorizonButton* InButton) {
				OnHypertextUnhoveredDelegate.Broadcast(hyperTextResult);
				OnHypertextUnhoveredDelegateNative.Broadcast(hyperTextResult);
				if (hyperTextResult.SegmentIndex < DialogueSegmentInfoList.Num())
				{
					auto& tmpSegInfo = DialogueSegmentInfoList[hyperTextResult.SegmentIndex];
					FSlateColor color = tmpSegInfo.ColorAndOpacity;
					if (tmpSegInfo.bHypertextVisited)
					{
						color = tmpSegInfo.HypertextVisitedColor;
					}

					SetSegmentColor(hyperTextResult.SegmentIndex, hyperTextResult.LineIndex, color);
				}
			}
			);

			UCanvasPanelSlot* pButtonCanvasPanelSlot = Cast<UCanvasPanelSlot>(pButton->Slot);
			pButtonCanvasPanelSlot->SetSize(blockInfo.BlockSize);
			pButtonCanvasPanelSlot->SetPosition(paddingPos);


			pButtonCanvasPanelSlot->SetZOrder(pCanvasPanelSlot->ZOrder - 1);
			//pButton->SetVisibility(ESlateVisibility::Visible);
		}
	

		
	}
}

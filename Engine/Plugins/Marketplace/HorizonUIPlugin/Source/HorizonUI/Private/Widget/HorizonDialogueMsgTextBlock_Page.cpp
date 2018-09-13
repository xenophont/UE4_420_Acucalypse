// Created by dorgon, All Rights Reserved.
// email: dorgonman@hotmail.com
// blog: dorgon.horizon-studio.net

#include "Widget/HorizonDialogueMsgTextBlock.h"
#include "HorizonUIPrivate.h"
#include "Widget/HorizonWidgetFunctionLibrary.h"
#include "Widget/HorizonTextBlock.h"


#include "Components/CanvasPanelSlot.h"


void UHorizonDialogueMsgTextBlock::RebuildPageInfoList()
{
	DialoguePageInfoList.Empty();


	UCanvasPanelSlot* pTextBlockCanvasPanelSlot = UHorizonWidgetFunctionLibrary::GetParentCanvasPanelSlot(this);
	if (nullptr == pTextBlockCanvasPanelSlot)
	{
		return;
	}
	float canvasHeight = pTextBlockCanvasPanelSlot->GetSize().Y;
	auto pTextBlockCanvasPanelSlotAnchor = pTextBlockCanvasPanelSlot->GetAnchors();

	if (!FMath::IsNearlyEqual(pTextBlockCanvasPanelSlotAnchor.Maximum.Y, pTextBlockCanvasPanelSlotAnchor.Minimum.Y))
	{
		auto geometry = GetCachedGeometry();
		canvasHeight = geometry.Size.Y;
	}

	int32 startLineIndex = 0;
	int32 currentLineIndex = 0;
	float currentPageHeight = 0;
	while (currentLineIndex < DialogueLineInfoList.Num())
	{
		const auto& lineInfo = DialogueLineInfoList[currentLineIndex];
		float maxPageHeight = FMath::Max((float)Font.Size, canvasHeight - lineInfo.MaxLineHeight);
		if (currentPageHeight >= maxPageHeight)
		{
			//handle extreme case, just paging everyline if startLineIndex == (currentLineIndex - 1)
			if (startLineIndex != (currentLineIndex - 1) )
			{
				--currentLineIndex;
				currentPageHeight = currentPageHeight - (LineMargin.Bottom);
				auto& prevLineInfo = DialogueLineInfoList[currentLineIndex];
				currentPageHeight = currentPageHeight - prevLineInfo.MaxLineHeight;
			}
			DialoguePageInfoList.Emplace(FHorizonDialoguePageInfo(startLineIndex, currentLineIndex, currentPageHeight));
			startLineIndex = currentLineIndex;
			currentPageHeight = 0;
		}
		else
		{
			if (FMath::IsNearlyZero(currentPageHeight))
			{
				currentPageHeight += LineMargin.Top;
			}
			else
			{
				currentPageHeight += LineMargin.Bottom;
			}

			currentPageHeight = currentPageHeight + lineInfo.MaxLineHeight;
			++currentLineIndex;

		}

	}

	//last page
	DialoguePageInfoList.Emplace(FHorizonDialoguePageInfo(startLineIndex, currentLineIndex, currentPageHeight));

}

void UHorizonDialogueMsgTextBlock::SetPageVisiblity(bool bVisibility, const FHorizonDialoguePageInfo& InPageInfo)
{
	for (int32 i = InPageInfo.StartLineIndex; i < InPageInfo.EndLineIndex; ++i)
	{
		FHorizonDialogueLineInfo& lineInfo = DialogueLineInfoList[i];
		for (auto& blockInfo : lineInfo.DialogueBlockInfoList)
		{
			if (blockInfo.WidgetWeakPtr.IsValid()) {
				if (bVisibility)
				{
					blockInfo.WidgetWeakPtr->SetVisibility(ESlateVisibility::HitTestInvisible);
				}
				else
				{
					blockInfo.WidgetWeakPtr->SetVisibility(ESlateVisibility::Hidden);

				}
			}
			if (blockInfo.WidgetBackgroundWeakPtr.IsValid())
			{
				if (bVisibility)
				{
					blockInfo.WidgetBackgroundWeakPtr->SetVisibility(ESlateVisibility::Visible);
				}
				else
				{
					blockInfo.WidgetBackgroundWeakPtr->SetVisibility(ESlateVisibility::Hidden);
				}
			}
		}
	
	}

}



void UHorizonDialogueMsgTextBlock::SetIsAutoNextDialogueMsgPage(bool b)
{
	bIsAutoNextDialogueMsgPage = b;

}

void UHorizonDialogueMsgTextBlock::NextDialogueMsgPage()
{
	++CurrentPageIndex;
	CurrentPageIndex = FMath::Min(CurrentPageIndex, DialoguePageInfoList.Num());
	SetDialogueMsgPage(CurrentPageIndex);

}


void UHorizonDialogueMsgTextBlock::SetDialogueMsgPage(int32 PageIndex)
{
	if (PageIndex < DialoguePageInfoList.Num())
	{
		for (auto& it : DialoguePageInfoList)
		{
			SetPageVisiblity(false, it);
		}
		CurrentPageHeightOffset = 0.0f;
		for (int32 i = 0; i < PageIndex; ++i)
		{
			CurrentPageHeightOffset += DialoguePageInfoList[i].PageHeight;	
			CurrentPageHeightOffset = CurrentPageHeightOffset + (LineMargin.Bottom - LineMargin.Top);
		}
		
		FHorizonDialogueDialoguePageResult result;
		result.PageIndex = PageIndex;
		OnSetDialoguePageFunction.Broadcast(result);
		OnSetDialoguePageFunctionNative.Broadcast(result);
	}
}

void UHorizonDialogueMsgTextBlock::SkipCurrentDialogueMsgPageTick()
{
	auto currentPageInfo = DialoguePageInfoList[CurrentPageIndex];

	for (int32 i = CurrentDialogueLineIndex; i < currentPageInfo.EndLineIndex; ++i)
	{
		FHorizonDialogueLineInfo& lineInfo = DialogueLineInfoList[i];
		

		for (auto& blockInfo : lineInfo.DialogueBlockInfoList)
		{
		
			if (blockInfo.WidgetWeakPtr.IsValid())
			{
				UCanvasPanelSlot* canvasPanelSlot = Cast<UCanvasPanelSlot>(blockInfo.WidgetWeakPtr->Slot);
				auto currentPosition = canvasPanelSlot->GetPosition();
				if (CurrentPageIndex > 0)
				{
					currentPosition.Y = lineInfo.Position.Y - CurrentPageHeightOffset;
				}
				canvasPanelSlot->SetPosition(currentPosition);
				blockInfo.WidgetWeakPtr->SetVisibility(ESlateVisibility::HitTestInvisible);
				if (blockInfo.WidgetBackgroundWeakPtr.IsValid())
				{
					blockInfo.WidgetBackgroundWeakPtr->SetVisibility(ESlateVisibility::Visible);
					UCanvasPanelSlot* buttonCanvasPanelSlot = Cast<UCanvasPanelSlot>(blockInfo.WidgetBackgroundWeakPtr->Slot);
					if (buttonCanvasPanelSlot)
					{
						buttonCanvasPanelSlot->SetPosition(currentPosition);
					}
				}

				if (blockInfo.WidgetWeakPtr->IsA<UHorizonTextBlock>()) {
					auto pTextBlock = Cast<UHorizonTextBlock>(blockInfo.WidgetWeakPtr.Get());
					pTextBlock->SetText(FText::FromString(blockInfo.MsgText));
				}
			}
		}
	}


	
	//Debug auto page position
	//if (currentPageInfo.StartLineIndex == CurrentDialogueLineIndex)
	//{
	//	UE_LOG(LogTemp, Log, TEXT("page %d, line: %d, %f"), CurrentPageIndex, CurrentDialogueLineIndex, currentPosition.Y);
	//}

	
	CurrentDialogueLineIndex = currentPageInfo.EndLineIndex;
}
bool UHorizonDialogueMsgTextBlock::IsDialogueMsgPageEnd()
{
	if (IsDialogueMsgText())
	{
		if (CurrentPageIndex < DialoguePageInfoList.Num())
		{
			auto currentPageInfo = DialoguePageInfoList[CurrentPageIndex];
			return CurrentDialogueLineIndex == currentPageInfo.EndLineIndex;
		}
		else
		{
			return true;
		}
	}
	else
	{
		return true;
	}

}
// Crated by dorgon, All Rights Reserved.
// email: dorgonman@hotmail.com
// blog: dorgon.horizon-studio.net
#include "Widget/HorizonDialogueMsgTextBlock.h"
#include "HorizonUIPrivate.h"
#include "Widget/HorizonTextBlock.h"
#include "Widget/HorizonButton.h"
#include "Widget/HorizonFlipbookWidget.h"
#include "Widget/HorizonWidgetFunctionLibrary.h"
#include "FileSystem/HorizonFileSystem.h"


//SlateCore
#include "Fonts/SlateFontInfo.h"
//UMG

#include "UMGStyle.h"

//#include "Regex.h"
#include "XmlParser.h"


//Engine
#include "PaperFlipbook.h"
#include "Kismet/KismetStringLibrary.h"
#include "Engine/Brush.h"
#include "UObject/ConstructorHelpers.h"
#include "PaperSprite.h"
#include "Engine/Font.h"
#include "Components/CanvasPanelSlot.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Materials/Material.h"
//
//#if WITH_EDITOR
//#include "Editor/Transactor.h"
//#endif
#define LOCTEXT_NAMESPACE "UMG"


FHorizonDialogueSegmentInfo::FHorizonDialogueSegmentInfo()
{
	//HypertextVisitedColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);
	//if (BackgroundButtonClass)
	//{
	//	auto pButton = Cast<UHorizonButton>(BackgroundButtonClass->GetDefaultObject());
	//	if (pButton)
	//	{
	//		//pButton->SetBackgroundColor(FLinearColor::Transparent);
	//		//pButton->WidgetStyle.Normal.TintColor = FSlateColor(FLinearColor::Transparent);
	//		//pButton->WidgetStyle.Pressed.TintColor = FSlateColor(FLinearColor::Transparent);
	//		//pButton->WidgetStyle.Hovered.TintColor = FSlateColor(FLinearColor::Transparent);
	//		//pButton->WidgetStyle.Disabled.TintColor = FSlateColor(FLinearColor::Transparent);
	//		pButton->SetVisibility(ESlateVisibility::Collapsed);
	//		//BackgroundButtonClass->SerializeDefaultObject
	////#if WITH_EDITOR
	////		FTransaction ModifierTransaction;
	////		ModifierTransaction.SaveObject(pButton);
	////#endif
	//	}
	//}
}

/////////////////////////////////////////////////////

UHorizonDialogueMsgTextBlock::UHorizonDialogueMsgTextBlock()
{
	if (!IsRunningDedicatedServer())
	{
		static ConstructorHelpers::FObjectFinder<UFont> RobotoFontObj(TEXT("/Engine/EngineFonts/Roboto"));
		Font = FSlateFontInfo(RobotoFontObj.Object, 24, FName("Bold"));
	}

}

void UHorizonDialogueMsgTextBlock::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	//MyTextBlock.Reset();
}

void UHorizonDialogueMsgTextBlock::SetColorAndOpacity(FSlateColor InColorAndOpacity)
{
	ColorAndOpacity = InColorAndOpacity;
	if (bIsRichText) {
		//RebuildWidget();
	}
	else {
		for (auto& lineInfoIt : DialogueLineInfoList) {
			for (auto& blockInfo : lineInfoIt.DialogueBlockInfoList) {

				if (blockInfo.WidgetWeakPtr.IsValid() && blockInfo.WidgetWeakPtr->IsA<UHorizonTextBlock>()) {
					auto pTextBlock = Cast<UHorizonTextBlock>(blockInfo.WidgetWeakPtr.Get());
					pTextBlock->SetColorAndOpacity(ColorAndOpacity);
				}
			}
		}
	}

}

void UHorizonDialogueMsgTextBlock::SetOpacity(float InOpacity)
{
	FLinearColor CurrentColor = ColorAndOpacity.GetSpecifiedColor();
	CurrentColor.A = InOpacity;

	SetColorAndOpacity(FSlateColor(CurrentColor));
}

void UHorizonDialogueMsgTextBlock::SetShadowColorAndOpacity(FLinearColor InShadowColorAndOpacity)
{
	ShadowColorAndOpacity = InShadowColorAndOpacity;
	if (bIsRichText) {
		//RebuildWidget();
	}
	else {
		for (auto& lineInfoIt : DialogueLineInfoList) {
			for (auto& blockInfo : lineInfoIt.DialogueBlockInfoList) {
				if (blockInfo.WidgetWeakPtr.IsValid() && blockInfo.WidgetWeakPtr->IsA<UHorizonTextBlock>()) {
					auto pTextBlock = Cast<UHorizonTextBlock>(blockInfo.WidgetWeakPtr.Get());
					pTextBlock->SetShadowColorAndOpacity(ShadowColorAndOpacity);
				}
			}
		}
	}
}

void UHorizonDialogueMsgTextBlock::SetShadowOffset(FVector2D InShadowOffset)
{
	ShadowOffset = InShadowOffset;
	if (bIsRichText) {
		//RebuildWidget();
	}
	else {
		for (auto& lineInfo : DialogueLineInfoList) {
			for (auto& blockInfo : lineInfo.DialogueBlockInfoList) {
				if (blockInfo.WidgetWeakPtr.IsValid() && blockInfo.WidgetWeakPtr->IsA<UHorizonTextBlock>()) {
					auto pTextBlock = Cast<UHorizonTextBlock>(blockInfo.WidgetWeakPtr.Get());
					pTextBlock->SetShadowOffset(ShadowOffset);
				}
			}
		}
	}
}

void UHorizonDialogueMsgTextBlock::SetFont(FSlateFontInfo InFontInfo)
{
	Font = InFontInfo;
	if (bIsRichText) {
		//RebuildWidget();
	}
	else {
		for (auto& lineInfo : DialogueLineInfoList) {
			for (auto& blockInfo : lineInfo.DialogueBlockInfoList) {
				if (blockInfo.WidgetWeakPtr.IsValid() && blockInfo.WidgetWeakPtr->IsA<UHorizonTextBlock>()) {
					auto pTextBlock = Cast<UHorizonTextBlock>(blockInfo.WidgetWeakPtr.Get());
					pTextBlock->SetFont(Font);
				}
		
			}
		}
	}
}

void  UHorizonDialogueMsgTextBlock::SetFontSize(int32 fontSize) {
	Font.Size = fontSize;
}

void UHorizonDialogueMsgTextBlock::SetJustification(ETextJustify::Type InJustification)
{
	Justification = InJustification;
	if (bIsRichText) {
		//RebuildWidget();
	}
	else {

		for (auto& lineInfo : DialogueLineInfoList) {
			for (auto& blockInfo : lineInfo.DialogueBlockInfoList) {
				if (blockInfo.WidgetWeakPtr.IsValid() && blockInfo.WidgetWeakPtr->IsA<UHorizonTextBlock>()) {
					auto pTextBlock = Cast<UHorizonTextBlock>(blockInfo.WidgetWeakPtr.Get());
					pTextBlock->SetJustification(Justification);
				}

			}
		}
	}
}

void UHorizonDialogueMsgTextBlock::RebuildBlockInfoJustification()
{
	for (auto& lineInfo : DialogueLineInfoList) {
		for (auto& blockInfo : lineInfo.DialogueBlockInfoList) {
			if (blockInfo.WidgetWeakPtr.IsValid()) {
				switch (Justification)
				{
				case ETextJustify::Left:
					break;
				case ETextJustify::Right:
				{
					auto widthOffset = MaxLineWidth - lineInfo.LineWidth;
					UCanvasPanelSlot* canvasPanelSlot = Cast<UCanvasPanelSlot>(blockInfo.WidgetWeakPtr->Slot);
					if (canvasPanelSlot)
					{
						auto currentPos = canvasPanelSlot->GetPosition();
						currentPos.X += widthOffset;
						canvasPanelSlot->SetPosition(currentPos);
					}
				}
				break;
				case ETextJustify::Center:
				{
					auto widthOffset = (MaxLineWidth - lineInfo.LineWidth) * 0.5f;
					UCanvasPanelSlot* canvasPanelSlot = Cast<UCanvasPanelSlot>(blockInfo.WidgetWeakPtr->Slot);
					if (canvasPanelSlot)
					{
						auto currentPos = canvasPanelSlot->GetPosition();
						currentPos.X += widthOffset;
						canvasPanelSlot->SetPosition(currentPos);
					}

				}
				break;
				}

			}

		}
	}

}
void UHorizonDialogueMsgTextBlock::RebuildDialogueMsgTextBlock() 
{
	TexLength = 0;
	RebuildSegmentInfoList();
	RebuildLineInfoList();
	RebuildPageInfoList();


	RebuildBlockInfoJustification();

	


	MsgDeltaTime = 0.0f;
	if (bIsDialogueMsgText) {
		SetIsStartTickDialogueMsg(true);
	}
	else {
		SetIsStartTickDialogueMsg(false);
	}

	CalculateAutoNextDialogueMsgPageInterval();

}

void UHorizonDialogueMsgTextBlock::CalculateAutoNextDialogueMsgPageInterval()
{
	int32 numPage = FMath::Max(1, DialoguePageInfoList.Num());
	AutoNextDialogueMsgPageInterval = AutoNextDialogueMsgPageIntervalRate * (GetTextLength() / numPage);
	AutoNextDialogueMsgPageInterval = FMath::Clamp(AutoNextDialogueMsgPageInterval, AutoNextDialogueMsgPageIntervalMin, AutoNextDialogueMsgPageIntervalMax);

}
void UHorizonDialogueMsgTextBlock::SetAutoNextDialogueMsgPageIntervalRate(float InAutoNextDialogueMsgPageIntervalRate)
{
	AutoNextDialogueMsgPageIntervalRate = InAutoNextDialogueMsgPageIntervalRate;
	CalculateAutoNextDialogueMsgPageInterval();
}




TSharedRef<SWidget> UHorizonDialogueMsgTextBlock::RebuildWidget() {
	return Super::RebuildWidget();
}
void UHorizonDialogueMsgTextBlock::OnWidgetRebuilt()
{
	Super::OnWidgetRebuilt();
	if (!bCreationFromPalette) {

		bNeedRebuildDialogueMsgText = true;
	}
	else {
		bCreationFromPalette = false;
	}
	//GetWorld()->ForceGarbageCollection(true);
}

void UHorizonDialogueMsgTextBlock::OnBindingChanged(const FName& Property)
{
	Super::OnBindingChanged(Property);

	
}

void UHorizonDialogueMsgTextBlock::SynchronizeProperties()
{

	Super::SynchronizeProperties();
	SetText(Text);
	SetColorAndOpacity(ColorAndOpacity);
	SetShadowColorAndOpacity(ShadowColorAndOpacity);
	SetShadowOffset(ShadowOffset);
	SetFont(Font);
	SetIsDialogueMsgText(bIsDialogueMsgText);
	SetIsStartTickDialogueMsg(bIsStartTickDialogueMsg);
	SetIsRepeatDialogueMsg(bIsRepeatDialogueMsg);
	SetDialogueMsgSpeed(DialogueMsgSpeed);
	SetIsRichText(bIsRichText);
	SetJustification(Justification);
	if (bIsRichText) {
		//OnWidgetRebuilt();
	}
}

FText UHorizonDialogueMsgTextBlock::GetText() const
{

	return Text;
}

void UHorizonDialogueMsgTextBlock::SetText(FText InText)
{
	Text = InText;
	//RebuildWidget();
}

void UHorizonDialogueMsgTextBlock::SetTextAndRebuildDialogue(const FText& InText)
{
	Text = InText;
	RebuildDialogueMsgTextBlock();

}
int32 UHorizonDialogueMsgTextBlock::GetTextLength()
{
	return TexLength;
}


int32 UHorizonDialogueMsgTextBlock::GetCurrentPageTextLength()
{
	int32 result = 0;
	auto& currentPageInfo = DialoguePageInfoList[CurrentPageIndex];

	for (int32 i = currentPageInfo.StartLineIndex; i < currentPageInfo.EndLineIndex; ++i)
	{
		FHorizonDialogueLineInfo& lineInfo = DialogueLineInfoList[i];


		for (auto& blockInfo : lineInfo.DialogueBlockInfoList)
		{
			result += blockInfo.MsgText.Len();
		}
	}

	return result;
}


FText UHorizonDialogueMsgTextBlock::GetPageTextByIndex(int32 PageIndex)
{
	FString result;
	if (PageIndex >= 0 && PageIndex < DialoguePageInfoList.Num())
	{
		auto& currentPageInfo = DialoguePageInfoList[PageIndex];

		for (int32 i = currentPageInfo.StartLineIndex; i < currentPageInfo.EndLineIndex; ++i)
		{
			FHorizonDialogueLineInfo& lineInfo = DialogueLineInfoList[i];


			for (auto& blockInfo : lineInfo.DialogueBlockInfoList)
			{
				result += blockInfo.MsgText;
			}
		}
	}

	return FText::FromString(result);

}


void UHorizonDialogueMsgTextBlock::SetDialogueMsgSpeed(float speed) {
	if (FMath::IsNearlyZero(speed))
	{
		speed = 0.000001f;
	}
	DialogueMsgSpeed = speed;
	if (bIsRichText) {
		//RebuildWidget();
	}
	else {
		for (auto& segInfoIt : DialogueSegmentInfoList) {
			segInfoIt.DialogueMsgSpeed = speed;
			
		}
	}
}


void UHorizonDialogueMsgTextBlock::SetIsRichText(bool b) { 
	bIsRichText = b; 
}

void UHorizonDialogueMsgTextBlock::LoadSound(FHorizonDialogueSegmentInfo& segInfo,
	FHorizonDialogueBlockInfo& blockInfo)
{
	
	USoundBase* pDialogueSound = nullptr;
	if (segInfo.DialogueSoundPath.IsSet())
	{
		pDialogueSound = UHorizonFileSystem::GetInstance()->LoadSound(segInfo.DialogueSoundPath.GetValue());
	}
	else
	{

		if (segInfo.StyleInfoReferenceIndex >= 0)
		{
			if (segInfo.SegmentStyleReferenceIndex >= 0) 
			{
				auto& segStyleList = StyleInfoList[segInfo.StyleInfoReferenceIndex]->SegmentStyleList;
				auto& segInfoStyle = segStyleList[segInfo.SegmentStyleReferenceIndex];
				pDialogueSound = segInfoStyle.DialogueSound.LoadSynchronous();
			}
		}
		else
		{
			if (segInfo.SegmentStyleReferenceIndex >= 0)
			{
				auto& segStyleList = SegmentStyleList;
				auto& segInfoStyle = segStyleList[segInfo.SegmentStyleReferenceIndex];
				pDialogueSound = segInfoStyle.DialogueSound.LoadSynchronous();
			}		
		}
	}
	blockInfo.DialogueSound = pDialogueSound;

}


FHorizonDialogueBlockInfo UHorizonDialogueMsgTextBlock::CreateDialogueTextBlockInfo(int32 segmentIndex, int32 startIndex, int32 endIndex, const FVector2D& pos) {
	FHorizonDialogueSegmentInfo& segInfo = DialogueSegmentInfoList[segmentIndex];
	FName name = *FString::Printf(TEXT("%s.%d.%d.%d"), *FString(GetName()), segmentIndex, startIndex, endIndex);
	
	int32 count = (endIndex - startIndex);
	FHorizonDialogueBlockInfo blockInfo;
	blockInfo.MsgText = segInfo.Text.Mid(startIndex, count);

	blockInfo.WidgetWeakPtr = NewObject<UHorizonTextBlock>(GetOuter());
	
	if (blockInfo.WidgetWeakPtr.IsValid()) {
		UHorizonTextBlock* pTextBlock = Cast<UHorizonTextBlock>(blockInfo.WidgetWeakPtr.Get());
		RemoveChild(pTextBlock);
		AddChild(pTextBlock);
		const TSharedRef< FSlateFontMeasure >& FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		blockInfo.BlockSize = FontMeasure->Measure(blockInfo.MsgText, segInfo.Font);

		pTextBlock->SetColorAndOpacity(segInfo.ColorAndOpacity);
		pTextBlock->SetShadowColorAndOpacity(segInfo.ShadowColorAndOpacity.GetSpecifiedColor());
		pTextBlock->SetShadowOffset(segInfo.ShadowOffset);
		pTextBlock->SetFont(segInfo.Font);
		pTextBlock->SetJustification(Justification);
		if (!bIsDialogueMsgText) {
			pTextBlock->SetText(FText::FromString(blockInfo.MsgText));
		}
		else {
			//info.TextBlock->SetText(FText::FromString(info.MsgText));
		}

		UCanvasPanelSlot* canvasPanelSlot = Cast<UCanvasPanelSlot>(pTextBlock->Slot);
		canvasPanelSlot->SetAutoSize(true);

		FVector2D paddingPos = pos + FVector2D(0, (int32)segInfo.PaddingMargin.Top);
		canvasPanelSlot->SetPosition(paddingPos);
		//UCanvasPanelSlot* pDialogueTextBlockCanvasPanelSlot = UHorizonWidgetFunctionLibrary::GetParentCanvasPanelSlot(this);
		//verifyf(pDialogueTextBlockCanvasPanelSlot, TEXT("oops! You need put HorizonDialogueMsgTextBlock under the hierarchy of CanvasPanel"));
		//canvasPanelSlot->SetSize(pDialogueTextBlockCanvasPanelSlot->GetSize());

		pTextBlock->SetVisibility(ESlateVisibility::HitTestInvisible); //should not have hit test in UMG editor

		AddBackgroundButton(segmentIndex, blockInfo, canvasPanelSlot, paddingPos);
	}


	//pTextBlock->SetJustification(segInfo)

	blockInfo.Name = name;

	blockInfo.SegmentReferenceIndex = segmentIndex;
	return blockInfo;
}


FHorizonDialogueBlockInfo UHorizonDialogueMsgTextBlock::CreateDialogueImageBlockInfo(int32 segmentIndex, const FVector2D& pos) {
	auto& segInfo = DialogueSegmentInfoList[segmentIndex];
	

	FName name = *FString::Printf(TEXT("%s.image%d"), *FString(GetName()), segmentIndex);

	FHorizonDialogueBlockInfo blockInfo;
	blockInfo.WidgetWeakPtr = NewObject<UHorizonImage>(GetOuter()); //NewObject<UHorizonImage>(this, name);
	if (blockInfo.WidgetWeakPtr.IsValid()){
		
		UHorizonImage* pImage = Cast<UHorizonImage>(blockInfo.WidgetWeakPtr.Get());
		pImage->UVRegion.bIsValid = false;
		AddChild(pImage);
		int32 width = 32;
		int32 height = 32;
		UTexture2D* pTexture = nullptr;
		UMaterial* pMaterial = nullptr;
		UPaperSprite* pSprite = nullptr;
		if (segInfo.FilePath.IsSet()) {
			FSoftObjectPath assetRef = segInfo.FilePath.GetValue();
			auto loadedObject = assetRef.TryLoad();
			pSprite = Cast<UPaperSprite>(loadedObject);
			pMaterial = Cast<UMaterial>(loadedObject);
			if (nullptr == pSprite && nullptr == pMaterial)
			{
				pTexture = UHorizonFileSystem::GetInstance()->LoadTexture2D(assetRef.ToString(), width, height);
			}
			

		}
		else {
			if (segInfo.StyleInfoReferenceIndex >= 0)
			{
				auto & segStyleList = StyleInfoList[segInfo.StyleInfoReferenceIndex]->SegmentStyleList;
				if (segInfo.SegmentStyleReferenceIndex >= 0)
				{
					auto& segInfoStyle = segStyleList[segInfo.SegmentStyleReferenceIndex];
					pTexture = segInfoStyle.Texture2D.LoadSynchronous();
					pMaterial = segInfoStyle.Material.LoadSynchronous();
					pSprite = segInfoStyle.PaperSprite.LoadSynchronous();
				}
			}
			else
			{
				auto& segStyleList = SegmentStyleList;
				if (segInfo.SegmentStyleReferenceIndex >= 0)
				{
					auto& segInfoStyle = segStyleList[segInfo.SegmentStyleReferenceIndex];
					pTexture = segInfoStyle.Texture2D.LoadSynchronous();
					pMaterial = segInfoStyle.Material.LoadSynchronous();
					pSprite = segInfoStyle.PaperSprite.LoadSynchronous();
				}
			}
			
		
		}



		if (pTexture) {
			pImage->SetBrushFromTexture(pTexture, true);
		}
		

		if (pMaterial) {
			pImage->SetBrushFromMaterial(pMaterial);
		}

		if (pSprite)
		{
			pImage->Brush.SetResourceObject(pSprite);
			
		}

		pImage->SynchronizeProperties();
		UCanvasPanelSlot* canvasPanelSlot = Cast<UCanvasPanelSlot>(pImage->Slot);
		if (segInfo.ImageSize.IsSet()) {
			canvasPanelSlot->SetSize(segInfo.ImageSize.GetValue());
		}
		FVector2D paddingPos = pos + FVector2D(0, (int)segInfo.PaddingMargin.Top);
		canvasPanelSlot->SetPosition(paddingPos);
		pImage->SetColorAndOpacity(segInfo.ColorAndOpacity.GetSpecifiedColor());
		if (bIsDialogueMsgText) {
			pImage->SetVisibility(ESlateVisibility::Hidden);
		}
		else {
			pImage->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		blockInfo.BlockSize = canvasPanelSlot->GetSize();
		AddBackgroundButton(segmentIndex, blockInfo, canvasPanelSlot, paddingPos);
		
	}


	
	
	blockInfo.Name = name;
	blockInfo.SegmentReferenceIndex = segmentIndex;
	return blockInfo;
}



FHorizonDialogueBlockInfo UHorizonDialogueMsgTextBlock::CreateDialoguePaperFlipbookBlockInfo(int32 segmentIndex, const FVector2D& pos) {
	auto& segInfo = DialogueSegmentInfoList[segmentIndex];
	
	
	
	FName name = *FString::Printf(TEXT("%s.PaperFlipbookWidget%d"), *FString(GetName()), segmentIndex);

	FHorizonDialogueBlockInfo blockInfo;
	blockInfo.WidgetWeakPtr = NewObject<UHorizonFlipbookWidget>(GetOuter()); //NewObject<UHorizonFlipbookWidget>(this, name);
	if (blockInfo.WidgetWeakPtr.IsValid()) {
		UHorizonFlipbookWidget* pWidget = Cast<UHorizonFlipbookWidget>(blockInfo.WidgetWeakPtr.Get());
		//FStringAssetReference assetRef = segInfo.ImageFilePath.GetValue();
		//UHorizonFileSystem::LoadPaperFlipbook
		UPaperFlipbook* flipbook = nullptr;
		TArray<FVector2D> PaperFlipbookSourceSize;
		TArray<FVector2D> PaperFlipbookSourceUV;

		if (segInfo.StyleInfoReferenceIndex >= 0)
		{
			if (segInfo.SegmentStyleReferenceIndex >= 0)
			{
				auto& segStyleList = StyleInfoList[segInfo.StyleInfoReferenceIndex]->SegmentStyleList;
				auto& segInfoStyle = segStyleList[segInfo.SegmentStyleReferenceIndex];
				flipbook = segInfoStyle.PaperFlipbook.LoadSynchronous();
				PaperFlipbookSourceSize = segInfoStyle.PaperFlipbookSourceSize;
				PaperFlipbookSourceUV = segInfoStyle.PaperFlipbookSourceUV;			
			}
		}
		else
		{
			if (segInfo.SegmentStyleReferenceIndex >= 0)
			{
				auto& segStyleList = SegmentStyleList;
				auto& segInfoStyle = segStyleList[segInfo.SegmentStyleReferenceIndex];
				flipbook = segInfoStyle.PaperFlipbook.LoadSynchronous();
				PaperFlipbookSourceSize = segInfoStyle.PaperFlipbookSourceSize;
				PaperFlipbookSourceUV = segInfoStyle.PaperFlipbookSourceUV;				
			}
		}



		if (segInfo.SegmentStyleReferenceIndex >= 0) 
		{
			if (flipbook) {
				AddChild(blockInfo.WidgetWeakPtr.Get());
				pWidget->SetFlipbook(flipbook);
				pWidget->SetFlipbookSourceSize(PaperFlipbookSourceSize);
				pWidget->SetFlipbookSourceUV(PaperFlipbookSourceUV);
				pWidget->SetColorAndOpacity(segInfo.ColorAndOpacity.GetSpecifiedColor());
				UCanvasPanelSlot* canvasPanelSlot = Cast<UCanvasPanelSlot>(pWidget->Slot);
				if (segInfo.ImageSize.IsSet()) {
					canvasPanelSlot->SetSize(segInfo.ImageSize.GetValue());
				}
				FVector2D paddingPos = pos + FVector2D(0, (int32)segInfo.PaddingMargin.Top);
				canvasPanelSlot->SetPosition(paddingPos);
				pWidget->SetColorAndOpacity(segInfo.ColorAndOpacity.GetSpecifiedColor());
				if (bIsDialogueMsgText) {
					pWidget->SetVisibility(ESlateVisibility::Hidden);
				}
				else {
					pWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
				}
				pWidget->SynchronizeProperties();
				blockInfo.BlockSize = canvasPanelSlot->GetSize();
				AddBackgroundButton(segmentIndex, blockInfo, canvasPanelSlot, paddingPos);
			}
		}
	}
	
	blockInfo.Name = name;
	blockInfo.SegmentReferenceIndex = segmentIndex;

	return blockInfo;
}





void UHorizonDialogueMsgTextBlock::AddDialogueBlock(FHorizonDialogueLineInfo& lineInfo, FHorizonDialogueBlockInfo&& blockInfo)
{
	auto& segInfo = DialogueSegmentInfoList[blockInfo.SegmentReferenceIndex];
	LoadSound(segInfo, blockInfo);

	UCanvasPanelSlot* canvasPanelSlot = Cast<UCanvasPanelSlot>(blockInfo.WidgetWeakPtr->Slot);
	verifyf(canvasPanelSlot, TEXT("oops! something error in UHorizonDialogueMsgTextBlock::AddDialogueBlock"));
	lineInfo.Position = FVector2D(LineMargin.Left, canvasPanelSlot->GetPosition().Y);
	TexLength += blockInfo.MsgText.Len();
	lineInfo.DialogueBlockInfoList.Emplace(Forward<FHorizonDialogueBlockInfo>(blockInfo));		
}


FHorizonDialogueSegmentInfo UHorizonDialogueMsgTextBlock::CreateDefaultSegmentInfo(EHorizonDialogueSegmentType segmentType) {
	FHorizonDialogueSegmentInfo info;
	info.TypeEnum = segmentType;
	info.ColorAndOpacity = ColorAndOpacity;
	info.DialogueMsgSpeed = DialogueMsgSpeed;
	info.DialogueMsgWait = 0.0f;
	info.Font = Font;
	info.ShadowColorAndOpacity = ShadowColorAndOpacity;
	info.ShadowOffset = ShadowOffset;
	return info;
}




void UHorizonDialogueMsgTextBlock::RebuildSegmentInfoListImplement(const FHorizonDialogueSegmentInfo& parentSegParam, const FXmlNode* pCurrentNode) {
	
	//depth first
	if (pCurrentNode && pCurrentNode->GetFirstChildNode()) {
		FHorizonDialogueSegmentInfo currentSegParam = CreateSegmentInfo(parentSegParam, pCurrentNode);
		RebuildSegmentInfoListImplement(currentSegParam, pCurrentNode->GetFirstChildNode());
	}

	if (pCurrentNode) {
		FHorizonDialogueSegmentInfo currentSegParam = CreateSegmentInfo(parentSegParam, pCurrentNode);

		//if (pCurrentNode->GetChildrenNodes().Num() == 0) {
			if (currentSegParam.TypeEnum == EHorizonDialogueSegmentType::Text) {
				//currentSegParam.
				currentSegParam.Text = pCurrentNode->GetContent();
				//Value.ReplaceInline(TEXT("&quot;"), TEXT("\""), ESearchCase::CaseSensitive);
				//Value.ReplaceInline(TEXT("&amp;"), TEXT("&"), ESearchCase::CaseSensitive);
				//Value.ReplaceInline(TEXT("&apos;"), TEXT("'"), ESearchCase::CaseSensitive);
				//Value.ReplaceInline(TEXT("&lt;"), TEXT("<"), ESearchCase::CaseSensitive);
				//Value.ReplaceInline(TEXT("&gt;"), TEXT(">"), ESearchCase::CaseSensitive);

				//handle special character
				currentSegParam.Text.ReplaceInline(TEXT("&nbsp;"), TEXT(" "));
				currentSegParam.Text.ReplaceInline(TEXT("&quot;"), TEXT("\""));
				currentSegParam.Text.ReplaceInline(TEXT("&amp;"), TEXT("&"));
				currentSegParam.Text.ReplaceInline(TEXT("&apos;"), TEXT("'"));
				currentSegParam.Text.ReplaceInline(TEXT("&lt;"), TEXT("<"));
				currentSegParam.Text.ReplaceInline(TEXT("&gt;"), TEXT(">"));
				//padding spcae
				if (currentSegParam.PaddingMargin.Left > 0) {
					currentSegParam.Text = currentSegParam.Text.LeftPad(currentSegParam.Text.Len() + (int32)currentSegParam.PaddingMargin.Left);
				}
				if ((int)currentSegParam.PaddingMargin.Right > 0) {
					currentSegParam.Text = currentSegParam.Text.RightPad(currentSegParam.Text.Len() + (int32)currentSegParam.PaddingMargin.Right);
				}
			}
			DialogueSegmentInfoList.Emplace(MoveTemp(currentSegParam));
		//}
		RebuildSegmentInfoListImplement(parentSegParam, pCurrentNode->GetNextNode());
	}


}

void UHorizonDialogueMsgTextBlock::AddTextSegmentInfo(const FString& text) {
	DialogueSegmentInfoList.Emplace(CreateDefaultSegmentInfo(EHorizonDialogueSegmentType::Text));
	FHorizonDialogueSegmentInfo& info = DialogueSegmentInfoList.Last();
	info.Text = text;
}
void UHorizonDialogueMsgTextBlock::RebuildSegmentInfoList() 
{

	//rebuild style info
	StyleInfoList.Empty();
	for (auto& styleInfoClassIt : StyleInfoClassList)
	{
		if (styleInfoClassIt)
		{
			auto styleInfo = NewObject<UHorizonDialogueStyleInfo>(this, styleInfoClassIt,
				NAME_None, EObjectFlags::RF_NoFlags,
				nullptr, true);
			if (styleInfo)
			{
				StyleInfoList.Emplace(styleInfo);
			}
		}
	}




	DialogueSegmentInfoList.Empty();//DialogueSegmentInfoList.RemoveAll([](const FHorizonDialogueSegmentInfo& element) {return true; });

	if (bIsRichText) {
		FString segmentStr = FString::Printf(TEXT("%s%s%s"), TEXT("<text>"), *Text.ToString(), TEXT("</text>"));
		FXmlFile * xmlFile = new FXmlFile(segmentStr, EConstructMethod::ConstructFromBuffer);

	
		//const char* AAA = R"(C:\AAAA\BBB)";
		if (xmlFile->IsValid()) {
			const FXmlNode* pRootNode = xmlFile->GetRootNode();
			FHorizonDialogueSegmentInfo rootSegParam = CreateSegmentInfo(CreateDefaultSegmentInfo(EHorizonDialogueSegmentType::Invalidated), pRootNode);
			if (pRootNode) {
				RebuildSegmentInfoListImplement(rootSegParam, pRootNode);	
			}
			else {
				AddTextSegmentInfo(Text.ToString());
				verifyf(false, TEXT("oops! something error, should never go here"));
			}
		}
		else {
			AddTextSegmentInfo(Text.ToString());
		}
	}
	else {
		AddTextSegmentInfo(Text.ToString());
	}
	


	//BuildSegmentInfoList(segmentStr);
	
}

void UHorizonDialogueMsgTextBlock::RebuildLineInfoList() 
{
	//clear up
	ClearChildren();
	DialogueLineInfoList.Empty();
	
	// setup line init info
	UCanvasPanelSlot* pTextBlockCanvasPanelSlot = UHorizonWidgetFunctionLibrary::GetParentCanvasPanelSlot(this);
	if (nullptr == pTextBlockCanvasPanelSlot)
	{
		FMessageLog HorizonUIPluginLog(GName_HorizonUIPlugin);
		auto message = FString::Printf(TEXT("oops! You need put HorizonDialogueMsgTextBlock under the hierarchy of CanvasPanel"));
		HorizonUIPluginLog.Warning(FText::FromString(message));
		HorizonUIPluginLog.Notify(FText::FromString("HorizonDialogueMsgTextBlock Can't find CanvasPanel"));
		//verifyf(pTextBlockCanvasPanelSlot, TEXT("oops! You need put HorizonDialogueMsgTextBlock under the hierarchy of CanvasPanel"));
		return;
	}
	
	const TSharedRef< FSlateFontMeasure >& FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

	//Hotfix: Because existing widget will not get correct Geometry, so use slot's size first
	float widgetWidth = pTextBlockCanvasPanelSlot->GetSize().X;
	auto pTextBlockCanvasPanelSlotAnchor = pTextBlockCanvasPanelSlot->GetAnchors();
	
	if (!FMath::IsNearlyEqual(pTextBlockCanvasPanelSlotAnchor.Maximum.X, pTextBlockCanvasPanelSlotAnchor.Minimum.X))
	{
		auto geometry = GetCachedGeometry();
		widgetWidth = geometry.Size.X;
	}

	MaxLineWidth = FMath::Max(Font.Size, (int32)(widgetWidth - (LineMargin.Left + LineMargin.Right)));
	float currentLineWidth = 0;

	DialogueLineInfoList.Emplace(FHorizonDialogueLineInfo());

	FVector2D blockPos(LineMargin.Left, 0);
	//int32 blockPosX = LineMargin.Left;
	//int32 blockPosY = 0;
	//parse segment into line and block
	//for (const auto& it : DialogueSegmentInfoList) {
	for(int32 currentSegInfoIndex = 0; currentSegInfoIndex < DialogueSegmentInfoList.Num(); ++currentSegInfoIndex){
		auto& segInfo = DialogueSegmentInfoList[currentSegInfoIndex];
		
		switch (segInfo.TypeEnum) {
			case EHorizonDialogueSegmentType::Text:
			{

				switch (TextOverFlowWarpMethod)
				{
					case EHorizonDialogueTextOverflowWarpMethod::Normal:
					{
						TextOverflowWarpNormal_Implement(currentLineWidth, blockPos, currentSegInfoIndex, MaxLineWidth, segInfo);
					}
					break;
					case EHorizonDialogueTextOverflowWarpMethod::BreakAll:
					{
						TextOverflowWarpBreakAll_Implement(currentLineWidth, blockPos, currentSegInfoIndex, MaxLineWidth, segInfo);
					}
					break;
				}
				
			}
			break;
			case EHorizonDialogueSegmentType::NewLine:
			{
				
				blockPos.X = LineMargin.Left;
				DialogueLineInfoList.Last().LineWidth = currentLineWidth;
				//new line
				auto lineInfo = FHorizonDialogueLineInfo();
				lineInfo.MaxLineHeight = Font.Size;
				lineInfo.Position.Y = DialogueLineInfoList.Last().Position.Y + DialogueLineInfoList.Last().MaxLineHeight + LineMargin.Bottom;
				DialogueLineInfoList.Emplace(lineInfo);
				currentLineWidth = 0;
				continue;
			}
			break;
			case EHorizonDialogueSegmentType::Image:
			case EHorizonDialogueSegmentType::PaperFlipbook:
			case EHorizonDialogueSegmentType::Material:
			{
				//first line only calculate from top
				if (DialogueLineInfoList.Num() == 1) {
					blockPos.Y = LineMargin.Top;
				}
				else {
					blockPos.Y = DialogueLineInfoList.Last(1).Position.Y + DialogueLineInfoList.Last(1).MaxLineHeight + LineMargin.Bottom;
				}

				//auto& blockInfo = segInfo.TypeEnum == EHorizonDialogueSegmentType::Image ? CreateDialogueImageBlockInfo(i, blockPos) : CreateDialoguePaperFlipbookBlockInfo(i, blockPos);
				FHorizonDialogueBlockInfo blockInfo;
				if (segInfo.TypeEnum == EHorizonDialogueSegmentType::Image ||
					segInfo.TypeEnum == EHorizonDialogueSegmentType::Material){
					blockInfo = CreateDialogueImageBlockInfo(currentSegInfoIndex, blockPos);
				}else if (segInfo.TypeEnum == EHorizonDialogueSegmentType::PaperFlipbook) {
					blockInfo = CreateDialoguePaperFlipbookBlockInfo(currentSegInfoIndex, blockPos);
				}else {
					verifyf(false, TEXT("oops! something error"));
				}

				if (blockInfo.WidgetWeakPtr.IsValid())
				{

					UCanvasPanelSlot* canvasPanelSlot = Cast<UCanvasPanelSlot>(blockInfo.WidgetWeakPtr->Slot);
					if (canvasPanelSlot) {
						FVector2D size = canvasPanelSlot->GetSize();
						currentLineWidth += size.X;


						//if image size larger than maxLineWidth, then move the image to new line
						if (currentLineWidth > MaxLineWidth) {
							//setup prev line width
							DialogueLineInfoList.Last().LineWidth = currentLineWidth - size.X;

							//add new line
							DialogueLineInfoList.Emplace(FHorizonDialogueLineInfo());
							DialogueLineInfoList.Last().LineWidth = size.X;
							currentLineWidth = size.X;

							//blockPos calculate from new line
							{
								blockPos.X = LineMargin.Left;
								//first line only calculate from top
								if (DialogueLineInfoList.Num() == 1) {
									blockPos.Y = LineMargin.Top;
								}
								else {
									blockPos.Y = DialogueLineInfoList.Last(1).Position.Y + DialogueLineInfoList.Last(1).MaxLineHeight + LineMargin.Bottom;
								}
							}

							FVector2D paddingPos = blockPos + FVector2D(0, (int32)segInfo.PaddingMargin.Top);
							canvasPanelSlot->SetPosition(paddingPos);
							AddBackgroundButton(currentSegInfoIndex, blockInfo, canvasPanelSlot, paddingPos);
						}
				
						DialogueLineInfoList.Last().MaxLineHeight = FMath::Max(DialogueLineInfoList.Last().MaxLineHeight, (int)(size.Y + segInfo.PaddingMargin.Top));
						AddDialogueBlock(DialogueLineInfoList.Last(), MoveTemp(blockInfo));
			
					}
				}
				
				

				blockPos.X = LineMargin.Left + currentLineWidth;

			}
			break;

			case EHorizonDialogueSegmentType::HyperText:
				break;

		};
	
		

	}
}

//void UHorizonDialogueMsgTextBlock::SkipDialogue()
//{
//	CurrentDialogueLineIndex = DialogueLineInfoList.Num();
//	for (auto& lineInfo : DialogueLineInfoList) {
//		lineInfo.CurrentDialogueBlockIndex = lineInfo.DialogueBlockInfoList.Num();
//		for (auto& blockInfo : lineInfo.DialogueBlockInfoList) {
//
//			if (blockInfo.WidgetWeakPtr.IsValid() && blockInfo.WidgetWeakPtr->IsA<UHorizonTextBlock>()) {
//				auto pTextBlock = Cast<UHorizonTextBlock>(blockInfo.WidgetWeakPtr.Get());
//				pTextBlock->SetText(FText::FromString(blockInfo.MsgText));
//			}
//		}
//	}
//}




void UHorizonDialogueMsgTextBlock::ResetDialogueMsgText()
{
	if (bIsDialogueMsgText) 
	{
		for (auto& lineInfoIt : DialogueLineInfoList) {
			lineInfoIt.CurrentDialogueBlockIndex = 0;
			for (auto& blockInfoIt : lineInfoIt.DialogueBlockInfoList) {
				blockInfoIt.CurrentCharIndex = 0;
				blockInfoIt.bDialogueSoundPlayed = false;
				if (blockInfoIt.WidgetWeakPtr.IsValid()) 
				{
					blockInfoIt.WidgetWeakPtr->SetVisibility(ESlateVisibility::Hidden);			
				}
				if (blockInfoIt.WidgetBackgroundWeakPtr.IsValid())
				{
					blockInfoIt.WidgetBackgroundWeakPtr->SetVisibility(ESlateVisibility::Hidden);
				}
			}
		}
		//SetIsStartTickDialogue(true);
		CurrentDialogueLineIndex = 0;
		CurrentPageIndex = 0;
		CurrentPageHeightOffset = 0.0f;
		MsgDeltaTime = 0.0f;
	}
}

void UHorizonDialogueMsgTextBlock::SetIsDialogueMsgText(bool b) {
	bIsDialogueMsgText = b;
	if (!bIsDialogueMsgText) {
		for (auto& lineInfo : DialogueLineInfoList) {
			for (auto& blockInfo : lineInfo.DialogueBlockInfoList) {
				if (blockInfo.WidgetWeakPtr.IsValid() && blockInfo.WidgetWeakPtr->IsA<UHorizonTextBlock>()) {
					auto pTextBlock = Cast<UHorizonTextBlock>(blockInfo.WidgetWeakPtr.Get());
					pTextBlock->SetText(FText::FromString(blockInfo.MsgText));
				}
				
			}
		}
	}
}

void UHorizonDialogueMsgTextBlock::SetIsStartTickDialogueMsg(bool b) {
	bIsStartTickDialogueMsg = b;
	if (bIsStartTickDialogueMsg){
		ResetDialogueMsgText();
	}
}

void UHorizonDialogueMsgTextBlock::SetIsRepeatDialogueMsg(bool b) {
	bIsRepeatDialogueMsg = b;
}


void UHorizonDialogueMsgTextBlock::SetRepeatDialogueMsgInterval(float interval) {
	RepeatDialogueMsgInterval = interval;
}



void UHorizonDialogueMsgTextBlock::StopDialogue()
{
	//reset dialogue
	ResetDialogueMsgText();
	//stop tick
	SetIsStartTickDialogueMsg(false);
}

void UHorizonDialogueMsgTextBlock::StartDialogue()
{
	SetIsStartTickDialogueMsg(true);
};



bool UHorizonDialogueMsgTextBlock::IsDialogueMsgCompleted()
{
	if (IsDialogueMsgText())
	{
		return CurrentDialogueLineIndex >= DialogueLineInfoList.Num();
	}
	else
	{
		return true;
	}
}

#undef LOCTEXT_NAMESPACE

// Created by dorgon, All Rights Reserved.
// email: dorgonman@hotmail.com
// blog: dorgon.horizon-studio.net


#include "Widget/HorizonDialogueMsgTextBlock.h"
#include "Widget/HorizonFlipbookWidget.h"
#include "Widget/HorizonWidgetFunctionLibrary.h"
#include "FileSystem/HorizonFileSystem.h"

//UMG
#include "UMGStyle.h"

//#include "Regex.h"
#include "XmlParser.h"


//engine
#include "PaperFlipbook.h"


#if WITH_EDITOR

FString UHorizonDialogueMsgTextBlock::GetLabelMetadata() const
{
	const int32 MaxSampleLength = 15;

	FString TextStr = Text.ToString();
	TextStr = TextStr.Len() <= MaxSampleLength ? TextStr : TextStr.Left(MaxSampleLength - 2) + TEXT("..");
	return TEXT(" \"") + TextStr + TEXT("\"");
}

void UHorizonDialogueMsgTextBlock::HandleTextCommitted(const FText& InText, ETextCommit::Type CommitteType)
{
	//TODO UMG How will this migrate to the template?  Seems to me we need the previews to have access to their templates!
	//TODO UMG How will the user click the editable area?  There is an overlay blocking input so that other widgets don't get them.
	//     Need a way to recognize one particular widget and forward things to them!
}


//#if (ENGINE_MAJOR_VERSION <= 4) && (ENGINE_MINOR_VERSION <= 12)
/*const FSlateBrush* UHorizonDialogueMsgTextBlock::GetEditorIcon()
{
	return FUMGStyle::Get().GetBrush("Widget.TextBlock");
}*/
//#endif //#if (ENGINE_MAJOR_VERSION <= 4) && (ENGINE_MINOR_VERSION <= 12)

const FText UHorizonDialogueMsgTextBlock::GetPaletteCategory()
{
	return NSLOCTEXT("HorizonPlugin", "HorizonPlugin", "HorizonPlugin");
}

void UHorizonDialogueMsgTextBlock::OnCreationFromPalette()
{
	Text = FText::FromString("Dialogue Text Block");
	//UE_HRIZONLOG_LOG("");
	bCreationFromPalette = true;
}


void UHorizonDialogueMsgTextBlock::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) {

	if (PropertyChangedEvent.Property)
	{
		auto nameCPP = PropertyChangedEvent.Property->GetNameCPP();
		// FIX crash
		if (nameCPP != GET_MEMBER_NAME_CHECKED(UHorizonDialogueMsgTextBlock, SegmentStyleList).ToString() &&
			nameCPP != TEXT("SpecifiedColor"))
		{
			Super::PostEditChangeProperty(PropertyChangedEvent);
		}
	}

	//


}

void UHorizonDialogueMsgTextBlock::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	auto pHeadNode = PropertyChangedEvent.PropertyChain.GetHead();
	if (pHeadNode)
	{
		UProperty* pHead = pHeadNode->GetValue();
		//auto propertyCppType = pHead->GetCPPType();
		//auto prevNode = PropertyChangedEvent.PropertyChain.GetTail()->GetPrevNode();
		auto nameCPP = pHead->GetNameCPP();
		//auto fieldCppType = TEXT("F") + FHorizonDialogueSegmentInfoStyle::StaticStruct()->GetName();
		if (nameCPP == GET_MEMBER_NAME_CHECKED(UHorizonDialogueMsgTextBlock, SegmentStyleList).ToString())
		{
			auto pStructList = pHead->ContainerPtrToValuePtr<TArray<FHorizonDialogueSegmentInfoStyle>>(this);
			FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
			if (PropertyName == GET_MEMBER_NAME_CHECKED(FHorizonDialogueSegmentInfoStyle, PaperFlipbook))
			{
				// rebuild sourceUV and sourceSize for HorizonFlipbookWidget
				for (int32 i = 0; i < pStructList->Num(); i++) {
					auto& pStruct = (*pStructList)[i];
					UHorizonWidgetFunctionLibrary::RebuildFlipbook(pStruct.PaperFlipbook.LoadSynchronous(), pStruct.PaperFlipbookSourceUV, pStruct.PaperFlipbookSourceSize);
				}

			}
		}
	}
}


#endif //#if WITH_EDITOR


// Created by dorgon, All Rights Reserved.
// email: dorgonman@hotmail.com
// blog: dorgon.horizon-studio.net
#include "Widget/HorizonDialogueMsgTextBlock.h"
#include "HorizonUIPrivate.h"
#include "FileSystem/HorizonFileSystem.h"
#include "XmlNode.h"


namespace horizonui
{
	static bool ApplySegmentStyle(FHorizonDialogueSegmentInfo& segParam,
		const FString& segName,
		const TArray<FHorizonDialogueSegmentInfoStyle>& segmentStyleList)
	{
		bool bStyleFound = false;
		for (int32 i = 0; i < segmentStyleList.Num(); ++i) {
			const auto& it = segmentStyleList[i];
			if (segName == it.StyleName.ToString()) {
				segParam.SegmentStyleReferenceIndex = i;
				bStyleFound = true;
				if (it.ColorAndOpacity.Num() > 0) {
					segParam.ColorAndOpacity = it.ColorAndOpacity.Last();
				}

				if (it.DialogueMsgSpeed.Num() > 0) {
					segParam.DialogueMsgSpeed = it.DialogueMsgSpeed.Last();
				}
				if (it.DialogueMsgWait.Num() > 0) {
					segParam.DialogueMsgWait = it.DialogueMsgWait.Last();
				}
				if (it.ShadowOffset.Num() > 0) {
					segParam.ShadowOffset = it.ShadowOffset.Last();
				}

				if (it.ShadowColorAndOpacity.Num() > 0) {
					segParam.ShadowColorAndOpacity = it.ShadowColorAndOpacity.Last();
				}
				if (it.Font.Num() > 0) {
					segParam.Font = it.Font.Last();
				}

				if (it.FontSize.Num() > 0) {
					segParam.Font.Size = it.FontSize.Last();
				}
				if (it.TypefaceFontName.Num() > 0) {
					segParam.Font.TypefaceFontName = it.TypefaceFontName.Last();
				}

				if (it.PaddingMargin.Num() > 0) {
					segParam.PaddingMargin = it.PaddingMargin.Last();
				}

				if (it.ImageSize.Num() > 0) {
					segParam.ImageSize = it.ImageSize.Last();
				}

				if (it.HypertextReference.Num() > 0) {
					segParam.HypertextReference = it.HypertextReference.Last();
				}

				if (it.HypertextHoveredColor.Num() > 0) {
					segParam.HypertextHoveredColor = it.HypertextHoveredColor.Last();
				}

				if (it.HypertextVisitedColor.Num() > 0) {
					segParam.HypertextVisitedColor = it.HypertextVisitedColor.Last();
				}

				if (it.BackgroundButtonClass.Num() > 0) {
					segParam.BackgroundButtonClass = it.BackgroundButtonClass.Last();
				}

				if (it.DialogueSoundVolumeMultiplier.Num() > 0) {
					segParam.DialogueSoundVolumeMultiplier = it.DialogueSoundVolumeMultiplier.Last();
				}
				if (it.DialogueSoundPitchMultiplier.Num() > 0) {
					segParam.DialogueSoundPitchMultiplier = it.DialogueSoundPitchMultiplier.Last();
				}
				if (it.DialogueSoundStartTime.Num() > 0) {
					segParam.DialogueSoundStartTime = it.DialogueSoundStartTime.Last();
				}

				break;
			}
		}
		return bStyleFound;
	}
};

FHorizonDialogueSegmentInfo UHorizonDialogueMsgTextBlock::CreateSegmentInfo(const FHorizonDialogueSegmentInfo& parentSegParam, const FXmlNode* pXmlNode) {
	
	FHorizonDialogueSegmentInfo segParam = parentSegParam;
	if (pXmlNode->GetTag().Equals(TEXT("text"), ESearchCase::IgnoreCase)) {
		segParam.TypeEnum = EHorizonDialogueSegmentType::Text;
	}
	if(pXmlNode->GetTag().Equals(TEXT("img"), ESearchCase::IgnoreCase)){
		segParam.TypeEnum = EHorizonDialogueSegmentType::Image;
	}

	if (pXmlNode->GetTag().Equals(TEXT("br"), ESearchCase::IgnoreCase)) {
		segParam.TypeEnum = EHorizonDialogueSegmentType::NewLine;
	}

	if (pXmlNode->GetTag().Equals(TEXT("PaperFlipBook"), ESearchCase::IgnoreCase) || 
		pXmlNode->GetTag().Equals(TEXT("PFB"), ESearchCase::IgnoreCase)) {
		segParam.TypeEnum = EHorizonDialogueSegmentType::PaperFlipbook;
	}

	if (pXmlNode->GetTag().Equals(TEXT("mat"), ESearchCase::IgnoreCase)) {
		segParam.TypeEnum = EHorizonDialogueSegmentType::Material;
	}


	if (pXmlNode->GetTag().Equals(TEXT("a"), ESearchCase::IgnoreCase)) {
		segParam.TypeEnum = EHorizonDialogueSegmentType::HyperText;
	}

	const auto& attributeList = pXmlNode->GetAttributes();
	for (const auto& attrIt : attributeList) {
		const FString tag = attrIt.GetTag();
		FString value = attrIt.GetValue();

		//apply style first, and then replace with
		if (tag.Equals(TEXT("style"), ESearchCase::IgnoreCase)) {
			bool bStyleFound = horizonui::ApplySegmentStyle(segParam, value, SegmentStyleList);


			if (!bStyleFound)
			{
				for (int32 i = 0; i < StyleInfoList.Num(); ++i)
				{
					auto& styleInfo = StyleInfoList[i];
					//if (styleInfo)
					{
						bStyleFound = horizonui::ApplySegmentStyle(segParam, value, styleInfo->SegmentStyleList);
						if (bStyleFound)
						{
							segParam.StyleInfoReferenceIndex = i; // current styleInfo index
							break;
						}
					}

				}
			}


			if (!bStyleFound)
			{
				FMessageLog HorizonUIPluginLog(GName_HorizonUIPlugin);
				auto message = FString::Printf(TEXT("%s can't find style name: %s"), *GetName(), *value);
				HorizonUIPluginLog.Warning(FText::FromString(message));
				HorizonUIPluginLog.Notify(FText::FromString(message));
				//ensureMsgf(bStyleFound, TEXT("can't find style in SegmentStyleList: %s"), *value);
			}
		}


		if (tag.Equals(TEXT("color"), ESearchCase::IgnoreCase)) {
			value.ReplaceInline(TEXT("0x"), TEXT("#"), ESearchCase::IgnoreCase);
			FColor color = FColor::FromHex(value);
			float colorBase = (float)255.0f;
			segParam.ColorAndOpacity = FLinearColor(color.R/colorBase, color.G/colorBase, color.B/colorBase, color.A/colorBase);
		}

		if (tag.Equals(TEXT("speed"), ESearchCase::IgnoreCase)) {
			if (value.IsNumeric()) {
				segParam.DialogueMsgSpeed = FCString::Atof(*value);
			}
		}

		if (tag.Equals(TEXT("sound"), ESearchCase::IgnoreCase)) 
		{
			segParam.DialogueSoundPath = value;
		}

		if (tag.Equals(TEXT("soundVolume"), ESearchCase::IgnoreCase))
		{
			if (value.IsNumeric()) 
			{
				segParam.DialogueSoundVolumeMultiplier = FCString::Atof(*value);
			}
		}


		if (tag.Equals(TEXT("soundPitch"), ESearchCase::IgnoreCase))
		{
			if (value.IsNumeric())
			{
				segParam.DialogueSoundPitchMultiplier = FCString::Atof(*value);
			}
		}


		if (tag.Equals(TEXT("soundStartTime"), ESearchCase::IgnoreCase))
		{
			if (value.IsNumeric())
			{
				segParam.DialogueSoundStartTime = FCString::Atof(*value);
			}
		}



		if (tag.Equals(TEXT("wait"), ESearchCase::IgnoreCase)) {
			if (value.IsNumeric()) {
				segParam.DialogueMsgWait = FCString::Atof(*value);
			}
		}



		if (tag.Equals(TEXT("shadowColor"), ESearchCase::IgnoreCase) ||
			tag.Equals(TEXT("shadow_color"), ESearchCase::IgnoreCase)) {
			value.ReplaceInline(TEXT("0x"), TEXT("#"), ESearchCase::IgnoreCase);
			FColor color = FColor::FromHex(value);
			float colorBase = (float)255.0f;
			segParam.ShadowColorAndOpacity = FLinearColor(color.R / colorBase, color.G / colorBase, color.B / colorBase, color.A / colorBase);
		}

		//	FVector2D ShadowOffset;
		if (tag.Equals(TEXT("shadowOffset"), ESearchCase::IgnoreCase)||
			tag.Equals(TEXT("shadow_offset"), ESearchCase::IgnoreCase)) {
			value.ReplaceInline(TEXT("("), TEXT(" "));
			value.ReplaceInline(TEXT("{"), TEXT(" "));
			value.ReplaceInline(TEXT("["), TEXT(" "));
			value.ReplaceInline(TEXT(")"), TEXT(" "));
			value.ReplaceInline(TEXT("}"), TEXT(" "));
			value.ReplaceInline(TEXT("]"), TEXT(" "));
			value.TrimStartAndEndInline();
			//value.Trim();
			//value.TrimTrailing();
			TArray<FString> parse;

			value.ParseIntoArray(parse, TEXT(","), true);
			if (parse.Num() >= 2) {
				segParam.ShadowOffset = FVector2D(FCString::Atof(*parse[0]), FCString::Atof(*parse[1]));
			}
		}


		if (tag.Equals(TEXT("fontPath"), ESearchCase::IgnoreCase) ||
			tag.Equals(TEXT("font_path"), ESearchCase::IgnoreCase)) {
		
			//ConstructorHelpers::FObjectFinder<UFont> RobotoFontObj(*value);
			UFont* font = UHorizonFileSystem::GetInstance()->LoadFont(*value);
			segParam.Font = FSlateFontInfo(font, Font.Size, Font.TypefaceFontName);
			//segParam.Font.TypefaceFontName 
		}

		if (tag.Equals(TEXT("fontType"), ESearchCase::IgnoreCase) ||
			tag.Equals(TEXT("font_type"), ESearchCase::IgnoreCase)) {
				segParam.Font.TypefaceFontName = *value;
		}

		if (tag.Equals(TEXT("fontSize"), ESearchCase::IgnoreCase) ||
			tag.Equals(TEXT("font_size"), ESearchCase::IgnoreCase)) {
			if (value.IsNumeric()) {
				segParam.Font.Size = FCString::Atoi(*value);
			}
		}

		if (tag.Equals(TEXT("padding-left"), ESearchCase::IgnoreCase)) {
			if (value.IsNumeric()) {
				segParam.PaddingMargin.Left = FCString::Atof(*value);
			}
		}

		if (tag.Equals(TEXT("padding-right"), ESearchCase::IgnoreCase)) {
			if (value.IsNumeric()) {
				segParam.PaddingMargin.Right = FCString::Atof(*value);
			}
		}

		if (tag.Equals(TEXT("padding-top"), ESearchCase::IgnoreCase)) {
			if (value.IsNumeric()) {
				segParam.PaddingMargin.Top = FCString::Atof(*value);
			}
		}


		if (tag.Equals(TEXT("filePath"), ESearchCase::IgnoreCase) || 
			tag.Equals(TEXT("file_path"), ESearchCase::IgnoreCase)) {

			segParam.FilePath = value;
		}

		if (tag.Equals(TEXT("size"), ESearchCase::IgnoreCase)) {
			value.ReplaceInline(TEXT("("), TEXT(" "));
			value.ReplaceInline(TEXT("{"), TEXT(" "));
			value.ReplaceInline(TEXT("["), TEXT(" "));
			value.ReplaceInline(TEXT(")"), TEXT(" "));
			value.ReplaceInline(TEXT("}"), TEXT(" "));
			value.ReplaceInline(TEXT("]"), TEXT(" "));
			value.TrimStartAndEndInline();
			//value.Trim();
			//value.TrimTrailing();
			TArray<FString> parse;

			value.ParseIntoArray(parse, TEXT(","), true);
			if (parse.Num() >= 2) {
				segParam.ImageSize = FVector2D(FCString::Atof(*parse[0]), FCString::Atof(*parse[1]));
			}
		}


		if (tag.Equals(TEXT("href"), ESearchCase::IgnoreCase)) 
		{

			segParam.HypertextReference = value;
		}

		if (tag.Equals(TEXT("hoveredColor"), ESearchCase::IgnoreCase))
		{
			value.ReplaceInline(TEXT("0x"), TEXT("#"), ESearchCase::IgnoreCase);
			FColor color = FColor::FromHex(value);
			float colorBase = (float)255.0f;
			segParam.HypertextHoveredColor = FLinearColor(color.R / colorBase, color.G / colorBase, color.B / colorBase, color.A / colorBase);
		}

		if (tag.Equals(TEXT("visitedColor"), ESearchCase::IgnoreCase))
		{
			value.ReplaceInline(TEXT("0x"), TEXT("#"), ESearchCase::IgnoreCase);
			FColor color = FColor::FromHex(value);
			float colorBase = (float)255.0f;
			segParam.HypertextVisitedColor = FLinearColor(color.R / colorBase, color.G / colorBase, color.B / colorBase, color.A / colorBase);
		}
	}

	return segParam;
}
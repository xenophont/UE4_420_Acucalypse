// Crated by dorgon, All Rights Reserved.
// email: dorgonman@hotmail.com
// blog: dorgon.horizon-studio.net

#include "Widget/HorizonWidgetFunctionLibrary.h"


// Engine
#include "PaperFlipbook.h"
#include "PaperFlipbookComponent.h"
#include "PaperSprite.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetStringLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"

UCanvasPanelSlot* UHorizonWidgetFunctionLibrary::GetParentCanvasPanelSlot(UWidget* pWidget){
	UCanvasPanelSlot* pPanelSlot = nullptr;
	//try to find out UCanvasPanelSlot, search from current widget
	UWidget* pTargetWidget = pWidget;
	if (pTargetWidget) {
		do {

			pPanelSlot = Cast<UCanvasPanelSlot>(pTargetWidget->Slot);
			if (pPanelSlot) {
				break;
			}
			else {
				if (pTargetWidget) {
					pTargetWidget = pTargetWidget->GetParent();
					//verifyf(pTargetWidget, TEXT("oops! something error"));
				}
				else {
					break;
				}
			}

		} while (pPanelSlot == nullptr && pTargetWidget != nullptr);
	}

	return pPanelSlot;

}

#if WITH_EDITOR

void UHorizonWidgetFunctionLibrary::RebuildFlipbook(const UPaperFlipbook* PaperFlipbook, 
	TArray<FVector2D>& OutSourceUV, 
	TArray<FVector2D>& OutSourceSize)
{
	OutSourceUV.Empty();
	OutSourceSize.Empty();

	if (PaperFlipbook) {
		int32 numFrame = PaperFlipbook->GetNumFrames();
		int32 numKeyFrame = PaperFlipbook->GetNumKeyFrames();
		for (int32 i = 0; i < numKeyFrame; i++) {
			auto spriter = PaperFlipbook->GetSpriteAtFrame(i);
			if (spriter) {
				OutSourceUV.Emplace(spriter->GetSourceUV());
				OutSourceSize.Emplace(spriter->GetSourceSize());
	
			}

		}
	}
}
#endif //#if WITH_EDITOR


UWidgetAnimation* UHorizonWidgetFunctionLibrary::GetUserWidgetAnimation(UUserWidget* pUserWidget,
	const FName& animeName) 
{
	UWidgetBlueprintGeneratedClass* pBGClass = Cast<UWidgetBlueprintGeneratedClass>(pUserWidget->GetClass());
	return GetUserWidgetAnimation(pBGClass, animeName);
}

UWidgetAnimation* UHorizonWidgetFunctionLibrary::GetUserWidgetAnimation(
	UWidgetBlueprintGeneratedClass* pBGClass, 
	const FName& animeName) 
{
	if (pBGClass) {
		for (UWidgetAnimation* Animation : pBGClass->Animations)
		{
			if (Animation->MovieScene)
			{
				if (animeName.IsEqual(Animation->MovieScene->GetFName())) {
					return Animation;
				}
			}
		}
	}
	return nullptr;

}

int32 UHorizonWidgetFunctionLibrary::FindCharIndexFromStr(int32 startIndex , TCHAR aChar, const FString& str)
{
	TArray<TCHAR> CharList;
	CharList.Emplace(aChar);
	int32 result = UHorizonWidgetFunctionLibrary::FindCharIndexFromStr(startIndex, CharList, str);


	return result;

}


int32 UHorizonWidgetFunctionLibrary::FindCharIndexFromStr(int32 startIndex, const TArray<TCHAR>& charList, const FString& str)
{
	int32 result = str.Len();
	int32 currentIndex = startIndex;
	while (currentIndex < str.Len()) {
		TCHAR currentChar = str[currentIndex];
		
		if(charList.Contains(currentChar))
		{
			result = currentIndex;
			break;
		}
	
		++currentIndex;
	};


	return result;

}


FString UHorizonWidgetFunctionLibrary::NormalizeBlueprintClassFilePath(const FString& filePath)
{
	FString result = filePath;
	//Blueprint'/Game/UMG/DialogueMsgTextTest/ButtonStyle/BP_DialogueBackgroundButtonStyle1.BP_DialogueBackgroundButtonStyle1'
	int32 lastFindCharIndex = -1;
	result.FindChar('\'', lastFindCharIndex);
	if (lastFindCharIndex != -1)
	{
		result = UKismetStringLibrary::GetSubstring(result, lastFindCharIndex + 1, filePath.Len() - lastFindCharIndex);
	}
	
	result = result.Replace(TEXT("'"), TEXT(""));


	result.FindLastChar('/', lastFindCharIndex);
	if (lastFindCharIndex != -1)
	{
		auto folderPath = UKismetStringLibrary::GetSubstring(result, 0, lastFindCharIndex + 1);
		auto fileName = UKismetStringLibrary::GetSubstring(result, lastFindCharIndex + 1, filePath.Len() - lastFindCharIndex);
		auto bIsClassSuffix = UKismetStringLibrary::GetSubstring(fileName, fileName.Len() - 2, 2) == TEXT("_C");
		if (bIsClassSuffix)
		{
			fileName = UKismetStringLibrary::GetSubstring(fileName, 0, fileName.Len() - 2);
		}

		fileName.FindLastChar('.', lastFindCharIndex);
		if (lastFindCharIndex == -1)
		{
			fileName = FString::Printf(TEXT("%s.%s"), *fileName, *fileName);
		}



		result = FString::Printf(TEXT("%s%s_C"), *folderPath, *fileName);
	}

	return result;

}




UWidget* UHorizonWidgetFunctionLibrary::GetWidgetFromNameRecursively(UUserWidget* pUserWidget, const FName& InWidgetName)
{
	UWidget* pResult = nullptr;
	if (pUserWidget && pUserWidget->WidgetTree)
	{
		pUserWidget->WidgetTree->ForEachWidget(
			[&](UWidget* InWidget)
		{
			if (InWidget->GetFName() == InWidgetName)
			{
				pResult = InWidget;
			}

			if (nullptr == pResult)
			{
				auto pChildUserWidget = Cast<UUserWidget>(InWidget);
				if (pChildUserWidget)
				{
					pResult = GetWidgetFromNameRecursively(pChildUserWidget, InWidgetName);
				}
			}
		});

	}
	return pResult;
}


UWidget* UHorizonWidgetFunctionLibrary::SetWidgetVisibility(
	UUserWidget* UserWidget,
	FName WidgetName, ESlateVisibility SlateVisiblity)
{
	auto pWidget = GetWidgetFromNameRecursively(UserWidget, WidgetName);
	if (pWidget)
	{
		pWidget->SetVisibility(SlateVisiblity);
	}
	else
	{
		ensureMsgf(false, TEXT("can't find Widget: %s"), *WidgetName.ToString());
	}

	return pWidget;
}
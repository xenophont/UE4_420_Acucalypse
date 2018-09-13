// Crated by dorgon, All Rights Reserved.
// email: dorgonman@hotmail.com
// blog: dorgon.horizon-studio.net

#pragma once
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Animation/WidgetAnimation.h"
#include "MovieScene.h"

#include "Widget/HorizonDialogueMsgTextBlock.h"
#include "Widget/HorizonDialogueStyleInfo.h"

#include "HorizonWidgetFunctionLibrary.generated.h"

class UPaperFlipbook;
class UUserWidget;
UCLASS()
class HORIZONUI_API UHorizonWidgetFunctionLibrary : public UBlueprintFunctionLibrary {
private:
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|WidgetHelper")
		static UCanvasPanelSlot* GetParentCanvasPanelSlot(UWidget* pWidget);


#if WITH_EDITOR
	//UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|WidgetHelper")
		static void RebuildFlipbook(const UPaperFlipbook* PaperFlipbook, 
			TArray<FVector2D>& OutSourceUV, 
			TArray<FVector2D>& OutSourceSize);
#endif //#if WITH_EDITOR


	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|WidgetHelper")
	static UWidgetAnimation* GetUserWidgetAnimation(UUserWidget* pUserWidget, 
		const FName& animeName);


	static UWidgetAnimation* GetUserWidgetAnimation(
		UWidgetBlueprintGeneratedClass* pBGClass,
		const FName& animeName);


	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|WidgetHelper")
	static UWidget* GetWidgetFromNameRecursively(UUserWidget* pUserWidget, const FName& InWidgetName);


	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|WidgetHelper")
	static UWidget* SetWidgetVisibility(UUserWidget* UserWidget, FName WidgetName, UPARAM(DisplayName = "SlateVisibility") ESlateVisibility eVisiblity);


	static int32 FindCharIndexFromStr(int32 startIndex, TCHAR aChar, const FString& str);
	static int32 FindCharIndexFromStr(int32 startIndex, const TArray<TCHAR>& charList, const FString& str);


	static FString NormalizeBlueprintClassFilePath(const FString& filePath);
};
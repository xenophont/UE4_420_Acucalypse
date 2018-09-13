// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Styling/SlateColor.h"
#include "Fonts/SlateFontInfo.h"
#include "Layout/Margin.h"
#include "HorizonDialogueStyleInfo.generated.h"

class FXmlNode;
class UHorizonFlipbookWidget;
class UPaperFlipbook;
class UTexture2D;
class UMaterial;
class UPaperSprite;
class UHorizonButton;
class USoundBase;

USTRUCT(BlueprintType)
struct FHorizonDialogueSegmentInfoStyle
{

	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HorizonPlugin|Style|Basic")
		FName StyleName;

	//Optional parameters. If not be set, will use parent's setting(or default)
	//Because TOptional can't be acceee from blueprint, so use TArray instead, will use TArray::Last() if num of array larger than 0
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HorizonPlugin|Style|Text")
		TArray<FString> Text;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HorizonPlugin|Style|Widget")
		TArray<FSlateColor> ColorAndOpacity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HorizonPlugin|Style|Font")
		TArray<FSlateFontInfo> Font;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HorizonPlugin|Style|Font")
		TArray<int> FontSize;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HorizonPlugin|Style|Font")
		TArray<FName> TypefaceFontName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HorizonPlugin|Style|Text")
		TArray<FVector2D> ShadowOffset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HorizonPlugin|Style|Text")
		TArray<FSlateColor> ShadowColorAndOpacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HorizonPlugin|Style|Text")
		TArray<float> DialogueMsgSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HorizonPlugin|Style|Text")
		TArray<float> DialogueMsgWait;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HorizonPlugin|Style|Layout")
		TArray<FMargin> PaddingMargin;//padding position for segment

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HorizonPlugin|Style|HyperText")
		TArray<FString> HypertextReference;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HorizonPlugin|Style|HyperText")
		TArray<FSlateColor> HypertextHoveredColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HorizonPlugin|Style|HyperText")
		TArray<FSlateColor> HypertextVisitedColor;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HorizonPlugin|Style|HyperText")
		TArray<TSubclassOf<UHorizonButton>> BackgroundButtonClass;
public: //for tag <img>, ignore case
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HorizonPlugin|Style|Image", meta = (DisplayThumbnail = "true", AllowedClasses = "Texture2D"))
		TSoftObjectPtr<UTexture2D> Texture2D = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HorizonPlugin|Style|Image", meta = (DisplayThumbnail = "true", AllowedClasses = "Material"))
		TSoftObjectPtr<UMaterial> Material = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HorizonPlugin|Style|Image", meta = (DisplayThumbnail = "true"))
		TSoftObjectPtr<UPaperSprite> PaperSprite = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HorizonPlugin|Style|Sound")
		TSoftObjectPtr<USoundBase> DialogueSound = nullptr;




	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HorizonPlugin|Style|Sound")
		TArray<float> DialogueSoundVolumeMultiplier;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HorizonPlugin|Style|Sound")
		TArray<float> DialogueSoundPitchMultiplier;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HorizonPlugin|Style|Sound")
		TArray<float> DialogueSoundStartTime;

public: //for tag <pfb> or <PaperFlipBook>, ignore case
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HorizonPlugin|Style|Image|Flipbook", meta = (DisplayThumbnail = "true", DisplayName = "PaperFlipbook", AllowedClasses = "PaperFlipbook"))
		TSoftObjectPtr<UPaperFlipbook> PaperFlipbook = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, AdvancedDisplay, Category = "HorizonPlugin|Style|Image|Flipbook")
		TArray<FVector2D> PaperFlipbookSourceUV;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, AdvancedDisplay, Category = "HorizonPlugin|Style|Image|Flipbook")
		TArray<FVector2D> PaperFlipbookSourceSize;

public:
	//image size for Texture2D, Material or PaperFlipbook
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HorizonPlugin|Style|Image")
		TArray<FVector2D> ImageSize;
};


/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class HORIZONUI_API UHorizonDialogueStyleInfo: public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HorizonPlugin|StyleList")
	TArray<FHorizonDialogueSegmentInfoStyle> SegmentStyleList;


public:
#if WITH_EDITOR
	//~ Begin UObject Interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
	//~ End UObject Interface
#endif
};

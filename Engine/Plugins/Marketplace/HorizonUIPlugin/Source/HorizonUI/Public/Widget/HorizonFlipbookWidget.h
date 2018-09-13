// Crated by dorgon, All Rights Reserved.
// email: dorgonman@hotmail.com
// blog: dorgon.horizon-studio.net

#pragma once
#include "Widget/HorizonImage.h"
#include "HorizonFlipbookWidget.generated.h"
class UPaperFlipbookComponent;
class UPaperFlipbook;
UCLASS()
class HORIZONUI_API UHorizonFlipbookWidget :
	public UCanvasPanel,
	public FTickableGameObject
{
	GENERATED_BODY()
public:


#if WITH_EDITOR
	//~ Begin UWidget Interface
	//#if (ENGINE_MAJOR_VERSION <= 4) && (ENGINE_MINOR_VERSION <= 12)
	//virtual const FSlateBrush* GetEditorIcon() override;
	//#endif
	virtual const FText GetPaletteCategory() override;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
public:

	/**
	* Sets the color and opacity of the text in this text block
	*
	* @param InColorAndOpacity		The new text color and opacity
	*/
	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|Appearance")
		void SetColorAndOpacity(FSlateColor InColorAndOpacity);

	/**
	* Sets the opacity of the text in this text block
	*
	* @param InOpacity		The new text opacity
	*/
	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|Appearance")
		void SetOpacity(float InOpacity);
	//~ Begin UWidget Interface
	virtual void SynchronizeProperties() override;
	//~ End UWidget Interface


protected:
	//~ Begin UWidget Interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void OnWidgetRebuilt() override;
	virtual void OnBindingChanged(const FName& Property) override;
	//~ End UWidget Interface



	//~ Begin UVisual Interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//~ End UVisual Interface

protected://FTickableGameObject implement
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UHorizonFlipbookWidget, STATGROUP_Tickables); }
	virtual bool IsTickable() const override { return bIsStartTick; }
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickableInEditor() const override {return true;}


public:
	void SetFlipbook(UPaperFlipbook* flipbook);

	void SetFlipbookSourceUV(const TArray<FVector2D>& sourceUV) { SourceUV = sourceUV; };
	void SetFlipbookSourceSize(const TArray<FVector2D>& sourceSize) { SourceSize = sourceSize; };


	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|Appearance")
	void SetFlipbookSource(const TArray<FVector2D>& sourceUV, const TArray<FVector2D>& sourceSize);


	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|Behavior")
	void ResetAnimation();
	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|Behavior")
	void PlayAnimation();
	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|Behavior")
	void StopAnimation();
	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|Behavior")
	void PauseAnimation();
	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|Behavior")
	void ResumeAnimation();
	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|Behavior")
	void SetCurrentAnimationDuration(float duration);
	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|Behavior")
	float GetCurrentAnimationDuration();
private:
	void SetCurrentImage(float currentDuration);
public:

	/** The color of the text */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Appearance)
	FSlateColor ColorAndOpacity = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HorizonPlugin|Behavior")
	bool bIsStartTick = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HorizonPlugin|Behavior")
	int32 NumOfLoop = 0;
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HorizonPlugin|Brush", meta = (DisplayThumbnail = "true", DisplayName = "PaperFlipbook", AllowedClasses = "PaperFlipbook"))
	UPaperFlipbook* PaperFlipbook = nullptr;



	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Brush)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HorizonPlugin|Brush")
	TArray<FVector2D> SourceUV;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Brush)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HorizonPlugin|Brush")
	TArray<FVector2D> SourceSize;

private:
	TWeakObjectPtr<UHorizonImage> ImageWeakPtr; //for showing current image
	float CurrentDuration = 0.0f;
	int32 CurrentNumOfLoop = 0;

    //float AnimationDeltaTime;
};
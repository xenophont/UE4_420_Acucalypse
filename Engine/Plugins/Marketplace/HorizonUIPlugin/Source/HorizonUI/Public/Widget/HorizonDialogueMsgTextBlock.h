// Crated by dorgon, All Rights Reserved.
// email: dorgonman@hotmail.com
// blog: dorgon.horizon-studio.net


#pragma once
#include "UObject/ObjectMacros.h"
#include "Framework/Text/TextLayout.h"
#include "Delegates/DelegateCombinations.h"
#include "Tickable.h"

#include "Components/CanvasPanel.h"

#include "Widget/HorizonButton.h"
#include "Widget/HorizonDialogueStyleInfo.h"
#include "HorizonDialogueMsgTextBlock.generated.h"

class UHorizonDialogueMsgTextBlock;
class USoundBase;

#if 1 //============================Begin DialogueText Enum=============================
UENUM(BlueprintType)
enum class EHorizonDialogueSegmentType :uint8
{
	Invalidated = 0,
	Text,
	NewLine,
	Image,
	PaperFlipbook,
	Material,
	HyperText
};



UENUM(BlueprintType)
enum class EHorizonDialogueTextOverflowWarpMethod :uint8
{
	Normal,    // Try get a "word"(use space to determine, for both CJK and non-CJK) and check if can place at current line, if not, move to next line and use BreakAll rule in next line for the "word"
	BreakAll, //  break all word at any place
};

#endif //============================End DialogueText Enum=============================

#if 1 //============================Begin DialogueText Struct=============================
USTRUCT(BlueprintType)
struct FHorizonDialogueBlockInfo 
{
	GENERATED_BODY()
public:
	FString MsgText;
	TWeakObjectPtr<UWidget> WidgetWeakPtr;
	TWeakObjectPtr<UWidget> WidgetBackgroundWeakPtr;
	int32 CurrentCharIndex = 0;
	int32 SegmentReferenceIndex = -1;//index ref to TArray<FHorizonDialogueSegmentInfo> DialogueSegmentInfoList
	FName Name;
	FVector2D BlockSize;


	UPROPERTY()
	USoundBase* DialogueSound = nullptr;
	bool bDialogueSoundPlayed = false;

};

USTRUCT(BlueprintType)
struct FHorizonDialogueLineInfo
{
	GENERATED_BODY()
public:
	int32 CurrentDialogueBlockIndex = 0;
	int32 MaxLineHeight = 0;
	int32 LineWidth = 0;
	TArray<FHorizonDialogueBlockInfo> DialogueBlockInfoList;
	FVector2D Position = FVector2D(0, 0);
};


// Parse text segment and store it tag info, 
// If no supported tag in input text, then there will have only one segment.
USTRUCT(BlueprintType)
struct FHorizonDialogueSegmentInfo {
	GENERATED_BODY()
	FHorizonDialogueSegmentInfo();

public:
	EHorizonDialogueSegmentType TypeEnum = EHorizonDialogueSegmentType::Invalidated;
	FString Text;
	FSlateColor ColorAndOpacity;
	FSlateFontInfo Font;
	FVector2D ShadowOffset;
	FSlateColor ShadowColorAndOpacity;

	float DialogueMsgSpeed = 0.01f;
	float DialogueMsgWait = 0.0f;
	FMargin PaddingMargin;//padding position for segment
	TOptional<FString> FilePath;
	TOptional<FString> DialogueSoundPath;
	TOptional<FVector2D> ImageSize;
	TOptional<FString> HypertextReference;
	FSlateColor HypertextHoveredColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); //color after clicked
	FSlateColor HypertextVisitedColor = FLinearColor(0.5f, 0.0f, 0.5f, 1.0f); //color after clicked
	TSubclassOf<UHorizonButton> BackgroundButtonClass;// = UHorizonButton::StaticClass();


	int32 StyleInfoReferenceIndex = -1; //if not -1, will use style in SegmentStyleListFromStyleClass
	int32 SegmentStyleReferenceIndex = -1;

	bool bHypertextVisited = false;

	float DialogueSoundVolumeMultiplier = 1.0f;
	float DialogueSoundPitchMultiplier = 1.0f;
	float DialogueSoundStartTime = 0.0f;

};


USTRUCT(BlueprintType)
struct FHorizonDialoguePageInfo
{
	GENERATED_BODY()
public:
	FHorizonDialoguePageInfo() {};
	FHorizonDialoguePageInfo(int32 InStartLineIndex, int32 InEndLineIndex, float InPageHeight)
	: StartLineIndex(InStartLineIndex)
	, EndLineIndex(InEndLineIndex)
	, PageHeight(InPageHeight){}
public:
	int32 StartLineIndex = -1;
	int32 EndLineIndex = -1;
	float PageHeight = 0.0f;
};

USTRUCT(BlueprintType)
struct FHorizonDialogueHypertextResult
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hypertext")
	UHorizonDialogueMsgTextBlock* DialogueMsgTextBlock = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Hypertext")
	int32 SegmentIndex = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Hypertext")
	int32 LineIndex = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Hypertext")
	int32 BlockIndex = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Hypertext")
	FString HypertextReference;
	//HypertextReference parsed map
	UPROPERTY(BlueprintReadOnly, Category = "Hypertext")
	TMap<FString, FString> UrlEncodeMap;
};


USTRUCT(BlueprintType)
struct FHorizonDialogueDialoguePageResult
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Category = "HorizonPlugin|DialogueMsgTextBlock")
		int32 PageIndex = -1;
};

#endif //============================End DialogueText Struct=============================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHorizonHypertextEvent, const FHorizonDialogueHypertextResult&, InResult);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnHorizonHypertextEventNative, const FHorizonDialogueHypertextResult&);


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHorizonDialogueMsgEvent);
DECLARE_MULTICAST_DELEGATE(FOnHorizonDialogueMsgEventNative);


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHorizonDialoguePageEvent, const FHorizonDialogueDialoguePageResult&, InResult);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnHorizonDialoguePageEventNative, const FHorizonDialogueDialoguePageResult&);


UCLASS()
class HORIZONUI_API UHorizonDialogueMsgTextBlock :
	public UCanvasPanel,
	public FTickableGameObject
{
	GENERATED_BODY()
public:
	UHorizonDialogueMsgTextBlock();



#if 1  //============================Begin Widget override=============================
public:
	//~ Begin UWidget Interface
	virtual void SynchronizeProperties() override;
	//~ End UWidget Interface

	//~ Begin UVisual Interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//~ End UVisual Interface

protected:
	//~ Begin UWidget Interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void OnWidgetRebuilt() override;
	virtual void OnBindingChanged(const FName& Property) override;
	//~ End UWidget Interface
#endif //============================End Widget override=============================

#if WITH_EDITOR
public:
	//~ Begin UWidget Interface
	//#if (ENGINE_MAJOR_VERSION <= 4) && (ENGINE_MINOR_VERSION <= 12)
	//virtual const FSlateBrush* GetEditorIcon() override;
	//#endif //#if (ENGINE_MAJOR_VERSION <= 4) && (ENGINE_MINOR_VERSION <= 12)
	virtual const FText GetPaletteCategory() override;
	virtual void OnCreationFromPalette() override;
	//~ End UWidget Interface

	virtual FString GetLabelMetadata() const override;

	void HandleTextCommitted(const FText& InText, ETextCommit::Type CommitteType);

	//~ Begin UObject Interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
	//~ End UObject Interface
#endif

#if 1  //============================Begin FTickableGameObject override=============================
protected://FTickableGameObject implement
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UHorizonDialogueMsgTextBlock, STATGROUP_Tickables); }
	virtual bool IsTickable() const override;
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickableInEditor() const override { return true; }
#endif //============================End FTickableGameObject override=============================


#if 1 //============================Begin DialogueText Method=============================
public: 
	/**
	* Sets the color and opacity of the text in this text block
	*
	* @param InColorAndOpacity		The new text color and opacity
	*/
	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|UI|DialogueMsg")
	virtual void SetColorAndOpacity(FSlateColor InColorAndOpacity);

	/**
	* Sets the opacity of the text in this text block
	*
	* @param InOpacity		The new text opacity
	*/
	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|UI|DialogueMsg")
	virtual void SetOpacity(float InOpacity);

	/**
	* Sets the color and opacity of the text drop shadow
	* Note: if opacity is zero no shadow will be drawn
	*
	* @param InShadowColorAndOpacity		The new drop shadow color and opacity
	*/
	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|UI|DialogueMsg")
	virtual void SetShadowColorAndOpacity(FLinearColor InShadowColorAndOpacity);

	/**
	* Sets the offset that the text drop shadow should be drawn at
	*
	* @param InShadowOffset		The new offset
	*/
	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|UI|DialogueMsg")
	virtual void SetShadowOffset(FVector2D InShadowOffset);

	/**
	* Dynamically set the font info for this text block
	*
	* @param InFontInfo THe new font info
	*/
	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|UI|DialogueMsg")
	virtual void SetFont(FSlateFontInfo InFontInfo);

	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|UI|DialogueMsg")
	virtual void SetFontSize(int32 fontSize);

	/**
	*  Set the text justification for this text block
	*
	*  @param Justification new justification
	*/
	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|UI|DialogueMsg")
	virtual void SetJustification(ETextJustify::Type InJustification);
	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|UI|DialogueMsg")
	FORCEINLINE TEnumAsByte<ETextJustify::Type> GetJustification() const { return Justification; }


	/**
	* Gets the widget text
	* @return The widget text
	*/
	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|UI|DialogueMsg", meta = (DisplayName = "GetText (Text)"))
	FText GetText() const;

	/**
	* Directly sets the widget text.
	* Warning: This will wipe any binding created for the Text property!
	* @param InText The text to assign to the widget
	*/
	//UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|Widget", meta = (DisplayName = "SetText (Text)"))
	virtual void SetText(FText InText);

	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|UI|DialogueMsg")
	virtual void SetTextAndRebuildDialogue(const FText& InText);


#endif //============================End Text Method=============================


#if 1 //============================Begin Dialogue Method=============================
public:
	virtual void SetIsRichText(bool b);

	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|UI|DialogueMsg")
	virtual void SetIsDialogueMsgText(bool b);
	FORCEINLINE bool IsDialogueMsgText() { return bIsDialogueMsgText; };

	virtual void SetDialogueMsgSpeed(float speed);


	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|UI|DialogueMsg")
	virtual void SetIsStartTickDialogueMsg(bool b);
	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|UI|DialogueMsg")
	virtual void SetIsRepeatDialogueMsg(bool b);

	virtual void SetRepeatDialogueMsgInterval(float interval);



	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|UI|DialogueMsg")
	virtual void StopDialogue();
	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|UI|DialogueMsg")
	virtual void StartDialogue();



	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|UI|DialogueMsg")
	virtual void NextDialogueMsgPage();
	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|UI|DialogueMsg")
	FORCEINLINE int32 GetCurrentPageIndex() { return CurrentPageIndex; };
	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|UI|DialogueMsg")
	virtual void SetDialogueMsgPage(int32 Page);


	// Note: if false, user should call NextDialogueMsgPage or SetDialogueMsgPage in order to proceed to next page.
	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|UI|DialogueMsg")
	virtual void SetIsAutoNextDialogueMsgPage(bool b);

	FORCEINLINE bool IsAutoNextDialogueMsgPage() { return bIsAutoNextDialogueMsgPage; }


	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|UI|DialogueMsg")
	virtual void SetAutoNextDialogueMsgPageIntervalRate(float InAutoNextDialogueMsgPageIntervalRate);

	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|UI|DialogueMsg")
	virtual int32 GetTextLength();
	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|UI|DialogueMsg")
	virtual int32 GetCurrentPageTextLength();

	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|UI|DialogueMsg")
	virtual FText GetPageTextByIndex(int32 PageIndex);



	virtual void ResetDialogueMsgText();

	virtual void RebuildDialogueMsgTextBlock();


	// please use SkipCurrentDialogueMsgPageTick instead
	//virtual void SkipDialogue();

	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|UI|DialogueMsg")
	virtual void SkipCurrentDialogueMsgPageTick();
	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|UI|DialogueMsg")
	virtual bool IsDialogueMsgPageEnd();
	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|UI|DialogueMsg")
	virtual bool IsDialogueMsgCompleted();



	FORCEINLINE TArray<FHorizonDialogueLineInfo>& GetDialogueLineInfoList() { return DialogueLineInfoList; };
	FORCEINLINE TArray<FHorizonDialoguePageInfo>& GetDialoguePageInfoList() { return DialoguePageInfoList; };
	FORCEINLINE TArray<FHorizonDialogueSegmentInfo>& GetDialogueSegmentInfoList() { return DialogueSegmentInfoList; };


	virtual bool IsNeedRebuildDialogueMsgText() { return bNeedRebuildDialogueMsgText; }

protected:
	virtual void TickRepeatDialogue(float DeltaTime);
private:
	void CalculateAutoNextDialogueMsgPageInterval();
	void SetPageVisiblity(bool b, const FHorizonDialoguePageInfo& InPageInfo);
#endif //============================End Dialogue Method=============================

#if 1 //============================Begin Dialogue Method: Rebuild=============================
protected:
	virtual void RebuildSegmentInfoList();
	virtual void RebuildLineInfoList();
	virtual void RebuildPageInfoList();
	virtual void RebuildBlockInfoJustification();
private:
	void RebuildSegmentInfoListImplement(const FHorizonDialogueSegmentInfo& parentSegParam, const FXmlNode* pCurrentNode);
	void AddTextSegmentInfo(const FString& text);

	FHorizonDialogueSegmentInfo CreateDefaultSegmentInfo(EHorizonDialogueSegmentType segmentType);
	FHorizonDialogueSegmentInfo CreateSegmentInfo(const FHorizonDialogueSegmentInfo& parentSegParam, const FXmlNode* pXmlNode);
	//void BuildSegmentInfoList(const FString& segmentStr);

	FHorizonDialogueBlockInfo CreateDialogueTextBlockInfo(int32 segmentIndex, int32 segTextStartIndex, int32 segTextEndIndex, const FVector2D& pos);
	FHorizonDialogueBlockInfo CreateDialogueImageBlockInfo(int32 segmentIndex, const FVector2D& pos);
	FHorizonDialogueBlockInfo CreateDialoguePaperFlipbookBlockInfo(int32 segmentIndex, const FVector2D& pos);

	//add BG button to Block
	void AddBackgroundButton(int32 segmentIndex,
		FHorizonDialogueBlockInfo& blockInfo,
		UCanvasPanelSlot* pCanvasPanelSlot, const FVector2D& paddingPos);
	void AddDialogueBlock(FHorizonDialogueLineInfo& lineInfo, FHorizonDialogueBlockInfo&& blockInfo);

	void LoadSound(FHorizonDialogueSegmentInfo& segInfo, FHorizonDialogueBlockInfo& blockInfo);
#endif //============================End Dialogue Method: Rebuild=============================


#if 1 //============================Begin Dialogue Method: Text Overflow=============================

private://text overflow warp method implement

	bool TryAddNewLine(TCHAR currentChar, float& currentLineWidth, FVector2D& blockPos,
		const int& currentSegInfoIndex, int32& segStartCharIndex, int32& segCharIndex);


	bool TryAddDialogueBlock(const float& maxLineWidth,
		const FVector2D& oneWordSize, float& currentLineWidth,
		FVector2D& blockPos,
		const int& currentSegInfoIndex, int32& segStartCharIndex, int32& segCharIndex,
		bool bWordBreakMode = false);





	void TextOverflowWarpNormal_Implement(float& currentLineWidth, FVector2D& blockPos, const int& currentSegInfoIndex,
		const float& maxLineWidth, const FHorizonDialogueSegmentInfo& segInfo);

	void TextOverflowWarpNormal_WordBreakImplement(int32& segCharIndex, int32& segWordCharEndIndex, float& currentLineWidth, FVector2D& blockPos, const int& currentSegInfoIndex,
		const float& maxLineWidth, const FHorizonDialogueSegmentInfo& segInfo);

	void TextOverflowWarpBreakAll_Implement(float& currentLineWidth, FVector2D& blockPos, const int& currentSegInfoIndex,
		const float& maxLineWidth, const FHorizonDialogueSegmentInfo& segInfo);

	void SetSegmentColor(int32 segmentIndex, int32 lineIndex, const FSlateColor& InColor);

#endif //============================End Dialogue Method: Text Overflow=============================


#if 1 //============================Begin Dialogue Delegate=============================
public: //delegate

		// for blueprint
	UPROPERTY(BlueprintAssignable, Category = "HorizonPlugin|UI|DialogueMsg")
	FOnHorizonHypertextEvent OnHypertextClickedDelegate;
	UPROPERTY(BlueprintAssignable, Category = "HorizonPlugin|UI|DialogueMsg")
	FOnHorizonHypertextEvent OnHypertextPressedDelegate;
	UPROPERTY(BlueprintAssignable, Category = "HorizonPlugin|UI|DialogueMsg")
	FOnHorizonHypertextEvent OnHypertextReleasedDelegate;
	UPROPERTY(BlueprintAssignable, Category = "HorizonPlugin|UI|DialogueMsg")
	FOnHorizonHypertextEvent OnHypertextHoveredDelegate;
	UPROPERTY(BlueprintAssignable, Category = "HorizonPlugin|UI|DialogueMsg")
	FOnHorizonHypertextEvent OnHypertextUnhoveredDelegate;
	UPROPERTY(BlueprintAssignable, Category = "HorizonPlugin|UI|DialogueMsg")
	FOnHorizonDialogueMsgEvent OnDialogueMsgLoopFunction;
	UPROPERTY(BlueprintAssignable, Category = "HorizonPlugin|UI|DialogueMsg")
	FOnHorizonDialogueMsgEvent OnDialogueMsgCompleteFunction;

	UPROPERTY(BlueprintAssignable, Category = "HorizonPlugin|UI|DialogueMsg")
	FOnHorizonDialoguePageEvent OnSetDialoguePageFunction;

	UPROPERTY(BlueprintAssignable, Category = "HorizonPlugin|UI|DialogueMsg")
	FOnHorizonDialoguePageEvent OnDialoguePageEndFunction;



	// for c++ callback binding
	FOnHorizonHypertextEventNative OnHypertextClickedDelegateNative;
	FOnHorizonHypertextEventNative OnHypertextPressedDelegateNative;
	FOnHorizonHypertextEventNative OnHypertextReleasedDelegateNative;
	FOnHorizonHypertextEventNative OnHypertextHoveredDelegateNative;
	FOnHorizonHypertextEventNative OnHypertextUnhoveredDelegateNative;

	FOnHorizonDialogueMsgEventNative OnDialogueMsgLoopFunctionNative;
	FOnHorizonDialogueMsgEventNative OnDialogueMsgCompleteFunctionNative;

	FOnHorizonDialoguePageEventNative OnSetDialoguePageFunctionNative;
	FOnHorizonDialoguePageEventNative OnDialoguePageEndFunctionNative;

#endif //============================End Dialogue Delegate=============================




#if 1 //============================Begin Attribute: Content=============================
public:
	/** The text to display */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Content", meta = (MultiLine = "true"))
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Content")
	EHorizonDialogueTextOverflowWarpMethod TextOverFlowWarpMethod = EHorizonDialogueTextOverflowWarpMethod::Normal;	

#endif //============================End Attribute: Content=============================


#if 1 //============================Begin Attribute: Behavior=============================
public:
	/** millisecond for a word */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (EditCondition = "bIsDialogueMsgText"))
	float DialogueMsgSpeed = 0.01f;

	/** should we start tick dialogue*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (EditCondition = "bIsDialogueMsgText"))
	bool bIsStartTickDialogueMsg = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (EditCondition = "bIsDialogueMsgText"))
	bool bIsRepeatDialogueMsg = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (EditCondition = "bIsRepeatDialogueMsg"))
	float RepeatDialogueMsgInterval = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	bool bIsDialogueMsgText = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	bool bIsAutoNextDialogueMsgPage = true;

	// AutoNextDialogueMsgPageInterval = AutoNextDialogueMsgPageIntervalRate * GetTextLength();
	// Then clamp to Min, Max
	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = "SetAutoNextDialogueMsgPageIntervalRate", Category = "Behavior", meta = (EditCondition = "bIsAutoNextDialogueMsgPage"))
	float AutoNextDialogueMsgPageIntervalRate = 0.01f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (EditCondition = "bIsAutoNextDialogueMsgPage"))
	float AutoNextDialogueMsgPageIntervalMin = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (EditCondition = "bIsAutoNextDialogueMsgPage"))
	float AutoNextDialogueMsgPageIntervalMax = 10.0f;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
	bool bIgnoreTimeDilation = false;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "!bIgnoreTimeDilation"), Category = "Behavior")
	float CustomTimeDilation = 1.0f;
#endif //============================End Attribute: Behavior=============================
#if 1 //============================Begin Attribute: Style=============================
public:
	//Load SegmentStyles from seperate UHorizonDialogueStyleInfo class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	TArray<TSubclassOf<UHorizonDialogueStyleInfo>> StyleInfoClassList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	TArray<FHorizonDialogueSegmentInfoStyle> SegmentStyleList;
#endif //============================End Attribute: Style=============================
#if 1 //============================Begin Attribute: Appearance=============================
public:

	/** if true, then try to parse Text xml tag*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	bool bIsRichText = true;



	/** The color of the text */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	FSlateColor ColorAndOpacity = FLinearColor::White;

	/** The font to render the text with */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	FSlateFontInfo Font;

	/** The direction the shadow is cast */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	FVector2D ShadowOffset = FVector2D(1.0f, 1.0f);

	/** The color of the shadow */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance", meta = (DisplayName = "Shadow Color"))
	FLinearColor ShadowColorAndOpacity = FLinearColor::Transparent;

	///** Called when this text is double clicked */
	//SLATE_EVENT(FOnClicked, OnDoubleClicked)

	// UTextLayoutWidget 
	/** How the text should be aligned with the margin. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	TEnumAsByte<ETextJustify::Type> Justification;

	/** The amount of blank space left around the edges of this text line. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance", AdvancedDisplay)
	FMargin LineMargin = FMargin(30, 10, 30, 5);



#endif //============================End Attribute: Appearance=============================
#if 1 //============================Begin Attribute: Performance=============================
	/** If true, it will automatically wrap this text widget with an invalidation panel */
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance", AdvancedDisplay)
	//bool bWrapWithInvalidationPanel = false;
#endif //============================End Attribute: Performance=============================
#if 1 //============================Begin Attribute: Non-Public=============================
protected:

	TArray<FHorizonDialoguePageInfo> DialoguePageInfoList;
	TArray<FHorizonDialogueLineInfo> DialogueLineInfoList;
	TArray<FHorizonDialogueSegmentInfo> DialogueSegmentInfoList;

	int32 CurrentDialogueLineIndex = 0;
	int32 CurrentPageIndex = 0;


	float MsgDeltaTime = 0.0f;

	float RepeatDialogueMsgDeltaTime = 0.0f;

	bool bCreationFromPalette = false;

private:
	float AutoNextDialogueMsgPageDeltaTime = 0.0f;
	//AutoNextDialogueMsgPageInterval = FMath::Clamp(AutoNextDialogueMsgPageInterval, AutoNextDialogueMsgPageIntervalMin, AutoNextDialogueMsgPageIntervalMax);
	float AutoNextDialogueMsgPageInterval = 1.0f;
	UPROPERTY()
	TArray<UHorizonDialogueStyleInfo*> StyleInfoList;

	bool bNeedRebuildDialogueMsgText = false;
	// off set for recalcuate position.Y for line's in other page
	float CurrentPageHeightOffset = 0.0f;

	int32 TexLength = 0;
	float MaxLineWidth = 0.0f;

#endif //============================Begin Attribute: Non-Public=============================





};
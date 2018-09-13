// Crated by dorgon, All Rights Reserved.
// email: dorgonman@hotmail.com
// blog: dorgon.horizon-studio.net


#include "Widget/HorizonFlipbookWidget.h"
#include "Widget/HorizonWidgetFunctionLibrary.h"

#include "PaperFlipbook.h"
#include "PaperFlipbookComponent.h"


//UMG
#include "UMGStyle.h"


#if WITH_EDITOR

//FString UHorizonFlipbookWidget::GetLabelMetadata() const
//{
	
	//return TEXT(" \"") + TextStr + TEXT("\"");
//}

//void UHorizonFlipbookWidget::HandleTextCommitted(const FText& InText, ETextCommit::Type CommitteType)
//{
	//TODO UMG How will this migrate to the template?  Seems to me we need the previews to have access to their templates!
	//TODO UMG How will the user click the editable area?  There is an overlay blocking input so that other widgets don't get them.
	//     Need a way to recognize one particular widget and forward things to them!
//}
//#if (ENGINE_MAJOR_VERSION <= 4) && (ENGINE_MINOR_VERSION <= 12)
/*const FSlateBrush* UHorizonFlipbookWidget::GetEditorIcon()
{
	return FUMGStyle::Get().GetBrush("Widget.Image");
}*/
//#endif //#if (ENGINE_MAJOR_VERSION <= 4) && (ENGINE_MINOR_VERSION <= 12)

const FText UHorizonFlipbookWidget::GetPaletteCategory()
{
	return NSLOCTEXT("HorizonPlugin", "HorizonPlugin", "HorizonPlugin");
}

//void UHorizonFlipbookWidget::OnCreationFromPalette()
//{
	//Text = LOCTEXT("TextBlockDefaultValue", "flipbook image");
//}




void UHorizonFlipbookWidget::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) {
	Super::PostEditChangeProperty(PropertyChangedEvent);
	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(UHorizonFlipbookWidget, PaperFlipbook)))
	{
		UHorizonWidgetFunctionLibrary::RebuildFlipbook(PaperFlipbook, SourceUV, SourceSize);
	

	}
}
#endif //#if WITH_EDITOR



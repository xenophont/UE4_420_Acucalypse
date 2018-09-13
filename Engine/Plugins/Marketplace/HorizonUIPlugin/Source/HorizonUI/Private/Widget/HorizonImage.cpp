// Crated by dorgon, All Rights Reserved.
// email: dorgonman@hotmail.com
// blog: dorgon.horizon-studio.net

#include "Widget/HorizonImage.h"
UHorizonImage::UHorizonImage()
{
	UVRegion.bIsValid = true;
}


#if WITH_EDITOR

const FText UHorizonImage::GetPaletteCategory()
{
	return NSLOCTEXT("HorizonPlugin", "HorizonPlugin", "HorizonPlugin");
}





void UHorizonImage::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) {
	Super::PostEditChangeProperty(PropertyChangedEvent);
	

}
#endif //#if WITH_EDITOR

void UHorizonImage::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	Brush.SetUVRegion(UVRegion);
}

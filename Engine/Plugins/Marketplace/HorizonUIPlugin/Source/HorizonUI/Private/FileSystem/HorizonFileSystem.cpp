// Crated by dorgon, All Rights Reserved.
// email: dorgonman@hotmail.com
// blog: dorgon.horizon-studio.net

#include "FileSystem/HorizonFileSystem.h"

#include "Engine/Texture2D.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "HAL/PlatformFilemanager.h"

//engine
#include "PaperFlipbook.h"
#include "Modules/ModuleManager.h"
#include "Misc/FileHelper.h"
#include "Sound/SoundBase.h"
#include "Engine/Font.h"
//#include "WidgetBlueprint.h"
// singleton stuff
UHorizonFileSystem* UHorizonFileSystem::Instance = nullptr;

static EImageFormat GetImageFormat(const FString& fileName)
{

	if (fileName.EndsWith(".jpg") || fileName.EndsWith(".JPG")) {
		return EImageFormat::JPEG;
	}
	else if (fileName.EndsWith(".png") || fileName.EndsWith(".PNG")) {
		return EImageFormat::PNG;

	}
	else if (fileName.EndsWith(".bmp") || fileName.EndsWith(".BMP")) {
		return EImageFormat::BMP;
	}
	else if (fileName.EndsWith(".ico") || fileName.EndsWith(".ICO")) {
		return EImageFormat::ICO;
	}
	else if (fileName.EndsWith(".exr") || fileName.EndsWith(".EXR")) {
		return EImageFormat::EXR;
	}
	else if (fileName.EndsWith(".icns") || fileName.EndsWith(".ICNS")) {
		return EImageFormat::ICNS;
	}
	return EImageFormat::PNG;
}

UHorizonFileSystem* UHorizonFileSystem::GetInstance() {
	if (!Instance)
	{
		Instance = NewObject<UHorizonFileSystem>();
		//s_instance = new UHorizonFileSystem();
		Instance->AddToRoot();
		//s_instance->SetFlags(EObjectFlags::RF_MarkAsRootSet);
	}

	return Instance;
}

void UHorizonFileSystem::DestroyInstance() {
	if (Instance) {
		Instance->RemoveFromRoot();
		//Instance->ConditionalBeginDestroy();
		Instance = nullptr;
	}
}

//
//void UHorizonFileSystem::ClearFileSearchPathList() {
//	SearchPathList.Reset();
//}
//
//const TArray<FString>& UHorizonFileSystem::GetFileSearchPathList() {
//	return SearchPathList;
//}
//
//void UHorizonFileSystem::RegisterFileSearchPath(FString InLongPackageName) {
//	
//	FString Result;
//	if (FPackageName::TryConvertLongPackageNameToFilename(InLongPackageName, Result, TEXT(""))) {
//		if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*Result)) {
//			CreateDirectoryRecursively(Result);
//		}
//		//FPaths::NormalizeDirectoryName(InLongPackageName);
//		SearchPathList.Insert(MoveTemp(InLongPackageName), 0);
//	}
//
//}


void UHorizonFileSystem::CreateDirectoryRecursively(FString FolderToMake)
{
	// code copy from here: https://wiki.unrealengine.com/Algorithm_Analysis:_Create_Directory_Recursively
	// thanks Rama!!

	//FolderToMake is not const so split can be used, and does not damage input

	//Loop Proteciton
	const int32 MAX_LOOP_ITR = 3000; //limit of 3000 directories in the structure

									 // Normalize all / and \ to TEXT("/") and remove any trailing TEXT("/") 
									 //if the character before that is not a TEXT("/") or a colon
	FPaths::NormalizeDirectoryName(FolderToMake);

	//Normalize removes the last "/", but my algorithm wants it
	FolderToMake += "/";

	FString Base;
	FString Left;
	FString Remaining;

	//Split off the Root
	FolderToMake.Split(TEXT("/"), &Base, &Remaining);
	Base += "/"; //add root text to Base

	int32 LoopItr = 0;
	while (Remaining != "" && LoopItr < MAX_LOOP_ITR)
	{
		Remaining.Split(TEXT("/"), &Left, &Remaining);

		//Add to the Base
		Base += Left + FString("/"); //add left and split text to Base

									 //Create Incremental Directory Structure!
		FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*Base);

		LoopItr++;
	}
}
UObject* UHorizonFileSystem::LoadUAsset(const FString& packageFilePath) {
	FStringAssetReference assetRef = packageFilePath;
	UObject* pRet = assetRef.TryLoad();
	if (nullptr == pRet) {
		FString inPath;
		//filePath
		if (packageFilePath.StartsWith(TEXT("/"), ESearchCase::CaseSensitive))
		{
			// overwrite trailing slash with terminator
			inPath = packageFilePath.Mid(1, packageFilePath.Len());
		}
		else {
			inPath = packageFilePath;
		}
		// try search file from search path
		FString LongPackageName;
		if (SearchPathList.Num() == 0) {
			//use GameContentDir as default File path
			LongPackageName = "/Game/" + inPath;
			assetRef = LongPackageName;
			pRet = assetRef.TryLoad();
		}
		else {
			for (auto& it : SearchPathList) {
				FPaths::NormalizeDirectoryName(it);
				LongPackageName = it + "/" + inPath;
				assetRef = LongPackageName;
				pRet = assetRef.TryLoad();
				if (pRet) {
					break;
				}
			}
		}
	}

	return pRet;
}

UPaperFlipbook* UHorizonFileSystem::LoadPaperFlipbook(const FString& packageFilePath) {

	UPaperFlipbook* pRet = Cast<UPaperFlipbook>(LoadUAsset(packageFilePath));
	return pRet;
}

UMaterial* UHorizonFileSystem::LoadMaterial(const FString& packageFilePath) {
	UMaterial* pRet = Cast<UMaterial>(LoadUAsset(packageFilePath));
	return pRet;
}



UFont* UHorizonFileSystem::LoadFont(const FString& packageFilePath) {
	UFont* pRet = Cast<UFont>(LoadUAsset(packageFilePath));
	return pRet;
}

USoundBase* UHorizonFileSystem::LoadSound(const FString& packageFilePath)
{
	auto pRet = Cast<USoundBase>(LoadUAsset(packageFilePath));
	return pRet;
}

//UUserWidget* UHorizonFileSystem::LoadUserWidget(UWorld* pWorld, const FString& packageFilePath) {
//
//	UUserWidget* pRet = nullptr;
//	/*UWidgetBlueprint* pWidgetBlueprint = Cast<UWidgetBlueprint>(LoadUAsset(packageFilePath));
//	if (pWidgetBlueprint) {
//		pRet = CreateWidget<UUserWidget>(pWorld, pWidgetBlueprint->GeneratedClass);
//	}*/
//	return pRet;
//}
UTexture2D* UHorizonFileSystem::LoadTexture2D(const FString& packageFilePath, int32& OutWidth, int32& OutHeight) {
	/*static const FString* PackageExtensions[] =
	{
		&AssetPackageExtension,
		&MapPackageExtension
	};*/

	//try load uassets
	UTexture2D* LoadedT2D = Cast<UTexture2D>(LoadUAsset(packageFilePath));
	//try loading Rawdata
	if (nullptr == LoadedT2D) {
		FString realFilePath;
		FPackageName::TryConvertLongPackageNameToFilename(packageFilePath, realFilePath, TEXT(""));
		//if (!FPaths::FileExists(realFilePath)) {
			//FPackageName::TryConvertLongPackageNameToFilename(packageFilePath, realFilePath, TEXT(""), false);
		//}

		bool isValid = false;
		// Full path, so we can load texture directly
		if (FPaths::FileExists(realFilePath)) {
			LoadedT2D = LoadTexture2DImplement(realFilePath, isValid, OutWidth, OutHeight);
		}
		else {
			// try search file from search path
			FString LongPackageName;
			if (SearchPathList.Num() == 0) {
				//use GameContentDir as default File path
				LongPackageName = "/Game/" + packageFilePath;
			}
			else { 
				for (auto& it : SearchPathList) {
					FPaths::NormalizeDirectoryName(it);
					LongPackageName = it + "/" + packageFilePath;
					FPackageName::TryConvertLongPackageNameToFilename(LongPackageName, realFilePath, TEXT(""));
					if (FPaths::FileExists(realFilePath)) {
						//find file, break
						break;
					}
				}
			}

			LoadedT2D = LoadTexture2DImplement(realFilePath, isValid, OutWidth, OutHeight);
		}
	}
	return LoadedT2D;

}

UTexture2D* UHorizonFileSystem::LoadTexture2DImplement(const FString& FullFilePath, bool& OutIsValid, int32& OutWidth, int32& OutHeight)
{
	// code copy from Victory plugin: https://github.com/EverNewJoy/VictoryPlugin
	// thanks Rama!!
	OutIsValid = false;


	UTexture2D* LoadedT2D = nullptr;

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(GetImageFormat(FullFilePath));

	//Load From File
	TArray<uint8> RawFileData;
	if (!FFileHelper::LoadFileToArray(RawFileData, *FullFilePath)) return NULL;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	//Create T2D!
	if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
	{
		const TArray<uint8>* UncompressedBGRA = NULL;
		if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA))
		{
			LoadedT2D = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);

			//Valid?
			if (!LoadedT2D) return NULL;


			//Out!
			OutWidth = ImageWrapper->GetWidth();
			OutHeight = ImageWrapper->GetHeight();

			//Copy!
			void* TextureData = LoadedT2D->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(TextureData, UncompressedBGRA->GetData(), UncompressedBGRA->Num());
			LoadedT2D->PlatformData->Mips[0].BulkData.Unlock();

			//Update!
			LoadedT2D->UpdateResource();
		}
	}

	// Success!
	OutIsValid = true;
	return LoadedT2D;

}



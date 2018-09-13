// Crated by dorgon, All Rights Reserved.
// email: dorgonman@hotmail.com
// blog: dorgon.horizon-studio.net

#pragma once



#include "HorizonFileSystem.generated.h"


class UPaperFlipbook;
UCLASS(BlueprintType, Blueprintable, config = HorizonPlugin, meta = (ShortTooltip = "HorizonFileSystem"))
class HORIZONUI_API UHorizonFileSystem : public UObject {
private:
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|FileSystem", meta = (Keywords = "GetInstance"))
	static UHorizonFileSystem* GetInstance();
	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|FileSystem", meta = (Keywords = "DestroyInstance"))
	static void DestroyInstance();


	//UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|FileSystem", meta = (Keywords = "Set File Search Path"))
	//	void ClearFileSearchPathList();

	//UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|FileSystem", meta = (Keywords = "Add File Search Path"))
	//	void RegisterFileSearchPath(FString InLongPackageName);


	//UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|FileSystem", meta = (Keywords = "Get File Search Path"))
	//	const TArray<FString>& GetFileSearchPathList();

	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|FileSystem", meta = (Keywords = "Create Folder recursively"))
		void CreateDirectoryRecursively(FString FolderToMake);


	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|FileSystem", meta = (Keywords = "LoadTexture2D"))
		UTexture2D* LoadTexture2D(const FString& packageFilePath, int32& OutWidth, int32& OutHeight);
	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|FileSystem", meta = (Keywords = "LoadFlipBook"))
		UPaperFlipbook* LoadPaperFlipbook(const FString& packageFilePath);


	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|FileSystem", meta = (Keywords = "LoadMaterial"))
		UMaterial* LoadMaterial(const FString& packageFilePath);

	//UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|FileSystem", meta = (Keywords = "LoadUserWidget"))
	//	UUserWidget* LoadUserWidget(UWorld* pWorld, const FString& packageFilePath);



	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|FileSystem", meta = (Keywords = "LoadFont"))
		USoundBase* LoadSound(const FString& packageFilePath);



	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|FileSystem", meta = (Keywords = "LoadSound"))
		UFont* LoadFont(const FString& packageFilePath);

	UFUNCTION(BlueprintCallable, Category = "HorizonPlugin|FileSystem", meta = (Keywords = "LoadUAsset"))
		UObject* LoadUAsset(const FString& packageFilePath);
private:
	UTexture2D* LoadTexture2DImplement(const FString& FullFilePath, bool& OutIsValid, int32& OutWidth, int32& OutHeight);
private:
	TArray<FString> SearchPathList;

	static UHorizonFileSystem* Instance;
};
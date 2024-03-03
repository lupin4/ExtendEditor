// Copyright Epic Games, Inc. All Rights Reserved.

#include "MegaManager.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "Debug.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Slate/AdvancedDeletionWidget.h"

#define LOCTEXT_NAMESPACE "FMegaManagerModule"

void FMegaManagerModule::StartupModule()
{
	InitCBMenuExtension();

	RegisterAdvancedDeletionTab();
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}
#pragma region ContentBrowserMenuExtension
void FMegaManagerModule::InitCBMenuExtension()
{
	FContentBrowserModule& ContentBrowserModule = 
	FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	// Referencing the actual array of menu extensions, not a copy
	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleMenuExtender =
	ContentBrowserModule.GetAllPathViewContextMenuExtenders();
	

	// FContentBrowserMenuExtender_SelectedPaths CustomCBMenuDelegate;
	// CustomCBMenuDelegate.BindRaw(this, &FMegaManagerModule::CustomCBMenuExtension);

	// ContentBrowserModuleMenuExtender.Add(CustomCBMenuDelegate);
	
    // In This line we add custom Delegates to the existing Delegates
	ContentBrowserModuleMenuExtender.Add(FContentBrowserMenuExtender_SelectedPaths::
		CreateRaw(this, &FMegaManagerModule::CustomCBMenuExtension));


	
}

// Defines the position of the entry in menu
TSharedRef<FExtender> FMegaManagerModule::CustomCBMenuExtension(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender (new FExtender);
	if(SelectedPaths.Num() > 0)
	{
		MenuExtender->AddMenuExtension(FName("Delete"), // extension hook name, position of where we want to add the menu entry
			EExtensionHook::After, // Insert position
			TSharedPtr<FUICommandList>(), // custom hotkey settings
			FMenuExtensionDelegate::CreateRaw(this, &FMegaManagerModule::AddCBMenuEntry )); // Second binding that defines details of the menu entry
			FolderPathsSelected = SelectedPaths;
    }

	return MenuExtender;
}


//Define details for the custom menu entry
void FMegaManagerModule::AddCBMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Delete Unused Assets")),
		FText::FromString(TEXT("Safely Delete all unused assets within a folder")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FMegaManagerModule::OnDeleteUnusedAssetButtonClicked)
	);

	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Delete Empty Folders")), //Title text for menu entry
		FText::FromString(TEXT("Safely delete all empty folders")), //Tooltip text
		FSlateIcon(),	//Custom icon
		FExecuteAction::CreateRaw(this,&FMegaManagerModule::OnDeleteEmptyFoldersButtonClicked) //The actual function to excute
	);

	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Advanced Deletion")),
		FText::FromString(TEXT("List Assets by speicifc conditions in a new Tab")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FMegaManagerModule::OnAdvancedDeletionButtonClicked)
	);
}

void FMegaManagerModule::OnDeleteUnusedAssetButtonClicked()
{
	// DebugHeader::Print(TEXT("Deleting Unused Assets"), FColor::Red);
	if(FolderPathsSelected.Num()>1)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("You can only do this to one folder"));
		return;
	}

	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);
	//Whether there are assets under selected folder
	if(AssetsPathNames.Num()==0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("No asset found under selected folder"));
		return;
	}

	EAppReturnType::Type ConfirmResult =
	DebugHeader::ShowMsgDialog(EAppMsgType::YesNo,TEXT("A total of ") + FString::FromInt(AssetsPathNames.Num()) + TEXT(" found.\nWoudle you like to procceed?"));

	if(ConfirmResult == EAppReturnType::No) return;

	FixUpRedirectors();
	
	TArray<FAssetData> UnusedAssetsDataArray;

	for(const FString& AssetPathName:AssetsPathNames)
	{
		//Don't touch root folder
		if(AssetPathName.Contains(TEXT("Developers")) ||
		AssetPathName.Contains(TEXT("Collections")) ||
		AssetPathName.Contains(TEXT("__ExternalActors__")) ||
		AssetPathName.Contains(TEXT("__ExternalObjects__")))
		{
		
			{
				continue;
			}

			if(!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) continue;

			TArray<FString> AssetReferencers =
			UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPathName);

			if(AssetReferencers.Num()==0)
			{
				const FAssetData UnusedAssetData = UEditorAssetLibrary::FindAssetData(AssetPathName);
				UnusedAssetsDataArray.Add(UnusedAssetData);
			}
		}

		if(UnusedAssetsDataArray.Num()>0)
		{
			ObjectTools::DeleteAssets(UnusedAssetsDataArray);
		}
		else
		{
			DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("No unused asset found under selected folder"));
		}
	
	}
}

void FMegaManagerModule::OnDeleteEmptyFoldersButtonClicked()
{	
	FixUpRedirectors();

	TArray<FString> FolderPathsArray = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0],true,true);
	uint32 Counter = 0;

	FString EmptyFolderPathsNames;
	TArray<FString> EmptyFoldersPathsArray;

	for(const FString& FolderPath:FolderPathsArray)
	{
		if(FolderPath.Contains(TEXT("Developers"))||
		FolderPath.Contains(TEXT("Collections")) ||
		FolderPath.Contains(TEXT("__ExternalActors__")) ||
		FolderPath.Contains(TEXT("__ExternalObjects__")))
		{
			continue;
		}

		if(!UEditorAssetLibrary::DoesDirectoryExist(FolderPath)) continue;

		if(!UEditorAssetLibrary::DoesDirectoryHaveAssets(FolderPath))
		{
			EmptyFolderPathsNames.Append(FolderPath);
			EmptyFolderPathsNames.Append(TEXT("\n"));

			EmptyFoldersPathsArray.Add(FolderPath);
		}		
	}

	if(EmptyFoldersPathsArray.Num()==0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("No empty folder found under selected folder"),false);
		return;
	}

	EAppReturnType::Type ConfirmResult = DebugHeader::ShowMsgDialog(EAppMsgType::OkCancel,
	TEXT("Empty folders found in:\n") + EmptyFolderPathsNames +TEXT("\nWould you like to delete all?"),false);

	if(ConfirmResult==EAppReturnType::Cancel) return;
	
	for(const FString& EmptyFolderPath : EmptyFoldersPathsArray)
	{
		if (UEditorAssetLibrary::DeleteDirectory(EmptyFolderPath)) {
			++Counter;
		} else {
			DebugHeader::Print(TEXT("Failed to delete " + EmptyFolderPath), FColor::Red);
		}
	}

	if (Counter > 0) {
		DebugHeader::ShowNotifyInfo(TEXT("Successfully deleted ") + FString::FromInt(Counter) + TEXT(" folders"));
	}

}

void FMegaManagerModule::OnAdvancedDeletionButtonClicked()
{
	DebugHeader::Print(TEXT("Advanced Deletion"), FColor::Green);
	FGlobalTabmanager::Get()->TryInvokeTab(FName("Advanced Deletion"));
}

void FMegaManagerModule::FixUpRedirectors()
{

	TArray<UObjectRedirector*> RedirectorsToFixArray;

	FAssetRegistryModule& AssetRegistryModule =
	FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));


	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace("/Game");
	Filter.ClassNames.Emplace("ObjectRedirector");

	TArray<FAssetData> OutRedirectors;
	AssetRegistryModule.Get().GetAssets(Filter,OutRedirectors);

	for(const FAssetData& RedirectorData:OutRedirectors)
	{
		if(UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorData.GetAsset()))
		{
			RedirectorsToFixArray.Add(RedirectorToFix);
		}
	}

	FAssetToolsModule& AssetToolsModule =
	FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

	AssetToolsModule.Get().FixupReferencers(RedirectorsToFixArray);
}






#pragma endregion

#pragma region CustomEditorTab

void FMegaManagerModule::RegisterAdvancedDeletionTab()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("Advanced Deletion"),
	FOnSpawnTab::CreateRaw(this,&FMegaManagerModule::OnSpawnAdvanceDeletionTab))
	.SetDisplayName(FText::FromString(TEXT("Advanced Deletion")));
}

TSharedRef<SDockTab> FMegaManagerModule::OnSpawnAdvanceDeletionTab(const FSpawnTabArgs &SpawnTabArgs)
	{	
		return
		SNew(SDockTab).TabRole(ETabRole::NomadTab)
			[
				SNew(SAdvancedDeletionTab)
				.AssetsDataToStore(GetAllAssetDataUnderSelectedFolder())
			];
	}

TArray<TSharedPtr<FAssetData>> FMegaManagerModule::GetAllAssetDataUnderSelectedFolder()
{
	TArray< TSharedPtr <FAssetData> > AvaiableAssetsData;

	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);

	for(const FString& AssetPathName:AssetsPathNames)
	{
		//Don't touch root folder
		if(AssetPathName.Contains(TEXT("Developers"))||
		AssetPathName.Contains(TEXT("Collections")) ||
		AssetPathName.Contains(TEXT("__ExternalActors__")) ||
		AssetPathName.Contains(TEXT("__ExternalObjects__")))
		{
			continue;
		}

		if(!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) continue;

		const FAssetData Data = UEditorAssetLibrary::FindAssetData(AssetPathName);

		AvaiableAssetsData.Add(MakeShared<FAssetData>(Data));
	}

	return AvaiableAssetsData;
}
	

#pragma endregion


void FMegaManagerModule::ShutdownModule()
	{
		// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
		// we call this function before unloading the module.
	}


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMegaManagerModule, MegaManager)

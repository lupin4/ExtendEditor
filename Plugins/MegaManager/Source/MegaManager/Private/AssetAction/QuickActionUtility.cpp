// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetAction/QuickActionUtility.h"
#include "Debug.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "AssetViewUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Internationalization/Regex.h"


void UQuickActionUtility::DuplicateAssets(int32 NumofAssets)
{
	if (NumofAssets <= 0)
	{
		// Print(TEXT("Invalid number of assets"), FColor::Red);
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Invalid number of assets"));


		
		return;
	}

	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	uint32 Counter = 0;

	for(const FAssetData& SelectedAssetData:SelectedAssetsData)
	{
		for(int32 i = 0; i<NumofAssets; i++)
		{
			const FString SourceAssetPath = SelectedAssetData.GetObjectPathString();
			const FString NewDuplicatedAssetName = SelectedAssetData.AssetName.ToString() + TEXT("_") + FString::FromInt(i+1);
			const FString NewPathName = FPaths::Combine(SelectedAssetData.PackagePath.ToString(),NewDuplicatedAssetName);

			if(UEditorAssetLibrary::DuplicateAsset(SourceAssetPath, NewPathName))
			{	
				UEditorAssetLibrary::SaveAsset(NewPathName,false);
				++Counter;
			}
		}
	}

	if(Counter>0)
	{	
		DebugHeader::ShowNotifyInfo(TEXT("Successfully duplicated " + FString::FromInt(Counter) + " files"));
		/*Print(TEXT("Successfully duplicated " + FString::FromInt(Counter) + " files"),FColor::Green);*/
	}
}

void UQuickActionUtility::AddPrefixes()
{
	TArray<UObject*>SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	uint32 Counter = 0;

	for(UObject* SelectedObject:SelectedObjects)
	{
		if(!SelectedObject) continue;
		const FString* PrefixFound = PrefixMap.Find(SelectedObject->GetClass());

		if(!PrefixFound||PrefixFound->IsEmpty())
		{
			DebugHeader::Print(TEXT("Failed to find prefix for class: ") + SelectedObject->GetClass()->GetName(), FColor::Red);
			continue;
		}
		FString OldName = SelectedObject->GetName();
		if(OldName.StartsWith(*PrefixFound))
		{
			DebugHeader::Print(OldName + TEXT(" Prefix already exists for class! "), FColor::Yellow);
			continue;
			
			
		}
		if(SelectedObject->IsA<UMaterialInstanceConstant>())
		{
			OldName.RemoveFromStart(TEXT("M_"));
			OldName.RemoveFromEnd(TEXT("_inst"));
		}
		const FString NewNameWithPrefix = *PrefixFound + OldName;

		UEditorUtilityLibrary::RenameAsset(SelectedObject, NewNameWithPrefix);
		Counter ++;
	}
	if(Counter>0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully renamed " + FString::FromInt(Counter) + " assets"));
	}
	
}void UQuickActionUtility::RemoveUnusedAssets()
{
	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<FAssetData> UnusedAssetsData;

	FixUpRedirectors();

	for(const FAssetData& SelectedAssetData:SelectedAssetsData)
	{	
		TArray<FString> AssetRefrencers =
		UEditorAssetLibrary::FindPackageReferencersForAsset(SelectedAssetData.ObjectPath.ToString());

		if(AssetRefrencers.Num()==0)
		{
			UnusedAssetsData.Add(SelectedAssetData);
		}
	}

	if(UnusedAssetsData.Num()==0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("No unused asset found among selected assets"),false);
		return;
	}

	const int32 NumOfAssetsDeleted = ObjectTools::DeleteAssets(UnusedAssetsData);

	if(NumOfAssetsDeleted == 0) return;

	DebugHeader::ShowNotifyInfo(TEXT("Successfully deleted " + FString::FromInt(NumOfAssetsDeleted) + TEXT(" unused assets")));
}




void UQuickActionUtility::FixUpRedirectors()
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

void UQuickActionUtility::BatchRenameAssets(const FString& RenamePattern, const FString& ReplacePattern)
{
	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	uint32 Counter = 0;

	for (UObject* SelectedObject : SelectedObjects)
	{
		if (!SelectedObject) continue;

		FString OldName = SelectedObject->GetName();
		FString NewName = OldName.Replace(*RenamePattern, *ReplacePattern, ESearchCase::IgnoreCase);

		// Optional: Check if the new name is actually different to avoid unnecessary operations
		if (OldName.Equals(NewName, ESearchCase::IgnoreCase))
		{
			continue; // Skip if the name would not change
		}

		UEditorUtilityLibrary::RenameAsset(SelectedObject, NewName);
		Counter++;
	}

	if (Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully renamed ") + FString::FromInt(Counter) + TEXT(" assets."));
	}
	else
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No assets were renamed."), false);
	}
}


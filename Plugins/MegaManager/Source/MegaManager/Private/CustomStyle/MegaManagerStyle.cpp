// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomStyle/MegaManagerStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

FName FMegaManagerStyle::StyleSetName = FName("MegaManagerStyle");
TSharedPtr<FSlateStyleSet> FMegaManagerStyle::CreatedSlateStyleSet = nullptr;

void FMegaManagerStyle::InitializeIcons()
{	
	if(!CreatedSlateStyleSet.IsValid())
	{
		CreatedSlateStyleSet = CreateSlateStyleSet();
		FSlateStyleRegistry::RegisterSlateStyle(*CreatedSlateStyleSet);
	}
	
}

TSharedRef<FSlateStyleSet> FMegaManagerStyle::CreateSlateStyleSet()
{	
	TSharedRef<FSlateStyleSet> CustomStyleSet = MakeShareable(new FSlateStyleSet(StyleSetName));

	const FString IconDirectory = 
	IPluginManager::Get().FindPlugin(TEXT("MegaManager"))->GetBaseDir() /"Resources";
	
	CustomStyleSet->SetContentRoot(IconDirectory);

	const FVector2D Icon16x16 (16.f,16.f);

	CustomStyleSet->Set("ContentBrowser.DeleteUnusedAssets",
	new FSlateImageBrush(IconDirectory/"DeleteUnusedAsset.png",Icon16x16));

	CustomStyleSet->Set("ContentBrowser.DeleteEmptyFolders",
	new FSlateImageBrush(IconDirectory/"DeleteEmptyFolders.png",Icon16x16));

	CustomStyleSet->Set("ContentBrowser.AdvanceDeletion",
	new FSlateImageBrush(IconDirectory/"AdvancedDeletion.png",Icon16x16));

	return CustomStyleSet;
}

void FMegaManagerStyle::ShutDown()
{
	if(CreatedSlateStyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*CreatedSlateStyleSet);
		CreatedSlateStyleSet.Reset();
	}
}
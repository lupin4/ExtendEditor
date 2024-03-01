// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Particles/ParticleSystem.h"	
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Engine/Texture.h"
#include "Blueprint/UserWidget.h"
#include "Components/SkeletalMeshComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraEmitter.h"
#include "QuickActionUtility.generated.h"
/**
 * 
 */
UCLASS()
class MEGAMANAGER_API UQuickActionUtility : public UAssetActionUtility
{
	GENERATED_BODY()
public:
	// Make duplicates of Assets
	UFUNCTION(CallInEditor)
	void DuplicateAssets(int32 NumofAssets);
	// Add prefixes and remove trailing _inst to the asset names
	UFUNCTION(CallInEditor)
	void AddPrefixes();
	// Remove Unused Assets
	UFUNCTION(CallInEditor)
	void RemoveUnusedAssets();
	// Declare the BatchRenameAssets function
	UFUNCTION(CallInEditor)
	void BatchRenameAssets(const FString& RenamePattern, const FString& ReplacePattern);



	
private:
	// Selection of Unreal Naming convention Prefixes
	TMap<UClass*,FString>PrefixMap =
		{
		{UBlueprint::StaticClass(),TEXT("BP_")},
		{UStaticMesh::StaticClass(),TEXT("SM_")},
		{UMaterial::StaticClass(), TEXT("M_")},
		{UMaterialInstanceConstant::StaticClass(),TEXT("MI_")},
		{UMaterialFunctionInterface::StaticClass(), TEXT("MF_")},
		{UParticleSystem::StaticClass(), TEXT("PS_")},
		{USoundCue::StaticClass(), TEXT("SC_")},
		{USoundWave::StaticClass(), TEXT("SW_")},
		{UTexture::StaticClass(), TEXT("T_")},
		{UTexture2D::StaticClass(), TEXT("T_")},
		{UUserWidget::StaticClass(), TEXT("WBP_")},
		{USkeletalMeshComponent::StaticClass(), TEXT("SK_")},
		{UNiagaraSystem::StaticClass(), TEXT("NS_")},
		{UNiagaraEmitter::StaticClass(), TEXT("NE_")}
		};
	void FixUpRedirectors();
	
};



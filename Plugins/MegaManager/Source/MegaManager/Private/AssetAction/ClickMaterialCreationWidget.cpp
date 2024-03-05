// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetAction/ClickMaterialCreationWidget.h"
#include "Debug.h"
#include "EditorAssetLibrary.h"
#include "EditorUtilityLibrary.h"
#include "AssetToolsModule.h"
#include "Factories/MaterialFactoryNew.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/Material.h"

#pragma region QuickMaterialCreationCore
	
void UClickMaterialCreationWidget::CreateMaterialFromSelectedTextures()
{
	if(bCustomMaterialName)
	{
		if(MaterialName.IsEmpty() || MaterialName.Equals(TEXT("M_")))
		{
			DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("Please enter a valid name"));
			return;
		}
	}
	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<UTexture2D*> SelectedTexturesArray;
	FString SelectedTextureFolderPath;
	uint32 PinsConnectedCounter = 0;

	if(!ProcessSelectedData(SelectedAssetsData, SelectedTexturesArray, SelectedTextureFolderPath)) return;
	if(CheckIsNameUsed(SelectedTextureFolderPath, MaterialName))return;
	UMaterial* CreatedMaterial = CreateMaterialAsset(MaterialName,SelectedTextureFolderPath);
	if(!CreatedMaterial)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("Failed to create material"));
        return;
	}
	for(UTexture2D* SelectedTexture:SelectedTexturesArray)
	{
		if(!SelectedTexture) continue;
		// switch(ChannelPackingType)
		// {
		// case E_ChannelPackingType::ECPT_NoChannelPacking:
		//
		// 	Default_CreateMaterialNodes(CreatedMaterial,SelectedTexture,PinsConnectedCounter);
		// 	break;
		//
		// case E_ChannelPackingType::ECPT_ORM:
		//
		// 	ORM_CreateMaterialNodes(CreatedMaterial,SelectedTexture,PinsConnectedCounter);
		// 	break;
		//
		// case E_ChannelPackingType::ECPT_MAX:
		// 	break;
		//
		// default:
		// 	break;
		// }
		Default_CreateMaterialNodes(CreatedMaterial,SelectedTexture,PinsConnectedCounter);
	}

	if(PinsConnectedCounter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Connected") + FString::FromInt(PinsConnectedCounter) + TEXT(" pins"));
	}
	// DebugHeader::Print(SelectedTextureFolderPath,FColor::Cyan);

	MaterialName = TEXT("M_");
}


//Process the selected data, will filter out textures,and return false if non-texture selected
bool UClickMaterialCreationWidget::ProcessSelectedData(const TArray<FAssetData>& SelectedDataToProccess, 
TArray<UTexture2D*>& OutSelectedTexturesArray, FString& OutSelectedTexturePackagePath)
{
	if(SelectedDataToProccess.Num()==0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("No texture selected"));
		return false;
	}
	bool bMaterialNameSet = false;
	for(const FAssetData& SelectedData:SelectedDataToProccess)
	{
		UObject* SelectedAsset = SelectedData.GetAsset();
		if(!SelectedAsset) continue;
		UTexture2D* SelectedTexture = Cast<UTexture2D>(SelectedAsset);
		if(!SelectedTexture)
		{
			DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("Please select only textures\n") + 
			SelectedAsset->GetName() + TEXT(" is not a texture"));
			return false;
		}
		OutSelectedTexturesArray.Add(SelectedTexture);
		if(OutSelectedTexturePackagePath.IsEmpty())
		{
			OutSelectedTexturePackagePath = SelectedData.PackagePath.ToString();
		}
		if(!bCustomMaterialName && !bMaterialNameSet)
		{
			MaterialName = SelectedAsset->GetName();
			MaterialName.RemoveFromStart(TEXT("T_"));
			MaterialName.InsertAt(0,TEXT("M_"));
			bMaterialNameSet = true;
		}		
	}
	return true;
}

// Will return true if the Material name is already used in the Specified folder
bool UClickMaterialCreationWidget::CheckIsNameUsed(const FString& FolderPathToCheck,
const FString& MaterialNameToCheck)
{
	TArray<FString> ExistingAssetsPaths = UEditorAssetLibrary::ListAssets(FolderPathToCheck,false);

	for(const FString& ExistingAssetPath:ExistingAssetsPaths)
	{
		const FString ExistingAssetName = FPaths::GetBaseFilename(ExistingAssetPath);

		if(ExistingAssetName.Equals(MaterialNameToCheck))
		{
			DebugHeader::ShowMsgDialog(EAppMsgType::Ok,MaterialNameToCheck + 
			TEXT(" is already used by asset"));

			return true;
		}
	}

	return false;
}

// Creates a new material object
UMaterial* UClickMaterialCreationWidget::CreateMaterialAsset(const FString& NameOfMaterial, const FString& PathToPutMaterial)
{
	FAssetToolsModule& AssetToolsModule = 
	FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

	UMaterialFactoryNew* MaterialFactory = NewObject<UMaterialFactoryNew>();

	UObject* CreatedObject = AssetToolsModule.Get().CreateAsset(NameOfMaterial,  PathToPutMaterial
		,UMaterial::StaticClass(), MaterialFactory);

	return Cast<UMaterial>(CreatedObject);
}

void UClickMaterialCreationWidget::Default_CreateMaterialNodes(UMaterial* CreatedMaterial, UTexture2D* SelectedTexture, uint32& PinsConnectedCounter)
{
	UMaterialExpressionTextureSample* TextureSampleNode = NewObject<UMaterialExpressionTextureSample>(CreatedMaterial, UMaterialExpressionTextureSample::StaticClass());

	if (!TextureSampleNode) return;

	// Conecting the Textures to the Material Node Slots
	if(!CreatedMaterial->HasBaseColorConnected())
	{
		if (TryConnectBaseColor(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}
	if(!CreatedMaterial->HasMetallicConnected())
	{
		if(TryConnectMetallic(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
			
		}
		
	}

	if(!CreatedMaterial->HasRoughnessConnected())
	{
		if (TryConnectRoughness(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}
	if(!CreatedMaterial->HasNormalConnected())
	{
		if(TryConnectNormal(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
			
		}
	}
	// Check and call for Ambient Occlusion
	if (!CreatedMaterial->HasAmbientOcclusionConnected()) // This is a placeholder; actual implementation may vary
	{
		if (TryConnectAO(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
		}
	}
	
	DebugHeader::Print(TEXT("Failed to connect the texture ") + SelectedTexture->GetName(), FColor::Red);
}





#pragma endregion
#pragma region CreateMaterialNodesConnectionPins



bool UClickMaterialCreationWidget::TryConnectBaseColor(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& BaseColorName : BaseColorArray)
	{
		if (SelectedTexture->GetName().Contains(BaseColorName))
		{
			// Set the texture sample node's texture to the selected texture.
			TextureSampleNode->Texture = SelectedTexture;
            
			// Add the texture sample node to the material's expressions.
			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);

			// Attempt to get the base color input expression. Assuming GetExpressionInputForProperty returns an FExpressionInput*.
			FExpressionInput* BaseColorInput = CreatedMaterial->GetExpressionInputForProperty(MP_BaseColor);
			if (BaseColorInput)
			{
				// Connect the texture sample node to the base color input.
				BaseColorInput->Connect(0, TextureSampleNode);
			}

			// Update the material editor's view
			TextureSampleNode->MaterialExpressionEditorX = -600; // Adjust positioning if necessary
			CreatedMaterial->PostEditChange();

			return true;
		}
	}

	return false;
}

bool UClickMaterialCreationWidget::TryConnectMetallic(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
    for (const FString& MetallicName : MetallicArray)
    {
        if (SelectedTexture->GetName().Contains(MetallicName))
        {
            // Adjust the selected texture's properties for metallic use
            SelectedTexture->Modify(); // Ensure the texture is marked for modification
            SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Default;
            SelectedTexture->SRGB = false;
            SelectedTexture->UpdateResource(); // Apply changes to the texture resource
            SelectedTexture->PostEditChange(); // Notify the engine of the change

            // Set the texture sample node's texture to the selected texture
            TextureSampleNode->Texture = SelectedTexture;
            // Set the texture sample to treat the texture as a linear color
            TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_LinearColor;

            // Add the texture sample node to the material's expressions
            CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);

            // Attempt to get the metallic input expression
            FExpressionInput* MetallicInput = CreatedMaterial->GetExpressionInputForProperty(MP_Metallic);
            if (MetallicInput)
            {
                // Connect the texture sample node to the metallic input
                MetallicInput->Connect(0, TextureSampleNode);
            }

            // Adjust the node's position in the material editor
            TextureSampleNode->MaterialExpressionEditorX = -600;
            TextureSampleNode->MaterialExpressionEditorY += 240; // Adjust Y positioning as discussed

            // Notify the engine that the material has been modified
            CreatedMaterial->PreEditChange(nullptr);
            CreatedMaterial->PostEditChange();

            return true;
        }
    }

    return false;
}


// Roughness map Connector
bool UClickMaterialCreationWidget::TryConnectRoughness(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& RoughnessName : RoughnessArray)
	{
		if (SelectedTexture->GetName().Contains(RoughnessName))
		{
			SelectedTexture->Modify();
			SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Default;
			SelectedTexture->SRGB = false;
			SelectedTexture->UpdateResource();
			SelectedTexture->PostEditChange();

			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_LinearColor;

			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);

			FExpressionInput* RoughnessInput = CreatedMaterial->GetExpressionInputForProperty(MP_Roughness);
			if (RoughnessInput)
			{
				RoughnessInput->Connect(0, TextureSampleNode);
			}

			TextureSampleNode->MaterialExpressionEditorX = -600;
			TextureSampleNode->MaterialExpressionEditorY += 440;

			CreatedMaterial->PreEditChange(nullptr);
			CreatedMaterial->PostEditChange();

			return true;
		}
	}

	return false;
}

// Normal Map Connector
bool UClickMaterialCreationWidget::TryConnectNormal(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& NormalName : NormalArray)
	{
		if (SelectedTexture->GetName().Contains(NormalName))
		{
			SelectedTexture->Modify();
			SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Normalmap;
			SelectedTexture->SRGB = false; 
			SelectedTexture->UpdateResource();
			SelectedTexture->PostEditChange();

			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_Normal;

			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);

			FExpressionInput* NormalInput = CreatedMaterial->GetExpressionInputForProperty(MP_Normal);
			if (NormalInput)
			{
				NormalInput->Connect(0, TextureSampleNode);
			}

			TextureSampleNode->MaterialExpressionEditorX = -600;
			TextureSampleNode->MaterialExpressionEditorY += 640;

			CreatedMaterial->PreEditChange(nullptr);
			CreatedMaterial->PostEditChange();

			return true;
		}
	}

	return false;
}


// AmbientOcclusion map Connector

bool UClickMaterialCreationWidget::TryConnectAO(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
    for (const FString& AOName : AmbientOcclusionArray)
    {
        if (SelectedTexture->GetName().Contains(AOName))
        {
            // Adjust the selected texture's properties for AO use
            SelectedTexture->Modify(); // Ensure the texture is marked for modification
            SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Default; // Set the appropriate compression settings
            SelectedTexture->SRGB = false; // Disable sRGB for AO textures
            SelectedTexture->UpdateResource(); // Apply changes to the texture resource
            SelectedTexture->PostEditChange(); // Notify the engine of the change

            // Set the texture sample node's texture to the selected texture
            TextureSampleNode->Texture = SelectedTexture;
            // Set the texture sample to treat the texture as a linear color, appropriate for AO maps
            TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_LinearColor;

            // Add the texture sample node to the material's expressions
            CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);

            // Attempt to get the AO input expression
            FExpressionInput* AOInput = CreatedMaterial->GetExpressionInputForProperty(MP_AmbientOcclusion);
            if (AOInput)
            {
                // Connect the texture sample node to the AO input
                AOInput->Connect(0, TextureSampleNode);
            }

            // Adjust the node's position in the material editor
            TextureSampleNode->MaterialExpressionEditorX = -600; 
            TextureSampleNode->MaterialExpressionEditorY += 840; 

            // Notify the engine that the material has been modified
            CreatedMaterial->PreEditChange(nullptr);
            CreatedMaterial->PostEditChange();

            return true;
        }
    }

    return false;
}

bool UClickMaterialCreationWidget::TryConnectORM(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	// Check if the selected texture name matches the ORM naming convention
	for(const FString& ORM_Name : ORMArray)
	{
		if(SelectedTexture->GetName().Contains(ORM_Name))
		{
			SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Masks;
			SelectedTexture->SRGB = false;
			SelectedTexture->PostEditChange();

			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_Masks;

			// Add the texture sample node to the material's expressions
			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);

			// Mapping each material property to its corresponding texture channel
			TMap<EMaterialProperty, int32> PropertyToChannelMap;
			PropertyToChannelMap.Add(EMaterialProperty::MP_AmbientOcclusion, 1); // Red channel for Ambient Occlusion
			PropertyToChannelMap.Add(EMaterialProperty::MP_Roughness, 2);        // Green channel for Roughness
			PropertyToChannelMap.Add(EMaterialProperty::MP_Metallic, 3);         // Blue channel for Metallic

			// Connect each texture channel to the corresponding material property
			for (const TPair<EMaterialProperty, int32>& Pair : PropertyToChannelMap)
			{
				FExpressionInput* Input = CreatedMaterial->GetExpressionInputForProperty(Pair.Key);
				if (Input)
				{
					Input->Connect(Pair.Value, TextureSampleNode);
				}
			}

			CreatedMaterial->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 960;

			return true;
		}
	}
	return false;
}




#pragma endregion

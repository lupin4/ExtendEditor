#include "Slate/AdvancedDeletionWidget.h"
#include "Debug.h"
#include "MegaManager.h"



#define ListAll	TEXT("List All Available Assets")

#define ListUnused	TEXT("List Unused Assets")

#define ListSameName	TEXT("List Assets with the Same Name ")
void SAdvancedDeletionTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;
	
	StoredAssetsData = InArgs._AssetsDataToStore;
	DisplayedAssetsData = StoredAssetsData;

	
	// Empty the arrays to clear the garbage on Tab Close
	CheckBoxesArray.Empty();
	AssetDataToDeleteArray.Empty();
	ComboBoxSourceItems.Empty();

	ComboBoxSourceItems.Add(MakeShared<FString>(ListAll));
	ComboBoxSourceItems.Add(MakeShared<FString>(ListUnused));
	ComboBoxSourceItems.Add(MakeShared<FString>(ListSameName));
	
	FSlateFontInfo TitleTextFont = GetEmbossedtextFont();

	
	TitleTextFont.Size = 30;

	ChildSlot
	[	//Main vertical box
		SNew(SVerticalBox)

		//First vertical slot for title text
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			
			.Text(FText::FromString(TEXT("Micro Manager")))
			.Font(TitleTextFont)
			.Justification(ETextJustify::Center)
			.ColorAndOpacity(FColor::White)
		]
		
        //Second vertical slot to specify the listing conditions
        +SVerticalBox::Slot()
        .AutoHeight()
        [
        	SNew(SHorizontalBox)
        	// ComboBox Slot
        	+SHorizontalBox::Slot()
			.AutoWidth()
			[
				ConstructComboBox()
			]
			//Help text for Combo Box Slot
			+SHorizontalBox::Slot()
			.FillWidth(.6f)
			[
			ConstructComboHelpText(TEXT("Select the type of assets you wish to list. Left Click to go to Asset in the Content Browser"),ETextJustify::Center)
			]
			//Help text for Folder Path
			+SHorizontalBox::Slot()
			.FillWidth(.3f)
			
			[
            ConstructComboHelpText(TEXT("Current Search Folder:<--------|\n ") + InArgs._CurrentSelectedFolder + ("<--------|"),ETextJustify::Right)
            ]
			
        ]

        
        //Third vertical slot for the asset list
        +SVerticalBox::Slot()
        .VAlign(VAlign_Fill)
        .Padding(.5f)
        
		[
			SNew(SScrollBox)
			
			+SScrollBox::Slot()
			[
				ConstructAssetListView()
			]
		]

        //Fourth vertical slot for the 3 action buttons
        +SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			//Button 1 Slot to delete the selected assets
			+SHorizontalBox::Slot()
			.FillWidth(10.f)
			.Padding(5.f)
			[
				ConstructDeleteAllButton()
                
            ]
            //Button 2 Slot to select all assets
			+SHorizontalBox::Slot()
			.FillWidth(10.f)
			.Padding(5.f)
			[
				ConstructSelectAllButton()
			]
			//Button 3 Slot to deselect all assets
			+SHorizontalBox::Slot()
            .FillWidth(10.f)
            .Padding(5.f)
            [
                ConstructDeSelectAllButton()
            ]
		]
	];	
}

TSharedRef<SListView<TSharedPtr<FAssetData>>> SAdvancedDeletionTab::ConstructAssetListView()
{	
	ConstructedAssetListView = SNew(SListView< TSharedPtr <FAssetData> >)
	.ItemHeight(24.f)
	.ListItemsSource(&DisplayedAssetsData)
	.OnGenerateRow(this,&SAdvancedDeletionTab::OnGenerateRowForList)
	.OnMouseButtonClick(this, &SAdvancedDeletionTab::OnRowWidgetMouseButtonClicked);

	return ConstructedAssetListView.ToSharedRef();
}
void SAdvancedDeletionTab::RefreshAssetListView()
{
	AssetDataToDeleteArray.Empty();
	CheckBoxesArray.Empty();
	
	if (ConstructedAssetListView.IsValid())
	{
		ConstructedAssetListView->RebuildList();
	}
}

#pragma region ComboBoxForListingCondition

TSharedRef<SComboBox<TSharedPtr<FString>>> SAdvancedDeletionTab::ConstructComboBox()
{
	TSharedRef< SComboBox < TSharedPtr <FString > > > ConstructedComboBox =
	SNew(SComboBox < TSharedPtr <FString > >)
	.OptionsSource(&ComboBoxSourceItems)
	.OnGenerateWidget(this,&SAdvancedDeletionTab::OnGenerateComboContent)
	.OnSelectionChanged(this,&SAdvancedDeletionTab::OnComboSelectionChanged)
	[
		SAssignNew(ComboDisplayTextBlock,STextBlock)
		.Text(FText::FromString(TEXT("List Assets Option")))
	];

	return ConstructedComboBox;
}

TSharedRef<SWidget> SAdvancedDeletionTab::OnGenerateComboContent(TSharedPtr<FString> SourceItem)
{	
	TSharedRef <STextBlock> ContructedComboText = SNew(STextBlock)
	.Text(FText::FromString(*SourceItem.Get()));

	return ContructedComboText;
}


void SAdvancedDeletionTab::OnComboSelectionChanged(TSharedPtr<FString> SelectedOption, 
ESelectInfo::Type InSelectInfo)
{
	DebugHeader::Print(*SelectedOption.Get(),FColor::Cyan);

	ComboDisplayTextBlock->SetText(FText::FromString(*SelectedOption.Get()));
	FMegaManagerModule& MegaManagerModule =
		FModuleManager::LoadModuleChecked<FMegaManagerModule>(TEXT("MegaManager"));
	// Pass Data for our Module to Filter
	if(*SelectedOption.Get() == ListAll)
	{
		// List all Stored Assets Data
		DisplayedAssetsData = StoredAssetsData;
		RefreshAssetListView();
	}
	else if (*SelectedOption.Get() == ListUnused)
	{
		// List all Unused Assets Data
		MegaManagerModule.ListUnusedAssetsForAssetList(StoredAssetsData, DisplayedAssetsData);
		RefreshAssetListView();
	}
	else if (*SelectedOption.Get() == ListSameName)
	{
		// List all Assets with the same name Assets Data
        MegaManagerModule.ListSameNameAssetsForAssetList(StoredAssetsData, DisplayedAssetsData);
        RefreshAssetListView();
	}
	
}
TSharedRef<STextBlock> SAdvancedDeletionTab::ConstructComboHelpText(const FString& TextContent,
	ETextJustify::Type TextJustification)
{
	TSharedRef<STextBlock> ConstructedHelpText =
		SNew(STextBlock)
	    .Text(FText::FromString(TextContent))
	    .Justification(TextJustification)
	    .AutoWrapText(true);
	return ConstructedHelpText;
}
#pragma endregion


#pragma region RowWidegtForAssetListView
// UI Builder
TSharedRef<ITableRow> SAdvancedDeletionTab::OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay,
                                                                 const TSharedRef<STableViewBase>& OwnerTable)
{
	// Convert the AssetClassPath to a string
	FString FullPath = AssetDataToDisplay->AssetClassPath.ToString();

	// Split the string by the period character
	TArray<FString> Parsed;
	FullPath.ParseIntoArray(Parsed, TEXT("."), true);

	// The last element of the array will be the class name you're interested in
	FString DisplayAssetClassName = Parsed.Last();
	
	const FString DisplayAssetName = AssetDataToDisplay->AssetName.ToString();

	FSlateFontInfo AssetClassNameFont = GetEmbossedtextFont();
	AssetClassNameFont.Size = 10;


	FSlateFontInfo AssetNameFont = GetEmbossedtextFont();
	AssetNameFont.Size = 15;
	
	TSharedRef<STableRow< TSharedPtr<FAssetData> > > ListViewRowWidget =
		SNew(STableRow < TSharedPtr <FAssetData> >,OwnerTable).Padding(FMargin(.5f))
		[	
			SNew(SHorizontalBox)

			//First slot for check box
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Fill)
			.FillWidth(.05f)
			[
				ConstructCheckBox(AssetDataToDisplay)
			]

			//Second slot for displaying asset class name
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Fill)
			.FillWidth(.5f)
			.Padding(FMargin(0.5f))
			[
				ConstructTextForRowWidget(DisplayAssetClassName, AssetClassNameFont)
				
			]
			//Third slot for displaying asset name
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Fill)
			
			[
				ConstructTextForRowWidget(DisplayAssetName, AssetNameFont)
			]

			//Fourth slot for a button
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Fill)
			[
				ConstructButtonForRowWidget(AssetDataToDisplay)
			]


		];
	return ListViewRowWidget;
	 
}


void SAdvancedDeletionTab::OnRowWidgetMouseButtonClicked(TSharedPtr<FAssetData> ClickedData)
{
	DebugHeader::Print(ClickedData->AssetName.ToString(),FColor::Cyan);
	FMegaManagerModule& MegaManagerModule =
		FModuleManager::LoadModuleChecked<FMegaManagerModule>(TEXT("MegaManager"));

	MegaManagerModule.SyncCBToClickedAssetForAssetList(ClickedData->ObjectPath.ToString());

}
TSharedRef<SCheckBox> SAdvancedDeletionTab::ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{	
	TSharedRef<SCheckBox> ConstructedCheckBox = SNew(SCheckBox)
	.Type(ESlateCheckBoxType::CheckBox)
	.OnCheckStateChanged(this,&SAdvancedDeletionTab::OnCheckBoxStateChanged,AssetDataToDisplay)
	.Visibility(EVisibility::Visible);

	CheckBoxesArray.Add(ConstructedCheckBox);

	return ConstructedCheckBox;
}

void SAdvancedDeletionTab::OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{	
	switch(NewState)
	{
	case ECheckBoxState::Unchecked:

		DebugHeader::Print(AssetData->AssetName.ToString() + TEXT(" is unchecked"), FColor::Red);
		if(AssetDataToDeleteArray.Contains(AssetData))
		{
			AssetDataToDeleteArray.Remove(AssetData);
		}

		break;

	case ECheckBoxState::Checked:

		DebugHeader::Print(AssetData->AssetName.ToString() + TEXT(" is checked"), FColor::Green);
		AssetDataToDeleteArray.AddUnique(AssetData);

		break;

	case ECheckBoxState::Undetermined:
		break;

	default:
		break;
	}

}


TSharedRef<STextBlock> SAdvancedDeletionTab::ConstructTextForRowWidget(const FString& TextContent,
	const FSlateFontInfo& FontToUse)
{
	TSharedRef<STextBlock> ConstructedTextBlock = SNew(STextBlock)
	.Text(FText::FromString(TextContent))
	.Font(FontToUse)
	.ColorAndOpacity(FColor::White);

	return ConstructedTextBlock;
}

TSharedRef<SButton> SAdvancedDeletionTab::ConstructButtonForRowWidget(
	const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	return SNew(SButton)	
	.Text(FText::FromString(TEXT("Delete")))
	.OnClicked(this,&SAdvancedDeletionTab::OnDeleteButtonClicked,AssetDataToDisplay);
		
	
}

FReply SAdvancedDeletionTab::OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData)
{	
	// DebugHeader::Print(ClickedAssetData->AssetName.ToString() + TEXT(" is clicked"), FColor::Green);

	FMegaManagerModule& MegaManagerModule =
		FModuleManager::LoadModuleChecked<FMegaManagerModule>(TEXT("MegaManager"));
	// Destructuring the Array of ClickedAssetData to get a single object
	
	// checking if the action was canceled or if anything is selected
	const bool BAssetDeleted = MegaManagerModule.DeleteSingleAssetForAssetList(ClickedAssetData.Get());

	if (BAssetDeleted)
	{
		//Refresh List
		if (StoredAssetsData.Contains(ClickedAssetData))
		{
			StoredAssetsData.Remove(ClickedAssetData);
		}
		// Refreshes the AssetListView
		if(DisplayedAssetsData.Contains(ClickedAssetData))
		{
			DisplayedAssetsData.Remove(ClickedAssetData);
		}
	}
	// Refresh The list
	return FReply::Handled();
}
#pragma endregion	

#pragma region TabButtons
TSharedRef<SButton> SAdvancedDeletionTab::ConstructDeleteAllButton()
{
	 TSharedRef<SButton> DeleteAllButton = SNew(SButton)
	 .ContentPadding(FMargin(5.f))
	 .OnClicked(this, &SAdvancedDeletionTab::OnDeleteAllButtonClicked );
	 DeleteAllButton->SetContent(ConstructTextForButtons(TEXT("Delete All")));
	 return DeleteAllButton;
	
}

FReply SAdvancedDeletionTab::OnDeleteAllButtonClicked()
{
	// DebugHeader::Print(TEXT("Delete All Clicked"), FColor::Cyan);
	if(AssetDataToDeleteArray.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No assets selected."));
		return FReply::Handled();
		
	}
	TArray<FAssetData> AssetDataToDelete;
	for(const TSharedPtr<FAssetData>& Data : AssetDataToDeleteArray)
	{
		AssetDataToDelete.Add(*Data.Get());
	}
	FMegaManagerModule& MegaManagerModule =
	FModuleManager::LoadModuleChecked<FMegaManagerModule>(TEXT("MegaManager"));


	const bool bAssetsDeleted = MegaManagerModule.DeleteMultipleAssetsForAssetList(AssetDataToDelete);

	if (bAssetsDeleted)
	{
		for(const TSharedPtr<FAssetData>& DeletedData : AssetDataToDeleteArray)
		{
			if(StoredAssetsData.Contains(DeletedData))
			{
				StoredAssetsData.Remove(DeletedData);
			}
			if(DisplayedAssetsData.Contains(DeletedData))
			{
				DisplayedAssetsData.Remove(DeletedData);
			}
				
		}
		RefreshAssetListView();
	}
	return FReply::Handled();
}

TSharedRef<SButton> SAdvancedDeletionTab::ConstructSelectAllButton()
{
	TSharedRef<SButton> SelectAllButton = SNew(SButton)
	 .ContentPadding(FMargin(5.f))
	 .OnClicked(this, &SAdvancedDeletionTab::OnSelectAllButtonClicked );
	SelectAllButton->SetContent(ConstructTextForButtons(TEXT("Select All")));
	return SelectAllButton;
}

FReply SAdvancedDeletionTab::OnSelectAllButtonClicked()
{
	// DebugHeader::Print(TEXT("Select All Clicked"), FColor::Cyan);
	if(CheckBoxesArray.Num() == 0) return FReply::Handled();

	for(const TSharedRef<SCheckBox>& CheckBox : CheckBoxesArray)
	{
		if(!CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState();
		}
	}
	return FReply::Handled();
}

TSharedRef<SButton> SAdvancedDeletionTab::ConstructDeSelectAllButton()
{
	TSharedRef<SButton> DeSelectAllButton = SNew(SButton)
	 .ContentPadding(FMargin(5.f))
	 .OnClicked(this, &SAdvancedDeletionTab::OnDeSelectAllButtonClicked );
	DeSelectAllButton->SetContent(ConstructTextForButtons(TEXT("Deselect All")));
	return DeSelectAllButton;
}

FReply SAdvancedDeletionTab::OnDeSelectAllButtonClicked()
{
	// DebugHeader::Print(TEXT("Deselect All Clicked"), FColor::Cyan);
	if(CheckBoxesArray.Num() == 0) return FReply::Handled();
	for(const TSharedRef<SCheckBox>& CheckBox : CheckBoxesArray)
	{
		if(CheckBox->IsChecked())	
		{
			CheckBox->ToggleCheckedState();
		}
	}
	return FReply::Handled();
}

TSharedRef<STextBlock> SAdvancedDeletionTab::ConstructTextForButtons(const FString& TextContent)
{
	FSlateFontInfo ButtonTextFont = GetEmbossedtextFont();
	ButtonTextFont.Size = 15;
	TSharedRef<STextBlock> ConstructedTextBlock = SNew(STextBlock)
    .Text(FText::FromString(TextContent))
	.Font(ButtonTextFont)
	.Justification(ETextJustify::Center);
    

    return ConstructedTextBlock;
}
#pragma endregion	







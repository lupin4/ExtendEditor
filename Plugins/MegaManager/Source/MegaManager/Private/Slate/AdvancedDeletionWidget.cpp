#include "Slate/AdvancedDeletionWidget.h"
#include "Debug.h"
#include "SlateBasics.h"


void SAdvancedDeletionTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;
	
	StoredAssetsData = InArgs._AssetsDataToStore;
	
	FSlateFontInfo TitleTextFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));

	
	TitleTextFont.Size = 30;

	ChildSlot
	[	//Main vertical box
		SNew(SVerticalBox)

		//First vertical slot for title text
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			
			.Text(FText::FromString(TEXT("Advanced Deletion")))
			.Font(TitleTextFont)
			.Justification(ETextJustify::Center)
			.ColorAndOpacity(FColor::White)
		]
		
        //Second vertical slot to specify the listing conditions
        +SVerticalBox::Slot()
        .AutoHeight()
        [
        	SNew(SHorizontalBox)
        ]

        
        //Third vertical slot for the asset list
        +SVerticalBox::Slot()
        .VAlign(VAlign_Fill)
		[
			SNew(SScrollBox)
			
			+SScrollBox::Slot()
			[
				SNew(SListView< TSharedPtr <FAssetData> >)
				.ItemHeight(24.f)
				.ListItemsSource(&StoredAssetsData)
				.OnGenerateRow(this,&SAdvancedDeletionTab::OnGenerateRowForList)
			]
		]

        //Fourth vertical slot for the 3 action buttons
        +SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
		]
	];	
}

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
			[
				ConstructTextForRowWidget(DisplayAssetClassName, AssetClassNameFont)
				
			]
			//Third slot for displaying asset name
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
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



TSharedRef<SCheckBox> SAdvancedDeletionTab::ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{	
	TSharedRef<SCheckBox> ConstructedCheckBox = SNew(SCheckBox)
	.Type(ESlateCheckBoxType::CheckBox)
	.OnCheckStateChanged(this,&SAdvancedDeletionTab::OnCheckBoxStateChanged,AssetDataToDisplay)
	.Visibility(EVisibility::Visible);

	return ConstructedCheckBox;
}

void SAdvancedDeletionTab::OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{	
	switch(NewState)
	{
	case ECheckBoxState::Unchecked:

		DebugHeader::Print(AssetData->AssetName.ToString() + TEXT(" is unchecked"), FColor::Red);

		break;

	case ECheckBoxState::Checked:

		DebugHeader::Print(AssetData->AssetName.ToString() + TEXT(" is checked"), FColor::Green);

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
	DebugHeader::Print(ClickedAssetData->AssetName.ToString() + TEXT(" is clicked"), FColor::Green);

	return FReply::Handled();
}
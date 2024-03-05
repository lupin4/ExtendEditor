#pragma once

#include "Widgets/SCompoundWidget.h"

class SAdvancedDeletionTab : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SAdvancedDeletionTab) {}
	SLATE_ARGUMENT(TArray<TSharedPtr<FAssetData> >,AssetsDataToStore)

	SLATE_ARGUMENT(FString, CurrentSelectedFolder)

	SLATE_END_ARGS()
	

public:
	void Construct(const FArguments& InArgs);

private:
	TArray<TSharedPtr<FAssetData>> StoredAssetsData;
	TArray<TSharedPtr<FAssetData>> DisplayedAssetsData;
	TArray<TSharedRef<SCheckBox>> CheckBoxesArray;
	TArray<TSharedPtr<FAssetData>> AssetDataToDeleteArray;

	TSharedRef<SListView<TSharedPtr<FAssetData>>> ConstructAssetListView();
	TSharedPtr<SListView<TSharedPtr<FAssetData>>> ConstructedAssetListView;
	void RefreshAssetListView();
#pragma region ComboBoxForListingCondition

	TSharedRef< SComboBox < TSharedPtr <FString> > > ConstructComboBox();

	TArray< TSharedPtr <FString> > ComboBoxSourceItems;

	TSharedRef<SWidget> OnGenerateComboContent(TSharedPtr<FString> SourceItem);

	void OnComboSelectionChanged(TSharedPtr<FString> SelectedOption,ESelectInfo::Type InSelectInfo);

	TSharedPtr<STextBlock> ComboDisplayTextBlock;

	TSharedRef<STextBlock> ConstructComboHelpText(const FString& TextContent, ETextJustify::Type Justification);

#pragma endregion
#pragma region RowWidegtForAssetListView 	
	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay,const TSharedRef<STableViewBase>& OwnerTable);
	void OnRowWidgetMouseButtonClicked(TSharedPtr<FAssetData> ClickedData);
	TSharedRef<SCheckBox> ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay);
	void OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData);

	TSharedRef<STextBlock> ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo& FontToUse);

	TSharedRef<SButton> ConstructButtonForRowWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay);
	FReply OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData);
#pragma endregion


#pragma region TabButtons
	TSharedRef<SButton> ConstructDeleteAllButton();

	TSharedRef<SButton> ConstructSelectAllButton();

	TSharedRef<SButton> ConstructDeSelectAllButton();

	FReply OnDeleteAllButtonClicked();

	FReply OnSelectAllButtonClicked();

	FReply OnDeSelectAllButtonClicked();

	TSharedRef<STextBlock> ConstructTextForButtons(const FString& TextContent);
#pragma endregion TabButtons


	
	FSlateFontInfo GetEmbossedtextFont() const{ return FCoreStyle::Get().GetFontStyle(FName("EmbossedText")); };
};

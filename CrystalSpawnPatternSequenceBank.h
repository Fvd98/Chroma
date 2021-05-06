// Chroma

#pragma once

#include "CoreMinimal.h"
#include "Struct/CrystalSpawnPatternSequence.h"
#include "CrystalSpawnPatternSequenceBank.generated.h"


USTRUCT(BlueprintType)
struct FCrystalSpawnPatternSequenceBank
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bank")
		TArray<FCrystalSpawnPatternSequence> PatternSequenceBank;

	FCrystalSpawnPatternSequenceBank() {
		FLinearColor CRed = FLinearColor::Red;
		FLinearColor CGreen = FLinearColor::Green;
		FLinearColor CBlue = FLinearColor::Blue;
		FLinearColor CCyan = FLinearColor(0, 1, 1);
		FLinearColor CMagenta = FLinearColor(1, 0, 1);
		FLinearColor CYellow = FLinearColor(1, 1, 0);
		FLinearColor CWhite = FLinearColor(1, 1, 1);


		// New tutorial - Red solo
		FCrystalSpawnPatternSequence sequenceTuto_soloRed = FCrystalSpawnPatternSequence
		{
			{
				FCrystalSpawnPattern
				{/*Required*/ CRed, /*Starting*/ {CRed}, /*Ends with*/ {}},
			},
			false
		};

		// New tutorial - Blue solo
		FCrystalSpawnPatternSequence sequenceTuto_soloBlue = FCrystalSpawnPatternSequence
		{
			{
				FCrystalSpawnPattern
				{/*Required*/ CBlue, /*Starting*/ {CBlue}, /*Ends with*/ {}},
			},
			false
		};

		// New tutorial - Green solo
		FCrystalSpawnPatternSequence sequenceTuto_soloGreen = FCrystalSpawnPatternSequence
		{
			{
				FCrystalSpawnPattern
				{/*Required*/ CGreen, /*Starting*/ {CGreen}, /*Ends with*/ {}},
			},
			false
		};

		// New tutorial - Merge
		FCrystalSpawnPatternSequence sequenceTuto_merge = FCrystalSpawnPatternSequence
		{
			{
				FCrystalSpawnPattern
				{/*Required*/ CWhite, /*Starting*/ {CGreen, CBlue, CRed}, /*Ends with*/ {}},
			},
			false
		};

		// New tutorial - Split Red
		FCrystalSpawnPatternSequence sequenceTuto_split = FCrystalSpawnPatternSequence
		{
			{
				FCrystalSpawnPattern
				{/*Required*/ CRed, /*Starting*/ {CWhite}, /*Ends with*/ {CBlue, CGreen}},
				{/*Required*/ CGreen, /*Starting*/ {CBlue, CGreen}, /*Ends with*/ {CBlue}},
				{/*Required*/ CBlue, /*Starting*/ {CBlue}, /*Ends with*/ {}},
			},
			false
		};

		// Sequence tutorial 1 in 3 part
		FCrystalSpawnPatternSequence sequenceTuto_1 = FCrystalSpawnPatternSequence
		{
			{
				// Tutorial 1 part #1
				FCrystalSpawnPattern
				{/*Required*/ CRed, /*Starting*/ {CRed, CCyan}, /*Ends with*/ {CCyan}},

				// Tutorial 1 part #2
				FCrystalSpawnPattern
				{/*Required*/ CBlue, /*Starting*/ {CCyan}, /*Ends with*/ {CGreen}},

				// Tutorial 1 part #3
				FCrystalSpawnPattern
				{/*Required*/ CYellow, /*Starting*/ {CGreen, CRed}, /*Ends with*/ { }}
			},
			false
		};

		// Sequence tutorial 2 in 3 part
		FCrystalSpawnPatternSequence sequenceTuto_2 = FCrystalSpawnPatternSequence
		{
			{
				// Tutorial 2 part #1
				FCrystalSpawnPattern
				{/*Required*/ CGreen, /*Starting*/ {CGreen, CMagenta}, /*Ends with*/ {CMagenta}},

				// Tutorial 2 part #2
				FCrystalSpawnPattern
				{/*Required*/ CRed, /*Starting*/ {CMagenta}, /*Ends with*/ {CBlue}},

				// Tutorial 2 part #3
				FCrystalSpawnPattern
				{/*Required*/ CCyan, /*Starting*/ {CBlue, CGreen}, /*Ends with*/ { }}
			},
			false
		};

		// Sequence tutorial 3 in 3 part
		FCrystalSpawnPatternSequence sequenceTuto_3 = FCrystalSpawnPatternSequence
		{
			{
				// Tutorial 3 part #1
				FCrystalSpawnPattern
				{/*Required*/ CBlue, /*Starting*/ {CBlue, CYellow}, /*Ends with*/ {CYellow}},

				// Tutorial 3 part #2
				FCrystalSpawnPattern
				{/*Required*/ CCyan, /*Starting*/ {CYellow}, /*Ends with*/ {CRed}},

				// Tutorial 3 part #3
				FCrystalSpawnPattern
				{/*Required*/ CMagenta, /*Starting*/ {CRed, CBlue}, /*Ends with*/ { }}
			},
			false
		};

		// Sequence 1 in 3 part
		FCrystalSpawnPatternSequence sequence_1_3Part = FCrystalSpawnPatternSequence
		{
			{
				// Sequence 1 part #1
				FCrystalSpawnPattern
				{/*Required*/ CRed, /*Starting*/ {CMagenta}, /*Ends with*/ {CBlue}},

				// Sequence 1 part #2
				FCrystalSpawnPattern
				{/*Required*/ CCyan, /*Starting*/ {CBlue, CYellow}, /*Ends with*/ {CRed}},

				// Sequence 1 part #3
				FCrystalSpawnPattern
				{/*Required*/ CMagenta, /*Starting*/ {CRed, CBlue}, /*Ends with*/ { }}
			},
			false
		};

		// Sequence 2 in 3 part
		FCrystalSpawnPatternSequence sequence_2_3Part = FCrystalSpawnPatternSequence
		{
			{
				// Sequence 2 part #1
				FCrystalSpawnPattern
				{/*Required*/ CGreen, /*Starting*/ {CCyan}, /*Ends with*/ {CBlue}},

				// Sequence 2 part #2
				FCrystalSpawnPattern
				{/*Required*/ CMagenta, /*Starting*/ {CBlue, CYellow}, /*Ends with*/ {CGreen}},

				// Sequence 2 part #3
				FCrystalSpawnPattern
				{/*Required*/ CCyan, /*Starting*/ {CGreen, CBlue}, /*Ends with*/ { }}
			},
			false
		};

		// Sequence 3 in 3 part
		FCrystalSpawnPatternSequence sequence_3_3Part = FCrystalSpawnPatternSequence
		{
			{
				// Sequence 3 part #1
				FCrystalSpawnPattern
				{/*Required*/ CRed, /*Starting*/ {CYellow}, /*Ends with*/ {CGreen}},

				// Sequence 3 part #2
				FCrystalSpawnPattern
				{/*Required*/ CYellow, /*Starting*/ {CGreen, CMagenta}, /*Ends with*/ {CRed}},

				// Sequence 3 part #3
				FCrystalSpawnPattern
				{/*Required*/ CYellow, /*Starting*/ {CRed, CGreen}, /*Ends with*/ { }}
			},
			false
		};

		// Sequence 4 in 3 part
		FCrystalSpawnPatternSequence sequence_4_3Part = FCrystalSpawnPatternSequence
		{
			{
				// Sequence 4 part #1
				FCrystalSpawnPattern
				{/*Required*/ CCyan, /*Starting*/ {CMagenta, CGreen}, /*Ends with*/ {CRed}},

				// Sequence 4 part #2
				FCrystalSpawnPattern
				{/*Required*/ CYellow, /*Starting*/ {CRed, CCyan}, /*Ends with*/ {CBlue}},

				// Sequence 4 part #3
				FCrystalSpawnPattern
				{/*Required*/ CMagenta, /*Starting*/ {CBlue, CRed}, /*Ends with*/ { }}
			},
			false
		};

		// Sequence 5 in 3 part
		FCrystalSpawnPatternSequence sequence_5_3Part = FCrystalSpawnPatternSequence
		{
			{
				// Sequence 5 part #1
				FCrystalSpawnPattern
				{/*Required*/ CYellow, /*Starting*/ {CCyan, CRed}, /*Ends with*/ {CBlue}},

				// Sequence 5 part #2
				FCrystalSpawnPattern
				{/*Required*/ CMagenta, /*Starting*/ {CBlue, CYellow}, /*Ends with*/ {CGreen}},

				// Sequence 5 part #3
				FCrystalSpawnPattern
				{/*Required*/ CCyan, /*Starting*/ {CGreen, CBlue}, /*Ends with*/ { }}
			},
			false
		};

		// Sequence 6 in 3 part
		FCrystalSpawnPatternSequence sequence_6_3Part = FCrystalSpawnPatternSequence
		{
			{
				// Sequence 6 part #1
				FCrystalSpawnPattern
				{/*Required*/ CMagenta, /*Starting*/ {CYellow, CBlue}, /*Ends with*/ {CGreen}},

				// Sequence 6 part #2
				FCrystalSpawnPattern
				{/*Required*/ CCyan, /*Starting*/ {CGreen, CMagenta}, /*Ends with*/ {CRed}},

				// Sequence 6 part #3
				FCrystalSpawnPattern
				{/*Required*/ CYellow, /*Starting*/ {CRed, CGreen}, /*Ends with*/ { }}
			},
			false
		};

		// Sequence 1 in 4 part
		FCrystalSpawnPatternSequence sequence_1_4Part = FCrystalSpawnPatternSequence
		{
			{
				// Sequence 1 part #1
				FCrystalSpawnPattern
				{/*Required*/ CMagenta, /*Starting*/ {CYellow, CCyan}, /*Ends with*/ {CGreen, CGreen}},

				// Sequence 1 part #2
				FCrystalSpawnPattern
				{/*Required*/ CYellow, /*Starting*/ {CGreen, CGreen, CMagenta}, /*Ends with*/ {CGreen, CBlue}},

				// Sequence 1 part #3
				FCrystalSpawnPattern
				{/*Required*/ CCyan, /*Starting*/ {CGreen, CBlue, CYellow}, /*Ends with*/ {CRed, CGreen}},

				// Sequence 1 part #4
				FCrystalSpawnPattern
				{/*Required*/ CWhite, /*Starting*/ {CRed, CGreen, CBlue}, /*Ends with*/ { }}
			},
			false
		};

		// Sequence 2 in 4 part
		FCrystalSpawnPatternSequence sequence_1_5Part = FCrystalSpawnPatternSequence
		{
			{
				// Sequence 2 part #1
				FCrystalSpawnPattern
				{/*Required*/ CCyan, /*Starting*/ {CYellow, CMagenta}, /*Ends with*/ {CRed, CRed}},

				// Sequence 2 part #2
				FCrystalSpawnPattern
				{/*Required*/ CYellow, /*Starting*/ {CRed, CRed, CCyan}, /*Ends with*/ {CRed, CBlue}},

				// Sequence 2 part #3
				FCrystalSpawnPattern
				{/*Required*/ CWhite, /*Starting*/ {CRed, CBlue, CYellow}, /*Ends with*/ {CRed}},

				// Sequence 2 part #4
				FCrystalSpawnPattern
				{/*Required*/ CMagenta, /*Starting*/ {CRed, CCyan}, /*Ends with*/ {CGreen}},

				// Sequence 2 part #4
				FCrystalSpawnPattern
				{/*Required*/ CWhite, /*Starting*/ {CGreen, CRed, CBlue}, /*Ends with*/ { }}
			},
			false
		};

		// Sequence 3 in 4 part
		FCrystalSpawnPatternSequence sequence_3_4Part = FCrystalSpawnPatternSequence
		{
			{
				// Sequence 3 part #1
				FCrystalSpawnPattern
				{/*Required*/ CYellow, /*Starting*/ {CCyan, CMagenta}, /*Ends with*/ {CBlue, CBlue}},

				// Sequence 3 part #2
				FCrystalSpawnPattern
				{/*Required*/ CCyan, /*Starting*/ {CBlue, CBlue, CYellow}, /*Ends with*/ {CRed, CBlue}},

				// Sequence 3 part #3
				FCrystalSpawnPattern
				{/*Required*/ CCyan, /*Starting*/ {CRed, CBlue, CYellow}, /*Ends with*/ {CRed, CBlue}},

				// Sequence 3 part #4
				FCrystalSpawnPattern
				{/*Required*/ CWhite, /*Starting*/ {CRed, CGreen, CBlue}, /*Ends with*/ { }}
			},
			false
		};

		// Sequence 4 in 4 part
		FCrystalSpawnPatternSequence sequence_Cristal_Blue_4Part = FCrystalSpawnPatternSequence
		{
			{
				// Sequence 3 part #1
				FCrystalSpawnPattern
				{/*Required*/ CYellow, /*Starting*/ {CWhite}, /*Ends with*/ {CBlue}},

				// Sequence 3 part #2
				FCrystalSpawnPattern
				{/*Required*/ CMagenta, /*Starting*/ {CBlue, CYellow, CCyan}, /*Ends with*/ {CGreen, CGreen, CBlue}},

				// Sequence 3 part #3, 2nd Cristal Blue
				FCrystalSpawnPattern
				{/*Required*/ CWhite, /*Starting*/ {CGreen, CGreen, CBlue, CMagenta}, /*Ends with*/ {CGreen}},

				// Sequence 3 part #4
				FCrystalSpawnPattern
				{/*Required*/ CWhite, /*Starting*/ {CRed, CGreen, CBlue}, /*Ends with*/ { }}
			},
			false
		};

		// Sequence 4 in 4 part
		FCrystalSpawnPatternSequence sequence_Cristal_Red_4Part = FCrystalSpawnPatternSequence
		{
			{
				// Sequence 3 part #1
				FCrystalSpawnPattern
				{/*Required*/ CCyan, /*Starting*/ {CWhite}, /*Ends with*/ {CRed}},

				// Sequence 3 part #2
				FCrystalSpawnPattern
				{/*Required*/ CMagenta, /*Starting*/ {CRed, CMagenta, CCyan}, /*Ends with*/ {CBlue, CBlue, CRed}},

				// Sequence 3 part #3, 2nd Cristal Red
				FCrystalSpawnPattern
				{/*Required*/ CWhite, /*Starting*/ {CBlue, CBlue, CRed, CYellow}, /*Ends with*/ {CBlue}},

				// Sequence 3 part #4
				FCrystalSpawnPattern
				{/*Required*/ CWhite, /*Starting*/ {CRed, CGreen, CBlue}, /*Ends with*/ { }}
			},
			false
		};

		// Sequence 4 in 4 part
		FCrystalSpawnPatternSequence sequence_Cristal_Green_4Part = FCrystalSpawnPatternSequence
		{
			{
				// Sequence 3 part #1
				FCrystalSpawnPattern
				{/*Required*/ CMagenta, /*Starting*/ {CWhite}, /*Ends with*/ {CGreen}},

				// Sequence 3 part #2
				FCrystalSpawnPattern
				{/*Required*/ CMagenta, /*Starting*/ {CGreen, CMagenta, CYellow}, /*Ends with*/ {CRed, CRed, CGreen}},

				// Sequence 3 part #3, 2nd Cristal Green
				FCrystalSpawnPattern
				{/*Required*/ CWhite, /*Starting*/ {CRed, CRed, CGreen, CCyan}, /*Ends with*/ {CRed}},

				// Sequence 3 part #4
				FCrystalSpawnPattern
				{/*Required*/ CWhite, /*Starting*/ {CRed, CGreen, CBlue}, /*Ends with*/ { }}
			},
			false
		};

		// Sequence Cristal 1
		FCrystalSpawnPatternSequence sequence_Cristal_l = FCrystalSpawnPatternSequence
		{
			{
				// Sequence part #1
				FCrystalSpawnPattern
				{/*Required*/ CGreen, /*Starting*/ {CCyan}, /*Ends with*/ {CBlue}},

				// Sequence part #2
				FCrystalSpawnPattern
				{/*Required*/ CMagenta, /*Starting*/ {CBlue, CYellow}, /*Ends with*/ {CGreen}},

				// Sequence part #3, 2nd Cristal Green
				FCrystalSpawnPattern
				{/*Required*/ CCyan, /*Starting*/ {CGreen, CBlue}, /*Ends with*/ { }}
			},
			false
		};

		// Sequence Cristal 1
		FCrystalSpawnPatternSequence sequence_Cristal_2 = FCrystalSpawnPatternSequence
		{
			{
				// Sequence part #1
				FCrystalSpawnPattern
				{/*Required*/ CYellow, /*Starting*/ {CCyan, CMagenta}, /*Ends with*/ {CBlue, CBlue}},

				// Sequence part #2
				FCrystalSpawnPattern
				{/*Required*/ CMagenta, /*Starting*/ {CBlue, CBlue , CYellow}, /*Ends with*/ {CGreen, CBlue}},

				// Sequence part #3, 2nd Cristal Green
				FCrystalSpawnPattern
				{/*Required*/ CCyan, /*Starting*/ {CGreen, CBlue}, /*Ends with*/ { }}
			},
			false
		};

		// Sequence Cristal 1
		FCrystalSpawnPatternSequence sequence_Cristal_3 = FCrystalSpawnPatternSequence
		{
			{
				// Sequence part #1
				FCrystalSpawnPattern
				{/*Required*/ CMagenta, /*Starting*/ {CWhite}, /*Ends with*/ {CGreen}},

				// Sequence part #2
				FCrystalSpawnPattern
				{/*Required*/ CYellow, /*Starting*/ {CGreen, CMagenta}, /*Ends with*/ {CBlue}},

				// Sequence part #3, 2nd Cristal Green
				FCrystalSpawnPattern
				{/*Required*/ CWhite, /*Starting*/ {CRed, CGreen, CBlue}, /*Ends with*/ { }}
			},
			false
		};


		PatternSequenceBank = 
		{
			sequenceTuto_soloRed,
			sequenceTuto_soloBlue,
			sequenceTuto_soloGreen,
			sequenceTuto_merge,
			sequenceTuto_split,
			sequence_Cristal_l,
			sequence_Cristal_2,
			sequence_Cristal_3
		};
	}
};


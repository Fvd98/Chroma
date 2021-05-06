// Chroma

#pragma once

#include "CoreMinimal.h"
#include "CrystalSpawnPattern.generated.h"


USTRUCT(BlueprintType)
struct FCrystalSpawnPattern
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
	FLinearColor RequiredColor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
	TArray<FLinearColor> StartingColor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
	TArray<FLinearColor> EndingColor;

	FCrystalSpawnPattern(FLinearColor _RequiredColor, TArray<FLinearColor> _StartingColor, TArray<FLinearColor> _EndingColor) {
		RequiredColor = _RequiredColor;
		StartingColor = _StartingColor;
		EndingColor = _EndingColor;
	}

	FCrystalSpawnPattern() {
		RequiredColor = FLinearColor::Black;
		StartingColor = {};
		EndingColor = {};
	}
};


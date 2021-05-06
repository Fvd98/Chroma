// Chroma

#pragma once

#include "CoreMinimal.h"
#include "Struct/CrystalSpawnPattern.h"
#include "CrystalSpawnPatternSequence.generated.h"


USTRUCT(BlueprintType)
struct FCrystalSpawnPatternSequence
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequence")
		TArray<FCrystalSpawnPattern> PatternArray;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequence")
		bool bIsActive;

	FCrystalSpawnPatternSequence(TArray<FCrystalSpawnPattern> _PatternArray, bool _bIsActive) {
		PatternArray = _PatternArray;
		bIsActive = _bIsActive;
	}

	FCrystalSpawnPatternSequence() {
		PatternArray = {};
		bIsActive = false;
	}
};

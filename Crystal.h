// Chroma

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Ennemies/Wisp.h"
#include "Actors/CrystalSpawner.h"
#include "Actors/CrystalGate.h"
#include "Struct/CrystalSpawnPattern.h"
#include "Struct/CrystalSpawnPatternSequenceBank.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystemComponent.h"

#include "Crystal.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWispAbsorbedDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCrystalCompletedDelegate, ACrystal*, Crystal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCrystalExplosionDelegate, ACrystal*, Crystal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCrystalExplosionCompleteDelegate, ACrystal*, Crystal);

UCLASS()
class CHROMA_API ACrystal : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACrystal();
	
	// Base
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int CrystalIndex;

	// Crystal completion | Wisp spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crystal")
		TArray<TSubclassOf<AWisp>> WispSpawnClasses;

	// Spawners | Gates
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Crystal Setup")
		TArray<ACrystalSpawner*> Spawners;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crystal Setup")
		bool bSpawnerRoundRobin;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
		int CurrentSpawnerRoundRobinIndex;
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Crystal Setup")
		TArray<ACrystalGate*> ConnectedGates;

	// Connected wisps
	TMap<FLinearColor, TArray<AWisp*>> RemainingWisps;
	UFUNCTION()
		void RegisterSpawnedWisp(AWisp* newWisp);

	// Pattern management
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Crystal")
		FCrystalSpawnPatternSequenceBank CrystalSpawnPatternBank;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Crystal")
		FCrystalSpawnPatternSequence CrystalSelectedSequence;
	TMap<FLinearColor, TArray<FCrystalSpawnPattern>> CrystalSpawnPatternBankStartingRelationship;
	void InitCrystalSpawnPatternBank();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crystal")
		bool bFollowPatternBankOrder;
	int FollowPatternBankOrderIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crystal")
		FCrystalSpawnPattern CurrentlySelectedCrystalSpawnPattern;

	UFUNCTION(BlueprintCallable)
		void SetupNextPattern();
	UFUNCTION(BlueprintCallable)
		FCrystalSpawnPattern FindNextPattern();
	UFUNCTION(BlueprintCallable)
		bool PatternsCompatible(FCrystalSpawnPattern LastPattern, FCrystalSpawnPattern NextPattern);

	// Crystal color management
	UPROPERTY(ReplicatedUsing = OnRep_CurrentRequiredWispColor, VisibleAnywhere, BlueprintReadOnly, Category = "Crystal")
		FLinearColor CurrentRequiredWispColor;
	UFUNCTION()
		void OnRep_CurrentRequiredWispColor();
	UFUNCTION(BlueprintCallable)
		void Set_CurrentRequiredWispColor(FLinearColor newCurrentRequiredWispColor);
	UFUNCTION(NetMulticast, Reliable)
		void Mutli_UpdateMaterialCrystalCompletionRPC();
		void Mutli_UpdateMaterialCrystalCompletionRPC_Implementation();

	// All Radius | Progress LERPs
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crystal")
		float BaseRadius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crystal")
		float MaxRadius;
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Crystal")
		float CurrentRadiusPercent;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Crystal")
		float CurrentRadiusPercentAlpha;
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Crystal")
		float TargetRadiusPercent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crystal")
		float RadiusUnitLERPSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crystal")
		float CompletionRadiusUnitLERPSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crystal")
		float CompletionRadiusLERPCurvePower;

	// Progress management
	UPROPERTY(ReplicatedUsing = OnRep_CompletedPatternsCount, EditAnywhere, BlueprintReadWrite, Category = "Crystal")
		int CompletedPatternsCount;
	UFUNCTION(BlueprintCallable)
		void Increase_CompletedPatternsCount();
	UFUNCTION()
		void OnRep_CompletedPatternsCount();

	// Completion explosion
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Crystal Setup")
		bool bExplode;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crystal Setup")
		float ExplosionDelay;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crystal Setup")
		TArray<FLinearColor> ExplosionColorRelevancy;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crystal Setup")
		float ExplosionRelevancyRange;
	UFUNCTION()
		void Explode();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crystal")
		int RequiredPatternComplete;
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Crystal")
		bool bCompleted;

	// Components
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crystal")
		UStaticMeshComponent* CrystalStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crystal")
		UParticleSystemComponent* CrystalParticleSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crystal")
		USphereComponent* WispAbsorbtionHitSphere;


	// Events
	UPROPERTY(BlueprintAssignable)
		FWispAbsorbedDelegate OnWispAbsorbed;

	UPROPERTY(BlueprintAssignable)
		FCrystalCompletedDelegate OnCrystalComplete;	
		
	UPROPERTY(BlueprintAssignable)
		FCrystalExplosionDelegate OnCrystalExplosion;	
		
	UPROPERTY(BlueprintAssignable)
		FCrystalExplosionDelegate OnCrystalExplosionComplete;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Dynamic materials
	UMaterialInstanceDynamic* CrystalColorDynamicMaterial;
	UMaterialInstanceDynamic* CrystalPedestalDynamicMaterial;

	// Easy access color constants
	FLinearColor CRed = FLinearColor::Red;
	FLinearColor CGreen = FLinearColor::Green;
	FLinearColor CBlue = FLinearColor::Blue;
	FLinearColor CCyan = FLinearColor(0, 1, 1);
	FLinearColor CMagenta = FLinearColor(1, 0, 1);
	FLinearColor CYellow = FLinearColor(1, 1, 0);
	FLinearColor CWhite = FLinearColor(1, 1, 1);

	virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITORONLY_DATA
	void OnObjectSelected(UObject* Object);
#endif

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Absorption management
	UFUNCTION(BlueprintCallable)
		void TryAbsorbWisp(AWisp* ToAbsorbWisp);
	UFUNCTION(NetMulticast, Reliable)
		void Multi_WispAbsorbedRPC(FLinearColor AbsorbedWispColor);
		void Multi_WispAbsorbedRPC_Implementation(FLinearColor AbsorbedWispColor);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crystal")
		UNiagaraSystem* AbsorbtionNiagaraSystem;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crystal")
		FVector MergeNiagaraSystemOffSet;

	UFUNCTION()
		void OnWispAbsorbtionHitSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};

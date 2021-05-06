// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Struct/FColorMap.h"
#include "Engine/TargetPoint.h"
#include "Grabbable.h"
#include "Components/MagicRopeMediator.h"
#include "Components/SphereComponent.h"
#include "Components/InteractableComponent.h"
#include "NiagaraSystem.h"
#include "Wisp.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDestroyedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnColorChangeDelegate, FLinearColor, Color);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWispSplitSpawnDelegate, AWisp*, newWisp);

UCLASS()
class CHROMA_API AWisp : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AWisp();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Split")
		TSubclassOf<AWisp> SplitSpawnClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Merge")
		bool bMerging;

	UPROPERTY()
		TArray<UMagicRopeMediator*> InitialAttachFrom;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
		class UWidgetComponent* WidgetComponent;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
		class UWidgetComponent* WidgetExclamationComponent;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
		UInteractableComponent* InteractableComponent;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
		FOnDestroyedDelegate OnDestroyedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
		FOnColorChangeDelegate OnColorChange;	

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
		FWispSplitSpawnDelegate OnWispSplitSpawn;

	//Sight Properties 
	UPROPERTY(EditAnywhere, Category = "Behavior|Vision")
		float SightRadius = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "Behavior|Vision")
		float PeripheralVisionAngleDegrees = 120.0f;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Behavior|Movement", meta = (AllowPrivateAccess = "true"))
		FVector HomePoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior|Movement")
		float SearchRadius = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Behavior|Movement")
		float MaxEndurance = 100;

	UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, Category = "Behavior|Movement")
		float Endurance = 100;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Behavior|Movement")
		float SprintEnduranceCost = 40;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Behavior|Movement")
		float RegenEnduranceGain = 30;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Behavior|Movement")
		float SprintCharacterStrength = 20;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Behavior|Movement")
		float RegenCharacterStrength = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Behavior|Movement")
		bool InRegen = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MaterialParam")
		float BaseFacialExpressionIndex = 0;

	UFUNCTION(NetMulticast, reliable, BlueprintCallable)
	void Multi_ToggleDashHUDRPC();
		void Multi_ToggleDashHUDRPC_Implementation();

	UPROPERTY(ReplicatedUsing = OnRep_WispColor, VisibleAnywhere, BlueprintReadOnly, Category = "Behavior|Color")
		FLinearColor WispColor;
	UFUNCTION(BlueprintCallable)
		void SetWispColor(FLinearColor newWispColor);
	UFUNCTION(Server, Reliable)
		void Server_SetWispColorRPC(FLinearColor newWispColor);
		void Server_SetWispColorRPC_Implementation(FLinearColor newWispColor);
	UFUNCTION()
		void OnRep_WispColor();

	UPROPERTY(ReplicatedUsing = OnRep_WispColorTarget, VisibleAnywhere, BlueprintReadOnly, Category = "Behavior|Color")
		FLinearColor WispColorTarget;
	UFUNCTION(BlueprintCallable)
		void SetWispColorTarget(FLinearColor newWispColorTarget);
	UFUNCTION()
		void OnRep_WispColorTarget();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior|Color")
		float WispColorLERPSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior|Color")
		float WispColorMissingLERPSpeed;

	UPROPERTY(ReplicatedUsing = OnRep_WispColorCurrent, VisibleAnywhere, BlueprintReadOnly, Category = "Behavior|Color")
		FLinearColor WispColorCurrent;
	UFUNCTION(BlueprintCallable)
		void SetWispColorCurrent(FLinearColor newWispColorCurrent);
	UFUNCTION(Server, Reliable)
		void Server_SetWispColorCurrentRPC(FLinearColor newWispColorCurrent);
		void Server_SetWispColorCurrentRPC_Implementation(FLinearColor newWispColorCurrent);
	UFUNCTION()
		void OnRep_WispColorCurrent();

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Behavior|Color")
		bool bWispColorLocked = false;
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Behavior|Color")
		bool bWispColorTargetReachedOnce = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior|Color")
		bool bWispPulseBackDown = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior|Movement")
		ATargetPoint* crystalLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UGrabbable* GrabbableComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UMagicRopeMediator* MagicRopeMediatorComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Color")
		UMaterialInstanceDynamic* WispColorMaterialCore;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Color")
		UMaterialInstanceDynamic* WispColorMaterialAura;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
		void Split();	
	UFUNCTION(BlueprintCallable)
		void DebugSplit();
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UNiagaraSystem* SplitNiagaraSystem;

	UFUNCTION(BlueprintCallable)
		void Merge(AWisp* OtherWisp);
	UFUNCTION(BlueprintCallable)
		bool CanMerge(AWisp* OtherWisp);
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UNiagaraSystem* MergeNiagaraSystem;

	UFUNCTION(NetMulticast, Reliable)
	void Multi_SpawnParticleAtActorLocationRPC(UNiagaraSystem* NiagaraSystem, FLinearColor Color1, FLinearColor Color2);
		void Multi_SpawnParticleAtActorLocationRPC_Implementation(UNiagaraSystem* NiagaraSystem, FLinearColor Color1, FLinearColor Color2);
};

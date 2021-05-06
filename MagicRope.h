// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CableComponent.h"
#include "GameFramework/PlayerController.h"
#include "MagicRope.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRopeBrokenDelegate, AMagicRope*, Rope);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVirationChangeDelegate, TEnumAsByte<EDynamicForceFeedbackAction::Type>, ActionVibration, float, Intensity);

// Forward Declaration class AMagicRope
class UMagicRopeMediator;

UCLASS()
class CHROMA_API AMagicRope : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMagicRope();

	UPROPERTY(BlueprintAssignable, Category = "Feedback")
		FOnVirationChangeDelegate OnVibrationChange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
		TSubclassOf<UMatineeCameraShake> TensionMatineeCameraShake;

	UFUNCTION(Server, Reliable)
	void Server_BreakRopeRPC();
		void Server_BreakRopeRPC_Implementation();

	UFUNCTION(NetMulticast, Reliable)
	void Multi_BreakRopeRPC();
		void Multi_BreakRopeRPC_Implementation();

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Tension")
		float LastCurrentTensionRatio;
	UPROPERTY(ReplicatedUsing = OnRep_CurrentTensionRatio, VisibleAnywhere, BlueprintReadOnly, Category = "Tension")
		float CurrentTensionRatio;
	UFUNCTION()
		void OnRep_CurrentTensionRatio();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY()
	float TOWMaxWalkSpeed;

	UPROPERTY()
	UMaterialInstanceDynamic* RopeDynamicMaterial;

	UPROPERTY()
	FLinearColor CenterColor;

	UPROPERTY()
	FLinearColor EdgeColor;
	
	bool bIsTensionShaking;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Rope")
		UMagicRopeMediator* Mediator1;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Rope")
		UMagicRopeMediator* Mediator2;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rope")
		UCableComponent* RopeCable1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rope")
		float TensionlessLenght;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rope")
		float MaxLenghtRatio;

	UPROPERTY(BlueprintAssignable, Category = "Rope")
		FRopeBrokenDelegate OnRopeBreak;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rope")
		UParticleSystem* RopeParticleSystem;

	UFUNCTION(BlueprintCallable, Category = "Rope")
		void BreakRope();

	UFUNCTION(BlueprintCallable, Category = "Rope")
		void CalculateTOWUpdate();
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Chroma/Public/Actors/MagicRope.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Chroma/Public/ChromaPlayerController.h"
#include "MagicRopeMediator.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTOWUpdateDelegate, UMagicRopeMediator*, Mediator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTOWRepositionDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTOWStartDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTOWEndDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRopeDetachDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPressureTriggerDelegate);


UCLASS( ClassGroup=(Custom), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent)) 
class CHROMA_API UMagicRopeMediator : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMagicRopeMediator();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MagicRopeMediator")
		int CharacterStrength;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MagicRopeMediator")
		float CharacterWeightMultiplier;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MagicRopeMediator")
		float TOWPriority;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MagicRopeMediator|Pressure")
		float CurrentPressure;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MagicRopeMediator|Pressure")
		float PressureTreshold; 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MagicRopeMediator|Pressure")
		float PressureTresholdDifficulty;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MagicRopeMediator|Pressure")
		float PressureCounter;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MagicRopeMediator|Pressure")
		float PressureCounterTrigger;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MagicRopeMediator|Pressure")
		float PressureCounterIncrement;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MagicRopeMediator|Pressure")
		float PressureCounterDecrement;	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MagicRopeMediator|Pressure")
		int PressureMinimumConnectorPointCount;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category= "MagicRopeMediator|Misc")
		bool bAlwaysFaceOpponent;

	UPROPERTY()
		FVector LastInputVector;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY()
		bool bCalculatesTOWU;

	UPROPERTY()
		FVector VolontaryMovementVector;
	UPROPERTY()
		FVector InputVector;
	
	UPROPERTY()
		TMap<AMagicRope*, FVector> PressureVectors;

	UPROPERTY()
		float TOWMaxWalkSpeed;
	UPROPERTY()
		float InitialMaxWalkSpeed;
	UPROPERTY()
		float InitialMaxAcceleration;
	UPROPERTY()
		float InitialBrakingDecelerationWalking;
	UPROPERTY()
		float TOWMaxMSRatio;

	UPROPERTY()
		UCharacterMovementComponent* OwnerCharacterMovementComponent;

	UFUNCTION(Server, Reliable)
	void Server_AttachRopeRPC(UMagicRopeMediator* OtherMediator);
		void Server_AttachRopeRPC_Implementation(UMagicRopeMediator* OtherMediator);

	UFUNCTION()
	void SpawnRopeHandle(UMagicRopeMediator* OtherMediator);

	UFUNCTION()
	void PlayerControllerChangeHandler(AChromaPlayerController* ChromaPlayerController);

	UFUNCTION()
	void OwnerDestroyedHandler(AActor* DestroyedActor);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RopeMediator")
	bool bTOW;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RopeMediator")
		TMap<UMagicRopeMediator*, AMagicRope*> Ropes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RopeMediator")
		TSubclassOf<AMagicRope> RopeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RopeMediator")
		FName SocketName;	
		
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RopeMediator")
		FLinearColor RopeOriginColor;

	UFUNCTION(BlueprintCallable, Category = "RopeMediator")
		void CalculateTOWUpdate();

	UFUNCTION(BlueprintCallable, Category = "RopeMediator")
		void PlayerMovementHandle();

	UFUNCTION(BlueprintCallable, Category = "RopeMediator")
		void AIMovementHandle(UPathFollowingComponent* PathFollowingComponent, FVector& AIMovementVector);

	UPROPERTY(BlueprintAssignable, Category = "RopeMediator")
		FTOWRepositionDelegate OnTOWReposition;

	UPROPERTY(BlueprintAssignable, Category = "RopeMediator")
		FTOWUpdateDelegate OnTOWUpdate;

	UPROPERTY(BlueprintAssignable, Category = "RopeMediator")
		FTOWStartDelegate OnTOWStart;

	UPROPERTY(BlueprintAssignable, Category = "RopeMediator")
		FTOWStartDelegate OnTOWEnd;

	UPROPERTY(BlueprintAssignable, Category = "RopeMediator")
		FRopeDetachDelegate OnRopeDetach;	

	UPROPERTY(BlueprintAssignable, Category = "MagicRopeMediator|Pressure")
		FPressureTriggerDelegate OnPressureTrigger;

	UFUNCTION(BlueprintCallable, Category = "RopeMediator")
		void AttachRope(UMagicRopeMediator* OtherMediator);

	UFUNCTION(BlueprintCallable, Category = "RopeMediator")
		void DetachRope(UMagicRopeMediator* OtherMediator);

	UFUNCTION(BlueprintCallable, Category = "RopeMediator")
		void DetachAllRope();

	UFUNCTION()
		void RopeBrokenHandle(AMagicRope* Rope);

	UFUNCTION(NetMulticast, Reliable)
	void Multi_RopeBrokenHandleRPC(AMagicRope* Rope);
		void Multi_RopeBrokenHandleRPC_Implementation(AMagicRope* Rope);

	UFUNCTION(NetMulticast, Reliable)
	void Multi_AddInputVectorRPC(FVector AddedInputVector, AMagicRope* NullablePressureRope = nullptr);
		void Multi_AddInputVectorRPC_Implementation(FVector AddedInputVector, AMagicRope* NullablePressureRope = nullptr);

	UFUNCTION(Category = "RopeMediator")
		void TOWStart();

	UFUNCTION(Category = "RopeMediator")
		void TOWEnd();

	UFUNCTION(Category = "InputVectorUtils")
		FVector VectorToTOWInputVector(FVector VectorToConvert);

	UFUNCTION(Category = "InputVectorUtils")
		FVector TOWInputVectorToVector(FVector InputVectorToConvert);
};
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "WispsAIController.generated.h"

/**
 * 
 */
UCLASS()
class CHROMA_API AWispsAIController : public AAIController
{
	GENERATED_BODY()

public: 
	AWispsAIController(FObjectInitializer const& object_initializer = FObjectInitializer::Get());
	void BeginPlay() override;
	void OnPossess(APawn* const pawn) override;
	class UBlackboardComponent* get_blackboard() const;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Blackboard")
		class UBehaviorTree* btree;
	UPROPERTY(EditDefaultsOnly, Category = "Blackboard")
		class UBlackboardData* BlackboardToUse;

	class UBlackboardComponent* blackboard;

	class UAISenseConfig_Sight* sight_config;

	UFUNCTION()
		void on_updated(TArray<AActor*> const& updated_actors);

	void setup_perception_system();

	UFUNCTION()
		void StartTOW();

	UFUNCTION()
		void EndTOW();
};

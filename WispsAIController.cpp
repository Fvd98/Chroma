// Fill out your copyright notice in the Description page of Project Settings.


#include "Ennemies/WispsAIController.h"
#include "perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionComponent.h"
#include "perception/AIPerceptionStimuliSourceComponent.h"
#include "Ennemies/Wisp.h"
#include "Components/MagicRopeMediator.h"
#include "blackboard_keys.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Chroma/Public/ChromaPlayerController.h"
#include "Chroma/Public/Actors/ChromaCharacter.h"
AWispsAIController::AWispsAIController(FObjectInitializer const& object_initializer)
{
	setup_perception_system();
}

void AWispsAIController::BeginPlay()
{
	Super::BeginPlay();

	if (!ensure(BlackboardToUse)) { return; }
		UseBlackboard(BlackboardToUse, blackboard);

	if (!ensure(btree)) { return; }
		RunBehaviorTree(btree);		
}

void AWispsAIController::OnPossess(APawn* const pawn)
{
	Super::OnPossess(pawn);
	AWisp* const npc = Cast<AWisp>(pawn);

	sight_config->SightRadius = npc->SightRadius;
	sight_config->PeripheralVisionAngleDegrees = npc->PeripheralVisionAngleDegrees;
	GetPerceptionComponent()->ConfigureSense(*sight_config);
	if (AWisp* wisp = Cast<AWisp>(GetPawn())) 
	{
		if (UMagicRopeMediator* mediator = Cast<UMagicRopeMediator>(wisp->GetComponentByClass(UMagicRopeMediator::StaticClass()))) 
		{		
			mediator->OnTOWStart.AddDynamic(this, &AWispsAIController::StartTOW);
			mediator->OnTOWEnd.AddDynamic(this, &AWispsAIController::EndTOW);
		}
	}
	
}

UBlackboardComponent* AWispsAIController::get_blackboard() const
{
	return blackboard;
}

void AWispsAIController::on_updated(TArray<AActor*> const& updated_actors)
{
}

void AWispsAIController::setup_perception_system()
{
	sight_config = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
	if (sight_config)
	{
		SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception Component")));
		sight_config->SightRadius = 1000.0f;
		sight_config->LoseSightRadius = sight_config->SightRadius + 100.0f;
		sight_config->PeripheralVisionAngleDegrees = 120.0f;
		sight_config->SetMaxAge(5.0f);
		sight_config->AutoSuccessRangeFromLastSeenLocation = 900.0f;
		sight_config->DetectionByAffiliation.bDetectEnemies =
			sight_config->DetectionByAffiliation.bDetectFriendlies =
			sight_config->DetectionByAffiliation.bDetectNeutrals = true;

		// add sight configuration component to perception component
		GetPerceptionComponent()->SetDominantSense(*sight_config->GetSenseImplementation());
		GetPerceptionComponent()->OnPerceptionUpdated.AddDynamic(this, &AWispsAIController::on_updated);
		GetPerceptionComponent()->ConfigureSense(*sight_config);
	}
}

void AWispsAIController::StartTOW()
{
	get_blackboard()->SetValueAsBool(bb_keys::is_in_tow, true);
}

void AWispsAIController::EndTOW()
{
	get_blackboard()->SetValueAsBool(bb_keys::is_in_tow, false);
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/MagicRope.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Chroma/Public/Components/MagicRopeMediator.h"
#include "Kismet/GameplayStatics.h"
#include "Chroma/Public/Actors/ChromaCharacter.h"

// Sets default values
AMagicRope::AMagicRope()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// Physical rope properties
	LastCurrentTensionRatio = 0;
	CurrentTensionRatio = 0;
	TensionlessLenght = 150;
	MaxLenghtRatio = 2.5f;
	TOWMaxWalkSpeed = 5000;

	// Visual rope properties
	RopeCable1 = CreateDefaultSubobject<UCableComponent>(TEXT("Rootcomponent"));
	RootComponent = RopeCable1;
	RopeCable1->CableLength = 250.0f;
	RopeCable1->NumSegments = 20;
	RopeCable1->SolverIterations = 16;
	RopeCable1->CableGravityScale = -10;
	RopeCable1->EndLocation = FVector::ZeroVector;
}

// Called when the game starts or when spawned
void AMagicRope::BeginPlay()
{
	Super::BeginPlay();

	// Confirms that the rope has two connection points
	if (Mediator1 && Mediator2) 
	{
		// Attaches the first end of the rope to the first mediator
		if (GetWorld()->IsServer())
		{
			//Setup Visual rope attachments
			AttachToComponent(Mediator1->GetOwner<ACharacter>()->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "RopeSocket");
		}

		// Subscribes to a vibration delegate assuming the mediator 1 is a player character
		if (AChromaCharacter* character = Mediator1->GetOwner<AChromaCharacter>())
		{
			if (character->bIsLocalPlayerCharacter)
			{
				this->OnVibrationChange.AddDynamic(character, &AChromaCharacter::ControllerVibrationHandler);
			}
		}

		// Attaches the second end of the rope to the second mediator
		RopeCable1->SetAttachEndTo(Mediator2->GetOwner(), NAME_None);

		// Subscribes the mediators to this rope's breaking delegate
		OnRopeBreak.AddDynamic(Mediator1, &UMagicRopeMediator::RopeBrokenHandle);
		OnRopeBreak.AddDynamic(Mediator2, &UMagicRopeMediator::RopeBrokenHandle);

		// Keep a reference of this rope in both mediators
		Mediator1->Ropes.Add(Mediator2, this);
		Mediator2->Ropes.Add(Mediator1, this);

		// Triggers a TOW start if it's a mediator's first rope attached
		if (Mediator1->Ropes.Num() == 1)
			Mediator1->TOWStart();
		if (Mediator2->Ropes.Num() == 1)
			Mediator2->TOWStart();

		// Assigns color to the cable component
		UMaterialInstanceDynamic* BaseMaterial = (UMaterialInstanceDynamic*)RopeCable1->GetMaterial(0);
		RopeDynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		RopeCable1->SetMaterial(0, RopeDynamicMaterial);
		int HUE = Mediator1->RopeOriginColor.LinearRGBToHSV().R;
		HUE = (HUE - 60 + 360) % 360;
		EdgeColor = FLinearColor(HUE, 1, 1).HSVToLinearRGB();
		CenterColor = Mediator1->RopeOriginColor;
		RopeDynamicMaterial->SetVectorParameterValue(FName("CenterColor"), CenterColor);
		RopeDynamicMaterial->SetVectorParameterValue(FName("EdgeColor"), EdgeColor);
	}
	else 
	{
		if (GetWorld()->IsServer()) 
		{
			UE_LOG(LogTemp, Warning, TEXT("A mediator was invalidated before the rope could initiate, terminating the rope."));
			Destroy();
		}
	}
}

// Called every frame
void AMagicRope::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Spawns particles around the "Cable"'s particle locations
	if (RopeParticleSystem)
	{
		TArray<FVector> ParticlePositions;
		RopeCable1->GetCableParticleLocations(ParticlePositions);
		UWorld* World = GetWorld();
		for (FVector ParticlePosition : ParticlePositions) 
		{
			if((FMath::RoundToInt(FMath::FRand() * 100)) > 93)
				UGameplayStatics::SpawnEmitterAtLocation(World, RopeParticleSystem, FTransform(ParticlePosition), true, EPSCPoolMethod::None, true);
		}
	}

	CalculateTOWUpdate();
}

// Called every frame, Once the 2 mediator request a TOW update. Calculates the forces applied to the mediators' owner. Breaks the rope if tension is too high.
void AMagicRope::CalculateTOWUpdate()
{
	if (!IsActorBeingDestroyed())
	{
		if (Mediator1 && Mediator2) 
		{
			// Distance from Mediator 1 to Mediator 2 vector
			FVector Med1ToMed2Direction = (Mediator2->GetOwner()->GetActorLocation() + Mediator2->TOWInputVectorToVector(Mediator2->LastInputVector) * GetWorld()->GetDeltaSeconds()) - (Mediator1->GetOwner()->GetActorLocation() + Mediator1->TOWInputVectorToVector(Mediator1->LastInputVector) * GetWorld()->GetDeltaSeconds());
			float RopeLenght = Med1ToMed2Direction.Size();
			// The extra length of the rope due to elasticity
			float RopeTensionLenght = FMath::Max(RopeLenght - TensionlessLenght, 0.0f);
			float RopeTensionLenghtPastMaximum = FMath::Max(RopeLenght - (TensionlessLenght * MaxLenghtRatio), 0.0f);

			// Calculates the forces applied from mediator 1 to 2 and from 2 to 1  : Mediator1 >->->--<-<-< Mediator2
			FVector ElasticityCancellationMed1 = FVector::ZeroVector;
			FVector ElasticityCancellationMed2 = FVector::ZeroVector;
			if (RopeTensionLenght != 0)
			{
				// Converts length vector to director vector
				Med1ToMed2Direction.Z = 0;
				Med1ToMed2Direction.Normalize();

				// Calculates each mediator strength ratio (Who's pulling stronger)
				float Mediator1Strength = (float)Mediator1->CharacterStrength * Mediator1->CharacterWeightMultiplier;
				float Mediator2Strength = (float)Mediator2->CharacterStrength * Mediator2->CharacterWeightMultiplier;
				float TotalStrength = Mediator1Strength + Mediator2Strength;
				float StrengthRatioMed1 = TotalStrength ? (float)(Mediator2Strength) / TotalStrength : 1.f;
				float StrengthRatioMed2 = 1.f - StrengthRatioMed1;

				// Cancels some force to create an elasticity effect 
				ElasticityCancellationMed1 = Mediator1->VectorToTOWInputVector((Med1ToMed2Direction * RopeTensionLenght * 4.1f * StrengthRatioMed1) + (Med1ToMed2Direction * RopeTensionLenghtPastMaximum * 4.1f));
				ElasticityCancellationMed2 = Mediator1->VectorToTOWInputVector((Med1ToMed2Direction * -1.f * RopeTensionLenght * 4.1f * StrengthRatioMed2) + (Med1ToMed2Direction * -1.f * RopeTensionLenghtPastMaximum * 4.1f));
			}

			// Sends the response to the Mediators
			Mediator1->Multi_AddInputVectorRPC_Implementation(ElasticityCancellationMed1, this);
			Mediator2->Multi_AddInputVectorRPC_Implementation(ElasticityCancellationMed2, this);

			if (GetWorld()->IsServer())
			{
				// Keeps track of the rope's tension
				float newCurrentTensionRatio = FMath::Clamp(RopeLenght / (MaxLenghtRatio * TensionlessLenght), 0.f, 0.99f);
				CurrentTensionRatio = newCurrentTensionRatio == CurrentTensionRatio && newCurrentTensionRatio > 0.01f ? newCurrentTensionRatio - 0.01f : newCurrentTensionRatio;
				OnRep_CurrentTensionRatio();
				LastCurrentTensionRatio = CurrentTensionRatio;
			}
		}
		else 
		{
			UE_LOG(LogTemp, Warning, TEXT("A mediator was invalidated before the rope could TOWUpdate, terminating the rope."));
			Destroy();
		}
	}
}

void AMagicRope::OnRep_CurrentTensionRatio()
{
	if (!IsActorBeingDestroyed())
	{
		if (APlayerController* PlayerController = Mediator1->GetOwner<ACharacter>()->GetController<APlayerController>())
		{
			OnVibrationChange.Broadcast(EDynamicForceFeedbackAction::Start, CurrentTensionRatio/2);

			// Tensions has started increasing
			if (!bIsTensionShaking && CurrentTensionRatio > 0.50f)
			{
				PlayerController->ClientStartCameraShake(TensionMatineeCameraShake, 1, ECameraShakePlaySpace::CameraLocal);
				bIsTensionShaking = true;
			}
			// Tensions has started decreasing
			else if (bIsTensionShaking && CurrentTensionRatio <= 0.50f)
			{
				PlayerController->ClientStopCameraShake(TensionMatineeCameraShake, true);
				bIsTensionShaking = false;
			}
		}
		if (RopeDynamicMaterial)
			RopeDynamicMaterial->SetScalarParameterValue(FName("Tension"), CurrentTensionRatio);
	}
}

// Notifies the mediator that the rope is broken
void AMagicRope::BreakRope()
{
	if (GetWorld()->IsServer())
		Multi_BreakRopeRPC();
	else
		Server_BreakRopeRPC();
}

void AMagicRope::Server_BreakRopeRPC_Implementation()
{
	Multi_BreakRopeRPC();
}

void AMagicRope::Multi_BreakRopeRPC_Implementation()
{
	OnRopeBreak.Broadcast(this);
	if (bIsTensionShaking && Mediator1)
	{
		if (APlayerController* PlayerController = Mediator1->GetOwner<ACharacter>()->GetController<APlayerController>())
		{
			PlayerController->ClientStopCameraShake(TensionMatineeCameraShake, true);
			bIsTensionShaking = false;
		}
	}
	if (GetWorld()->IsServer())
	{
		Destroy();
	}
}

// Variables replication
void AMagicRope::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AMagicRope, Mediator1, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(AMagicRope, Mediator2, COND_InitialOnly);
	DOREPLIFETIME(AMagicRope, CurrentTensionRatio);
	DOREPLIFETIME(AMagicRope, LastCurrentTensionRatio);
}


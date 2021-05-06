// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/MagicRopeMediator.h"
#include "Chroma/Public/Actors/ChromaCharacter.h"
#include "Navigation/PathFollowingComponent.h"
#include "Actors/MagicRope.h"
#include "Net/UnrealNetwork.h"
#include "Ennemies/WispsAIController.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UMagicRopeMediator::UMagicRopeMediator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// Default values
	bTOW = false;
	InputVector = FVector::ZeroVector;
	LastInputVector = FVector::ZeroVector;

	// TOW max movement speed (Maximum speed reached after both the player input and the ropes forces are applied)
	TOWMaxWalkSpeed = 5000;
	CharacterStrength = 12;
	CharacterWeightMultiplier = 1.f;

	// Socket to search fore to attach the cable component of the AMagicRope
	SocketName = "RopeSocket";

	// Pressure configurations
	CurrentPressure = 0.f;
	PressureTreshold = 700.f;
	PressureTresholdDifficulty = 1.f;

	PressureCounter = 0.f;
	PressureCounterIncrement = 2.f;
	PressureCounterDecrement = 1.f;
	PressureCounterTrigger = 3.0f;
	PressureMinimumConnectorPointCount = 1;
}

// Called when the game starts
void UMagicRopeMediator::BeginPlay()
{
	Super::BeginPlay();
	if(GetWorld()->IsServer())
		GetOwner()->OnDestroyed.AddDynamic(this, &UMagicRopeMediator::OwnerDestroyedHandler);

	// Configures itself based on the type of controller the attached actor has
	AController* Controller = GetOwner<ACharacter>()->GetController();
	if (Controller) 
	{
		if (AChromaPlayerController* ChromaPlayerController = Cast<AChromaPlayerController>(Controller))
		{
			// Has a ChromaPlayerController
			PlayerControllerChangeHandler(ChromaPlayerController);
		}
		else if (AWispsAIController* WispsAIController = Cast<AWispsAIController>(Controller))
		{
			// Has an WispsAIController
			WispsAIController->GetPathFollowingComponent()->PostProcessMove.BindUObject(this, &UMagicRopeMediator::AIMovementHandle);
			bAlwaysFaceOpponent = false;
		}
	}
	else 
	{
		// The character doesn't have a controller yet, disables this until it does
		if (AChromaCharacter* ChromaCharacter = GetOwner<AChromaCharacter>())
		{
			ChromaCharacter->OnPlayerControllerAssigned.AddDynamic(this, &UMagicRopeMediator::PlayerControllerChangeHandler);
		}
	}

	// Sets up the movement related variables
	OwnerCharacterMovementComponent = Cast<UCharacterMovementComponent>(GetOwner()->GetComponentByClass(UCharacterMovementComponent::StaticClass()));
	if (OwnerCharacterMovementComponent)
	{
		InitialMaxWalkSpeed = OwnerCharacterMovementComponent->MaxWalkSpeed;
		InitialMaxAcceleration = OwnerCharacterMovementComponent->MaxAcceleration;
		InitialBrakingDecelerationWalking = OwnerCharacterMovementComponent->BrakingDecelerationWalking;
		TOWMaxMSRatio = (float)InitialMaxWalkSpeed / TOWMaxWalkSpeed;
	}
}

void UMagicRopeMediator::OwnerDestroyedHandler(AActor* DestroyedActor) 
{
	DetachAllRope();
}

void UMagicRopeMediator::PlayerControllerChangeHandler(AChromaPlayerController* ChromaPlayerController)
{
	ChromaPlayerController->OnPostProcessMoveInput.AddDynamic(this, &UMagicRopeMediator::PlayerMovementHandle);
	ChromaPlayerController->OnPostProcessMoveInput.AddDynamic(this, &UMagicRopeMediator::CalculateTOWUpdate);
	bAlwaysFaceOpponent = true;
}

// Called every frame
void UMagicRopeMediator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Manages the rotations during TOW
	if (bTOW && bAlwaysFaceOpponent) 
	{
		TArray<UMagicRopeMediator*> OpponentMediators;
		Ropes.GenerateKeyArray(OpponentMediators);
		UMagicRopeMediator* OpponentMediator = OpponentMediators[0];
		if(OpponentMediator)
			GetOwner()->GetRootComponent()->SetRelativeRotation(UKismetMathLibrary::FindLookAtRotation(GetOwner()->GetActorLocation(), OpponentMediator->GetOwner()->GetActorLocation()));
	}
}

// Called when a rope notifies it's break event
void UMagicRopeMediator::RopeBrokenHandle(AMagicRope* Rope)
{
	if(!GetOwner()->IsActorBeingDestroyed())
		Multi_RopeBrokenHandleRPC(Rope);
}

void UMagicRopeMediator::Multi_RopeBrokenHandleRPC_Implementation(AMagicRope* Rope)
{
	if (UMagicRopeMediator* OtherMediator = Rope ? Rope->Mediator1 == this ? Rope->Mediator2 : Rope->Mediator1 : nullptr) 
	{
		if (OtherMediator)
			Ropes.Remove(OtherMediator);
		else
			UE_LOG(LogTemp, Warning, TEXT("A rope got destroyed before the rope broken handle could remove it from the list, a total of %s ropes are still registered."), *FString::FromInt(Ropes.Num()));
	}
	OnRopeDetach.Broadcast();
	if (!Ropes.Num())
		TOWEnd();
}

// CORE OF THE CLASS
// Called by controllers every frame to update the TOW, notifies the ropes expecting a response next tick, applies last tick response to InputVector
void UMagicRopeMediator::CalculateTOWUpdate()
{
	if (bTOW && OwnerCharacterMovementComponent)
	{
		// Adds in the last tick's TOWUpdate vector (averaged with the previous one)
		if (InputVector != FVector::ZeroVector || LastInputVector != FVector::ZeroVector) 
		{
			// Re-applies the edited vector
			OwnerCharacterMovementComponent->AddInputVector(InputVector);

			// Keep track of the last input vector for state change comparison
			LastInputVector = InputVector;

			// Reset the input vector
			InputVector = FVector::ZeroVector;
			
			// Calculates the total pressure on the owner actor
			FVector TotalPressureVectors = FVector::ZeroVector;
			float TotalPressureVectorsMagnithudes = 0.f;
			TArray<FVector> PressureVectorArray;
			PressureVectors.GenerateValueArray(PressureVectorArray);
			for (FVector PressureVector : PressureVectorArray)
			{
				TotalPressureVectorsMagnithudes += PressureVector.Size();
				TotalPressureVectors += PressureVector;
			}

			// Calculates actor TOW priority (Who's dragging who)

			// Forces applied by external actors (e.g. Ropes)
			FVector OutsideForces = TOWInputVectorToVector(LastInputVector);
			OutsideForces.Normalize();
			// Forces the owner actor applies to itself 
			FVector InnerForces = TOWInputVectorToVector(VolontaryMovementVector);
			InnerForces.Normalize();

			// Gets the angle between both inner and outside normalized forces
			float TOWPriorityBase = FMath::RadiansToDegrees(acosf(FVector::DotProduct(OutsideForces, InnerForces))) /180.f;
			// 1 means the owner have priority, 0 means the external forces have priority and 0.5 means either they cancel out or they have no magnitude
			TOWPriority = !OutsideForces.Size() && !InnerForces.Size() ? 0.5 : !OutsideForces.Size() ? 1 : !InnerForces.Size() ? 0 : TOWPriorityBase < 0.5 ? 0 : 1;

			// Updates the pressure management and triggers if the configured threshold is reached
			CurrentPressure = FMath::Clamp(((TotalPressureVectorsMagnithudes - TotalPressureVectors.Size()) / PressureTresholdDifficulty) / PressureTreshold, 0.f, 1.f);
			if (CurrentPressure == 1.0f)
			{
				PressureCounter += (PressureCounterIncrement * GetWorld()->GetDeltaSeconds());
				if (PressureCounter >= PressureCounterTrigger && PressureMinimumConnectorPointCount <= Ropes.Num())
					OnPressureTrigger.Broadcast();
			}
			else if (!PressureCounter)
			{
				PressureCounter = FMath::Max(PressureCounter - (PressureCounterDecrement* GetWorld()->GetDeltaSeconds()), 0.f);
			}
			PressureVectors.Empty();
		}
	}
}

// Converts the already present InputVector to the TOW ratio-ed one
void UMagicRopeMediator::PlayerMovementHandle()
{
	if (bTOW && OwnerCharacterMovementComponent)
	{
		// Gets and consumes the actor's InputVector
		FVector ActorInputVector = OwnerCharacterMovementComponent->ConsumeInputVector();
		if (ActorInputVector != FVector::ZeroVector)
		{
			// Converts it to the ratio-ed equivalent (converts Base-MS to TOW-MS)  
			ActorInputVector.Normalize();
			ActorInputVector *= TOWMaxMSRatio;
			VolontaryMovementVector = ActorInputVector;
			Multi_AddInputVectorRPC(ActorInputVector);
		}
	}
}

// Converts the AI's movement vector to the TOW ratio-ed one
void UMagicRopeMediator::AIMovementHandle(UPathFollowingComponent* PathFollowingComponent, FVector& AIMovementVector)
{
	if (bTOW) 
	{
		FVector OriginalAIDirectionnalVector = AIMovementVector;
		OriginalAIDirectionnalVector.Normalize();
		OriginalAIDirectionnalVector *= TOWMaxMSRatio * TOWMaxWalkSpeed;
		AIMovementVector = FVector::ZeroVector;
		FVector AIInputVector = VectorToTOWInputVector(OriginalAIDirectionnalVector);
		VolontaryMovementVector = AIInputVector;
		Multi_AddInputVectorRPC(AIInputVector);
	}
}

// Gives an InputVector to the local controllers
void UMagicRopeMediator::Multi_AddInputVectorRPC_Implementation(FVector AddedInputVector, AMagicRope* NullablePressureRope)
{
	if (ACharacter* OwnerCharacter = GetOwner<ACharacter>())
	{
		if (AController* OwnerController = OwnerCharacter->GetController())
		{
			FVector MovementAddedVector = TOWInputVectorToVector(AddedInputVector);
			if (NullablePressureRope)
				PressureVectors.Add(NullablePressureRope, MovementAddedVector);
			InputVector = VectorToTOWInputVector(TOWInputVectorToVector(InputVector) + MovementAddedVector);
		}
	}
}

// Attach rope handler Client->Server->Multicast || Server->Multicast
void UMagicRopeMediator::AttachRope(UMagicRopeMediator* OtherMediator)
{
	if (OtherMediator) 
	{
		Ropes.Add(OtherMediator, nullptr);
		if (GetWorld()->IsServer())
			SpawnRopeHandle(OtherMediator);
		else
			Server_AttachRopeRPC(OtherMediator);
	}
}
// Attach Rope Server->Mutlicast
void UMagicRopeMediator::Server_AttachRopeRPC_Implementation(UMagicRopeMediator* OtherMediator)
{
	SpawnRopeHandle(OtherMediator);
}
// Attaches a rope (Spawns it) from owner to given mediator's owner
void UMagicRopeMediator::SpawnRopeHandle(UMagicRopeMediator* OtherMediator)
{
	// Spawns deferred the rope to attach to both our mediators
	FTransform RopeTransform = FTransform(GetOwner()->GetActorLocation());
	AMagicRope* NewRope = GetWorld()->SpawnActorDeferred<AMagicRope>(RopeClass, RopeTransform, (AActor*)nullptr, (APawn*)nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	// References setup
	NewRope->Mediator1 = this;
	NewRope->Mediator2 = OtherMediator;
	// Set owner so that network functions work
	NewRope->SetOwner(GetOwner());
	// Spawns the rope for real
	NewRope->FinishSpawning(RopeTransform);
}

// Tells a rope it has to break
void UMagicRopeMediator::DetachRope(UMagicRopeMediator* OtherMediator)
{
	if(Ropes[OtherMediator])
		Ropes[OtherMediator]->BreakRope();
}

// Tells all ropes they have to break
void UMagicRopeMediator::DetachAllRope()
{
	TArray<AMagicRope*> ARopes;
	Ropes.GenerateValueArray(ARopes);
	for(AMagicRope* Rope : ARopes)
	{
		if(Rope)
			Rope->BreakRope();
	}
}

// Starts the TOW state
void UMagicRopeMediator::TOWStart()
{
	bTOW = true;
	if (OwnerCharacterMovementComponent) 
	{
		OwnerCharacterMovementComponent->MaxWalkSpeed = TOWMaxWalkSpeed;
		OwnerCharacterMovementComponent->MaxAcceleration = 100000;
		OwnerCharacterMovementComponent->BrakingDecelerationWalking = 100000;
	}
	OnTOWStart.Broadcast();
}

// Ends the TOW state
void UMagicRopeMediator::TOWEnd()
{
	bTOW = false;
	if (OwnerCharacterMovementComponent)
	{
		OwnerCharacterMovementComponent->MaxWalkSpeed = InitialMaxWalkSpeed;
		OwnerCharacterMovementComponent->MaxAcceleration = InitialMaxAcceleration;
		OwnerCharacterMovementComponent->BrakingDecelerationWalking = InitialBrakingDecelerationWalking;
	}
	OnTOWEnd.Broadcast();
}

// Converts a movement vector (circle) to an input vector (square)
FVector UMagicRopeMediator::VectorToTOWInputVector(FVector VectorToConvert)
{
	if (VectorToConvert.IsZero())
		return VectorToConvert;
	
	FVector MagnithudeVector = VectorToConvert;
	float XMagnithude = FMath::Abs(MagnithudeVector.X / TOWMaxWalkSpeed);
	float YMagnithude = FMath::Abs(MagnithudeVector.Y / TOWMaxWalkSpeed);
	FVector NormalizedVectorToConvert = VectorToConvert;

	// Converts length vector to director vector
	NormalizedVectorToConvert.Z = 0;
	NormalizedVectorToConvert.Normalize();

	// Converts the director vector to the input vector format and re-applies length
	float InputVectorConversionDivider = FMath::Max(FMath::Abs(NormalizedVectorToConvert.X), FMath::Abs(NormalizedVectorToConvert.Y));
	//Normalized input vector
	FVector OutputTOWInputVector = !NormalizedVectorToConvert.Size() ? FVector::ZeroVector : FVector(NormalizedVectorToConvert.X / InputVectorConversionDivider, NormalizedVectorToConvert.Y / InputVectorConversionDivider, 0);
	OutputTOWInputVector.X *= XMagnithude;
	OutputTOWInputVector.Y *= YMagnithude;
	return OutputTOWInputVector;
}

// Converts an input vector (square) to a movement vector (circle) 
FVector UMagicRopeMediator::TOWInputVectorToVector(FVector InputVectorToConvert)
{
	if (InputVectorToConvert.IsZero())
		return InputVectorToConvert;

	// keeps the length of the given input vector
	float InputVectorLenghtX = InputVectorToConvert.X * TOWMaxWalkSpeed;
	float InputVectorLenghtY = InputVectorToConvert.Y * TOWMaxWalkSpeed;

	// Re-applies length
	FVector OutputVector = FVector(InputVectorLenghtX, InputVectorLenghtY, 0);
	return OutputVector;
}

// Variables replication
void UMagicRopeMediator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UMagicRopeMediator, bAlwaysFaceOpponent);
}

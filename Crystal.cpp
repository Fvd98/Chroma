// Chroma


#include "Actors/Crystal.h"
#include "Net/UnrealNetwork.h"
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>
#include "Engine/Selection.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

// Properties' replication
void ACrystal::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicates playerColor.
	DOREPLIFETIME(ACrystal, CurrentRequiredWispColor);
	DOREPLIFETIME(ACrystal, CurrentRadiusPercent);
	DOREPLIFETIME(ACrystal, TargetRadiusPercent);
	DOREPLIFETIME(ACrystal, CompletedPatternsCount);
	DOREPLIFETIME(ACrystal, bCompleted);
}

// Sets default values
ACrystal::ACrystal()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Crystal static mesh
	CrystalStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrystalStaticMesh"));
	CrystalStaticMesh->SetCustomDepthStencilValue(255);
	RootComponent = CrystalStaticMesh;

	// Wisp absorption hit sphere
	WispAbsorbtionHitSphere = CreateDefaultSubobject<USphereComponent>(TEXT("WispAbsorbtionHitSphere"));
	WispAbsorbtionHitSphere->SetupAttachment(RootComponent);
	WispAbsorbtionHitSphere->SetSphereRadius(250.0f);
	WispAbsorbtionHitSphere->SetRelativeLocation(FVector(0, 0, 0));

	// Crystal's idle particles
	CrystalParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("CrytalParticleSystemComponent"));
	CrystalParticleSystem->SetupAttachment(RootComponent);
	CrystalParticleSystem->SetCustomDepthStencilValue(255);

	// Completion configurations
	RequiredPatternComplete = 3;

	// Explosion configurations
	ExplosionDelay = 3.f;
	ExplosionRelevancyRange = 10000.f;
	ExplosionColorRelevancy = { FLinearColor::Red, FLinearColor::Green, FLinearColor::Blue };
	bExplode = false;

	// Completion radius configurations
	BaseRadius = 200.0f;
	MaxRadius = 2800.0f;
	CurrentRadiusPercent = BaseRadius / MaxRadius;
	CurrentRadiusPercentAlpha = 0.f;
	TargetRadiusPercent = 0.f;
	RadiusUnitLERPSpeed = 100.0f;
	CompletionRadiusLERPCurvePower = 4.0f;
	CompletionRadiusUnitLERPSpeed = 500.0f;

	// Round robin configurations
	CurrentSpawnerRoundRobinIndex = 0;
	bSpawnerRoundRobin = false;

	// Pattern configurations
	CrystalSpawnPatternBank = FCrystalSpawnPatternSequenceBank();
	bFollowPatternBankOrder = true;
	FollowPatternBankOrderIndex = -1;
	bReplicates = true;

#if WITH_EDITOR
	this->bRunConstructionScriptOnDrag = true;
#endif
#if WITH_EDITORONLY_DATA
	USelection::SelectObjectEvent.AddUObject(this, &ACrystal::OnObjectSelected);
#endif
}

// TODO : Would make selected patterns randomised 
void ACrystal::InitCrystalSpawnPatternBank()
{
	/*for (FCrystalSpawnPattern CrystalSpawnPattern : CrystalSpawnPatternBank) 
	{
		for (FLinearColor LinearColor : CrystalSpawnPattern.StartingColor) 
		{
			if (!CrystalSpawnPatternBankStartingRelationship.Contains(LinearColor))
				CrystalSpawnPatternBankStartingRelationship.Add(LinearColor, {});

			CrystalSpawnPatternBankStartingRelationship[LinearColor].Add(CrystalSpawnPattern);
		}
	}*/
}

// Fetches and setups the next pattern of the current sequence
void ACrystal::SetupNextPattern()
{
	if (Spawners.Num())
	{
		if (WispSpawnClasses.Num()) 
		{
			FCrystalSpawnPattern LastSelectedCrystalSpawnPattern = CurrentlySelectedCrystalSpawnPattern;
			CurrentlySelectedCrystalSpawnPattern = FindNextPattern();
			Set_CurrentRequiredWispColor(CurrentlySelectedCrystalSpawnPattern.RequiredColor);
			for (FLinearColor LinearColor : CurrentlySelectedCrystalSpawnPattern.StartingColor)
			{
				if (!LastSelectedCrystalSpawnPattern.EndingColor.Contains(LinearColor)) 
				{
					ACrystalSpawner* UsedSpawner = Spawners[bSpawnerRoundRobin ? CurrentSpawnerRoundRobinIndex : FMath::RandRange(0, Spawners.Num() - 1)];
					if(bSpawnerRoundRobin)
						CurrentSpawnerRoundRobinIndex = CurrentSpawnerRoundRobinIndex + 1 % Spawners.Num();
					RegisterSpawnedWisp(UsedSpawner->SpawnWispDynamic(LinearColor, WispSpawnClasses[FMath::RandRange(0, WispSpawnClasses.Num() - 1)]));
				}
			}
		}
		else
			UE_LOG(LogTemp, Error, TEXT("Crystal %s : No wisp class found to spawn wisps from."), *GetFName().ToString());
	}
	else
		UE_LOG(LogTemp, Error, TEXT("Crystal %s : No spawner found to release pattern."), *GetFName().ToString());
}

// Keep track of connected wisps
void ACrystal::RegisterSpawnedWisp(AWisp* newWisp)
{
	RemainingWisps.FindOrAdd(newWisp->WispColor, {});
	RemainingWisps[newWisp->WispColor].Add(newWisp);
	newWisp->OnWispSplitSpawn.AddDynamic(this, &ACrystal::RegisterSpawnedWisp);
}

// Gets the next pattern in bank
FCrystalSpawnPattern ACrystal::FindNextPattern()
{
	// First pattern
	if (CrystalSelectedSequence.PatternArray.Num())
	{
		bool bFirstPattern = CurrentlySelectedCrystalSpawnPattern.RequiredColor == FLinearColor::Black;
		if (bFollowPatternBankOrder)
		{
			if (!CrystalSelectedSequence.PatternArray.IsValidIndex(++FollowPatternBankOrderIndex) || !bFirstPattern && !PatternsCompatible(CurrentlySelectedCrystalSpawnPattern, CrystalSelectedSequence.PatternArray[FollowPatternBankOrderIndex]))
			{
				UE_LOG(LogTemp, Error, TEXT("Crystal %s : Chain of pattern broken by a pattern that didn't follow the previous one."), *GetFName().ToString());
				return FCrystalSpawnPattern();
			}

			return CrystalSelectedSequence.PatternArray[FollowPatternBankOrderIndex];
		}
		else
		{
			//TODO procedurally random pattern searching
			UE_LOG(LogTemp, Error, TEXT("Crystal %s : Sorry dynamic pattern randomizing isn't implemented yet. (turn bFollowPatternBankOrder, on to fix)"), *GetFName().ToString());
			return FCrystalSpawnPattern();
		}
	}
	else
		UE_LOG(LogTemp, Error, TEXT("Crystal %s : Pattern bank empty."), *GetFName().ToString());
	

	/*TEMP*/
	return FCrystalSpawnPattern();
}
// Verifies that the given pattern is compatible with the last one
bool ACrystal::PatternsCompatible(FCrystalSpawnPattern LastPattern, FCrystalSpawnPattern NextPattern) 
{
	bool bIsValid = true;
	for (FLinearColor LinearColor : LastPattern.EndingColor) 
		if (!NextPattern.StartingColor.Contains(LinearColor)) 
			bIsValid = false;
	return bIsValid;
}

// Sets the crystal's color
void ACrystal::Set_CurrentRequiredWispColor(FLinearColor newCurrentRequiredWispColor)
{
	CurrentRequiredWispColor = newCurrentRequiredWispColor;
	OnRep_CurrentRequiredWispColor();

	if (bCompleted) 
	{
		Mutli_UpdateMaterialCrystalCompletionRPC();
	}
}
// Updates the crystal's material
void ACrystal::Mutli_UpdateMaterialCrystalCompletionRPC_Implementation()
{
	if (!CrystalColorDynamicMaterial)
	{
		CrystalColorDynamicMaterial = CrystalStaticMesh->CreateAndSetMaterialInstanceDynamic(2);
		CrystalColorDynamicMaterial->SetScalarParameterValue(FName("emissive_power"), 10);
	}
	if(!CrystalPedestalDynamicMaterial)
		CrystalPedestalDynamicMaterial = CrystalStaticMesh->CreateAndSetMaterialInstanceDynamic(4);

	CrystalPedestalDynamicMaterial->SetScalarParameterValue(FName("emissive_power"), 4.0f);
	CrystalColorDynamicMaterial->SetScalarParameterValue(FName("emissive_rainbow_power"), 1.0f);
}
// Updates variable affected by the crystal's color
void ACrystal::OnRep_CurrentRequiredWispColor()
{
	if (!CrystalColorDynamicMaterial)
	{
		CrystalColorDynamicMaterial = CrystalStaticMesh->CreateAndSetMaterialInstanceDynamic(2);
		CrystalColorDynamicMaterial->SetScalarParameterValue(FName("emissive_power"), 10);
	}
		
	CrystalColorDynamicMaterial->SetVectorParameterValue(FName("emissive_color"), CurrentRequiredWispColor);
}

// A wisp was absorbed, prepare the following pattern
void ACrystal::Increase_CompletedPatternsCount()
{
	CompletedPatternsCount++;
	OnRep_CompletedPatternsCount();
}
void ACrystal::OnRep_CompletedPatternsCount()
{
	if (GetWorld()->IsServer()) 
	{
		bool LastPatternCompleted = RequiredPatternComplete == CompletedPatternsCount;
		TargetRadiusPercent = LastPatternCompleted ? 1.f : (BaseRadius / MaxRadius) + ((float)CompletedPatternsCount * 0.01f);
		if (LastPatternCompleted)
		{
			bCompleted = true;
			Set_CurrentRequiredWispColor(CWhite);
			OnCrystalComplete.Broadcast(this);
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ACrystal::Explode, ExplosionDelay, false);
		}
		else
		{
			SetupNextPattern();
		}
	}
}

// Crystal is now completed, satisfying color explosion!
void ACrystal::Explode()
{
	bExplode = true;
	OnCrystalExplosion.Broadcast(this);
}

// Called when the game starts or when spawned
void ACrystal::BeginPlay()
{
	Super::BeginPlay();

	if (CrystalParticleSystem->Template)
		CrystalParticleSystem->Activate();
	
	if (GetWorld()->IsServer()) 
	{
		WispAbsorbtionHitSphere->OnComponentBeginOverlap.AddDynamic(this, &ACrystal::OnWispAbsorbtionHitSphereBeginOverlap);
		for (FCrystalSpawnPatternSequence CrystalSpawnPatternSequence : CrystalSpawnPatternBank.PatternSequenceBank) 
		{
			if (CrystalSpawnPatternSequence.bIsActive)
			{
				CrystalSelectedSequence = CrystalSpawnPatternSequence;
				break;
			}
		}
		OnRep_CompletedPatternsCount();
	}
}

// Called every frame
void ACrystal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Manages the explosion's growth, open connected mist gates if the explosion completes
	if (GetWorld()->IsServer() && CurrentRadiusPercent != TargetRadiusPercent && bExplode)
	{
		CurrentRadiusPercentAlpha = FMath::FInterpConstantTo(CurrentRadiusPercentAlpha, TargetRadiusPercent, DeltaTime, (bCompleted ? CompletionRadiusUnitLERPSpeed : CompletionRadiusUnitLERPSpeed) / MaxRadius);
		CurrentRadiusPercent = FMath::InterpEaseIn(CurrentRadiusPercent, TargetRadiusPercent, CurrentRadiusPercentAlpha, CompletionRadiusLERPCurvePower);
		if (CurrentRadiusPercent == 1)
		{
			for (ACrystalGate* CrystalGate : ConnectedGates)
			{
				CrystalGate->Server_DisableMistRPC();
			}
			OnCrystalExplosionComplete.Broadcast(this);
		}
	}	
}

// Tries to absorb the given wisp
void ACrystal::TryAbsorbWisp(AWisp* ToAbsorbWisp)
{
	if (GetWorld()->IsServer() && ToAbsorbWisp->WispColor == CurrentRequiredWispColor && !bCompleted) 
	{
		Multi_WispAbsorbedRPC(ToAbsorbWisp->WispColor);
		ToAbsorbWisp->Destroy();
		Increase_CompletedPatternsCount();

		// Broadcast events
		OnWispAbsorbed.Broadcast();
	}
}

// A wisp of the given color was absorbed, manages particles, sounds and effects
void ACrystal::Multi_WispAbsorbedRPC_Implementation(FLinearColor AbsorbedWispColor)
{
	if (AbsorbtionNiagaraSystem) 
	{
		UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), AbsorbtionNiagaraSystem, GetActorLocation() + MergeNiagaraSystemOffSet, GetActorRotation(), FVector::OneVector, true, false);
		if (NiagaraComponent)
		{
			NiagaraComponent->SetRenderCustomDepth(true);
			NiagaraComponent->SetCustomDepthStencilValue(255);
			NiagaraComponent->SetColorParameter(FName("Color_01"), AbsorbedWispColor);
			NiagaraComponent->ActivateSystem();
		}
	}
}

//Something collided with the crystal collider
void ACrystal::OnWispAbsorbtionHitSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (GetWorld()->IsServer()) 
	{
		if (AWisp* PossibleWispToAbsorb = Cast<AWisp>(OtherActor))
		{
			if (PossibleWispToAbsorb->GetRootComponent() == OtherComp)
			{
				TryAbsorbWisp(PossibleWispToAbsorb);
			}
		}
	}
}

#if WITH_EDITORONLY_DATA
void ACrystal::OnObjectSelected(UObject* Object)
{
	if (Object == this)
	{
		OnConstruction(GetActorTransform());
	}
}
#endif


void ACrystal::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	CurrentRadiusPercent = BaseRadius / MaxRadius;

#if WITH_EDITOR
	if (GetWorld() && !GetWorld()->GetGameInstance())
	{
		UKismetSystemLibrary::FlushPersistentDebugLines(this);
		UKismetSystemLibrary::DrawDebugCylinder(this, GetActorLocation(), GetActorLocation() + FVector(0, 0, 150), BaseRadius, 50, FLinearColor(50,50,255), 10000, 5);
		UKismetSystemLibrary::DrawDebugCylinder(this, GetActorLocation(), GetActorLocation() + FVector(0, 0, 150), ((BaseRadius / MaxRadius) + ((float)RequiredPatternComplete * 0.01f)) * MaxRadius, 50, FLinearColor(150,150,255), 10000, 5);
		UKismetSystemLibrary::DrawDebugCylinder(this, GetActorLocation(), GetActorLocation() + FVector(0, 0, 150), MaxRadius, 50, FLinearColor(200,200,255), 10000, 5);
		UKismetSystemLibrary::DrawDebugCylinder(this, GetActorLocation(), GetActorLocation() + FVector(0, 0, 150), ExplosionRelevancyRange, 50, FLinearColor(255, 255, 0), 10000, 5);
	}
#endif
}

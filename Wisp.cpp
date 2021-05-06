// Fill out your copyright notice in the Description page of Project Settings.


#include "Ennemies/Wisp.h"
#include "blackboard_keys.h"
#include "Net/UnrealNetwork.h"
#include "Components/WidgetComponent.h"
#include "Components/ProgressBar.h"
#include "Ennemies/WispsAIController.h"
#include "BrainComponent.h"
#include <ctime> 
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"


// Properties' replication
void AWisp::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicates playerColor.
	DOREPLIFETIME(AWisp, WispColor);
	DOREPLIFETIME(AWisp, WispColorCurrent);
	DOREPLIFETIME(AWisp, WispColorTarget);
	DOREPLIFETIME(AWisp, Endurance);
	DOREPLIFETIME(AWisp, bWispColorTargetReachedOnce);
	DOREPLIFETIME(AWisp, bWispColorLocked);
}
// Sets default values
AWisp::AWisp():
	WidgetComponent(CreateDefaultSubobject<UWidgetComponent  >(TEXT("WidgetComponent"))),
	WidgetExclamationComponent(CreateDefaultSubobject<UWidgetComponent  >(TEXT("WidgetInteractionComponent"))),
	InteractableComponent(CreateDefaultSubobject<UInteractableComponent>(TEXT("InteractableComp")))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SplitSpawnClass = StaticClass();

	if (WidgetComponent)
	{
		WidgetComponent->SetupAttachment(GetMesh());
		WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
		WidgetComponent->SetRelativeLocation(FVector(-80.0f, 0.0f, 0.0f));
		WidgetComponent->SetUsingAbsoluteRotation(true);
		static ConstructorHelpers::FClassFinder<UUserWidget> WidgetClass(TEXT("/Game/Visual_Media/UIs/BP_WispWidget"));

		if (WidgetClass.Succeeded())
		{
			WidgetComponent->SetWidgetClass(WidgetClass.Class);
		}
	}

	if (WidgetExclamationComponent)
	{
		WidgetExclamationComponent->SetupAttachment(GetMesh());
		WidgetExclamationComponent->SetWidgetSpace(EWidgetSpace::World);
		WidgetExclamationComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));
		WidgetExclamationComponent->SetUsingAbsoluteRotation(true);
		WidgetExclamationComponent->SetVisibility(false);
		static ConstructorHelpers::FClassFinder<UUserWidget> WidgetInteractionClass(TEXT("/Game/Visual_Media/UIs/BP_WispExclamationWidget"));

		if (WidgetInteractionClass.Succeeded())
		{
			WidgetExclamationComponent->SetWidgetClass(WidgetInteractionClass.Class);
		}
	}

	if (InteractableComponent)
	{
		InteractableComponent->AttachToComponent(RootComponent,FAttachmentTransformRules::KeepRelativeTransform);
	}
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	MagicRopeMediatorComp = CreateDefaultSubobject<UMagicRopeMediator>(TEXT("MagicRopeMediatorComp"));
	GrabbableComp = CreateDefaultSubobject<UGrabbable>(TEXT("GrabbableComp"));
	GetMesh()->SetRenderCustomDepth(true);
	GetMesh()->SetCustomDepthStencilValue(255);
	WispColor = FLinearColor(0, 0, 0);
	WispColorTarget = FLinearColor(0, 0, 0);
	WispColorLERPSpeed = 1.75f;
	WispColorMissingLERPSpeed = 1.0f;

	bReplicates = true;
}

// Called when the game starts or when spawned - [TODO] -> Refactor network
void AWisp::BeginPlay()
{
	Super::BeginPlay();

	if (InitialAttachFrom.Num())
		for(UMagicRopeMediator* AttachedFromMagicRopeMediator : InitialAttachFrom)
			AttachedFromMagicRopeMediator->AttachRope(MagicRopeMediatorComp);

	if (GetWorld()->IsServer())
		MagicRopeMediatorComp->OnPressureTrigger.AddDynamic(this, &AWisp::Split);

	WispColorMaterialAura = GetMesh()->CreateAndSetMaterialInstanceDynamic(1);
	WispColorTarget = FLinearColor::Black;
	WispColorCurrent = FLinearColor::Black;
	OnRep_WispColorCurrent();
	
	WispColorMaterialCore = GetMesh()->CreateAndSetMaterialInstanceDynamic(0);


	HomePoint = GetActorLocation();

}


// Called every frame
void AWisp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if(HasAuthority() && MagicRopeMediatorComp->bTOW)
	{
		MagicRopeMediatorComp->CalculateTOWUpdate();

		if (Endurance < 0)
		{
			InRegen = true;
		}
		else if (Endurance > 100)
		{
			InRegen = false;
		}

		TArray<AMagicRope*> ropes;
		MagicRopeMediatorComp->Ropes.GenerateValueArray(ropes);

		for (AMagicRope* rope : ropes)
		{
			if (InRegen)
			{
				Endurance = Endurance + (RegenEnduranceGain * DeltaTime);
				break;
			}
			if (rope->CurrentTensionRatio >= 0.8)
			{
				Endurance = Endurance - (SprintEnduranceCost * DeltaTime);
			}
		}
		
		FVector wispLocation = GetActorLocation();
		float cosValue = FMath::Cos(GetGameTimeSinceCreation() * MagicRopeMediatorComp->CharacterStrength);
		wispLocation.Y = (5 * cosValue) + wispLocation.Y;
		SetActorLocation(wispLocation);
	}
	auto const uw = Cast<UProgressBar>(WidgetComponent->GetUserWidgetObject());
	if (uw)
	{
		uw->SetPercent(Endurance / MaxEndurance);
	}

	if (GetWorld()->IsServer()) 
	{
		float LERPSpeed = bWispColorTargetReachedOnce ? WispColorMissingLERPSpeed : WispColorLERPSpeed;
		if (!bWispPulseBackDown)
		{
			if (WispColorCurrent != WispColorTarget || !bWispColorLocked)
			{
				float LERPSpeedDTime = DeltaTime * LERPSpeed;
				float NewRed = WispColorTarget.R - WispColorCurrent.R > 0
					? FMath::Min(WispColorTarget.R, WispColorCurrent.R + LERPSpeedDTime)
					: FMath::Max(WispColorTarget.R, WispColorCurrent.R - LERPSpeedDTime);

				float NewGreen = WispColorTarget.G - WispColorCurrent.G > 0
					? FMath::Min(WispColorTarget.G, WispColorCurrent.G + LERPSpeedDTime)
					: FMath::Max(WispColorTarget.G, WispColorCurrent.G - LERPSpeedDTime);

				float NewBlue = WispColorTarget.B - WispColorCurrent.B > 0
					? FMath::Min(WispColorTarget.B, WispColorCurrent.B + LERPSpeedDTime)
					: FMath::Max(WispColorTarget.B, WispColorCurrent.B - LERPSpeedDTime);

				WispColorCurrent = FLinearColor(NewRed, NewGreen, NewBlue);
				if (WispColorCurrent == WispColorTarget && !bWispColorLocked)
				{
					bWispPulseBackDown = true;
					bWispColorTargetReachedOnce = true;
				}
			}
		}
		else 
		{
			float LERPSpeedDTime = DeltaTime * LERPSpeed;
			float NewRed = FMath::Max(0.f, WispColorCurrent.R - LERPSpeedDTime);
			float NewGreen =  FMath::Max(0.f, WispColorCurrent.G - LERPSpeedDTime);
			float NewBlue = FMath::Max(0.f, WispColorCurrent.B - LERPSpeedDTime);
			WispColorCurrent = FLinearColor(NewRed, NewGreen, NewBlue);
			if (WispColorCurrent == FLinearColor::Black || bWispColorLocked)
			{
				bWispPulseBackDown = false;
			}
		}
	}
}

void AWisp::Multi_ToggleDashHUDRPC_Implementation()
{
	WidgetExclamationComponent->ToggleVisibility();
}

void AWisp::SetWispColorCurrent(FLinearColor newWispColorCurrent)
{
	if (GetWorld()->IsServer())
		Server_SetWispColorCurrentRPC_Implementation(newWispColorCurrent);
	else
		Server_SetWispColorCurrentRPC(newWispColorCurrent);
}
void AWisp::Server_SetWispColorCurrentRPC_Implementation(FLinearColor newWispColorCurrent)
{
	WispColorCurrent = newWispColorCurrent;
	OnRep_WispColorCurrent();
}
void AWisp::OnRep_WispColorCurrent()
{
	if(!WispColorMaterialAura)
		WispColorMaterialAura = GetMesh()->CreateAndSetMaterialInstanceDynamic(1);
	if(!WispColorMaterialCore)
		WispColorMaterialCore = GetMesh()->CreateAndSetMaterialInstanceDynamic(0);

	WispColorMaterialAura->SetVectorParameterValue(FName("color"), FVector(WispColorCurrent.R, WispColorCurrent.G, WispColorCurrent.B));
	WispColorMaterialCore->SetVectorParameterValue(FName("color"), FVector(WispColorCurrent.R, WispColorCurrent.G, WispColorCurrent.B));
}

void AWisp::SetWispColor(FLinearColor newWispColor)
{
	if (GetWorld()->IsServer())
		Server_SetWispColorRPC_Implementation(newWispColor);
	else
		Server_SetWispColorRPC(newWispColor);
}
void AWisp::Server_SetWispColorRPC_Implementation(FLinearColor newWispColor)
{
	WispColor = newWispColor;
	OnRep_WispColor();
}
void AWisp::OnRep_WispColor()
{
	GrabbableComp->AllowedColors.Empty();
	if (WispColor.R == 1)
	{
		GrabbableComp->AllowedColors.Add(FLinearColor::Red);
	}
	if (WispColor.G == 1)
	{
		GrabbableComp->AllowedColors.Add(FLinearColor::Green);
	}
	if (WispColor.B == 1)
	{
		GrabbableComp->AllowedColors.Add(FLinearColor::Blue);
	}

	MagicRopeMediatorComp->RopeOriginColor = WispColor;
	MagicRopeMediatorComp->PressureTresholdDifficulty = FMath::Clamp(GrabbableComp->AllowedColors.Num(), 1, 3);
	float Size = WispColor.R + WispColor.G + WispColor.B;
	float SizeMultiplier = Size == 1 ? 1 : Size == 2 ? 1.75 : Size == 3 ? 2.75 : 1;
	MagicRopeMediatorComp->PressureMinimumConnectorPointCount = SizeMultiplier;
	MagicRopeMediatorComp->CharacterWeightMultiplier = Size == 1 ? 0.5 : Size == 2 ? 1.00 : Size == 3 ? 2.00 : 1;
	GetMesh()->SetRelativeScale3D(FVector(SizeMultiplier));
	WidgetComponent->SetRelativeLocation(WidgetComponent->GetRelativeLocation() * SizeMultiplier);
	WidgetExclamationComponent->SetRelativeLocation(FVector(0,0, SizeMultiplier == 1 ? 200.0f : SizeMultiplier == 2 ? 200.0f : SizeMultiplier == 3 ? 150.0f : 200.0f));
	BaseFacialExpressionIndex = Size == 1 ? 0 : Size == 2 ? 4 : Size == 3 ? 3 : 0;

	OnColorChange.Broadcast(WispColor);
	if(!WispColorMaterialCore)
		WispColorMaterialCore = GetMesh()->CreateAndSetMaterialInstanceDynamic(0);
	WispColorMaterialCore->SetVectorParameterValue(FName("color"), FVector(WispColor.R, WispColor.G, WispColor.B));
}


void AWisp::SetWispColorTarget(FLinearColor newWispColorTarget)
{
	WispColorTarget = newWispColorTarget;
	OnRep_WispColorTarget();
}
void AWisp::OnRep_WispColorTarget()
{
	bWispColorTargetReachedOnce = false;
	if (!bWispColorLocked && GetWorld()->IsServer() && WispColorTarget == WispColor)
		bWispColorLocked = true;
}

void AWisp::Split()
{
	if (GetWorld()->IsServer())
	{
		Cast<AWispsAIController>(GetController())->BrainComponent->StopLogic("Actor Destroyed.");
		
		TArray<FLinearColor> SubWispColors;
		if (WispColor.R == 1)
			SubWispColors.Add(FLinearColor::Red);
		if (WispColor.G == 1)
			SubWispColors.Add(FLinearColor::Green);
		if (WispColor.B == 1)
			SubWispColors.Add(FLinearColor::Blue);

		FLinearColor ParticleColor1;
		FLinearColor ParticleColor2;
		if (SubWispColors.Num() == 2)
		{
			ParticleColor1 = SubWispColors[0];
			ParticleColor2 = SubWispColors[1];
		}
		else if (SubWispColors.Num() == 3)
		{
			ParticleColor1 = FLinearColor::White;
			ParticleColor2 = FLinearColor::Gray;
		}
		else
		{
			ParticleColor1 = WispColor;
			ParticleColor2 = WispColor;
		}
		Multi_SpawnParticleAtActorLocationRPC(SplitNiagaraSystem, ParticleColor1, ParticleColor2);

		//Spawn all sub-wisps
		for (const TPair<UMagicRopeMediator*, AMagicRope*>& pair : MagicRopeMediatorComp->Ropes)
		{
			FVector WispsSpawnCoordinates = GetActorLocation();
			FRotator WispsSpawnRotator = FRotator(0, FMath::RandRange(0, 360), 0);
			FTransform WispsSpawnTransform = FTransform(WispsSpawnRotator, WispsSpawnCoordinates);

			AWisp* NewWisp = GetWorld()->SpawnActorDeferred<AWisp>(SplitSpawnClass, WispsSpawnTransform, (AActor*)nullptr, (APawn*)nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
			// Whatever between creation and spawning
			NewWisp->InitialAttachFrom.Add(pair.Key);
			NewWisp->SetWispColor(pair.Key->RopeOriginColor);
			NewWisp->FinishSpawning(WispsSpawnTransform);
			OnWispSplitSpawn.Broadcast(NewWisp);
		}

		Destroy();
	}
}

void AWisp::DebugSplit()
{
	if (GetWorld()->IsServer())
	{
		Cast<AWispsAIController>(GetController())->BrainComponent->StopLogic("Actor Destroyed.");
		TArray<FLinearColor> SubWispColors;
		if (WispColor.R == 1)
			SubWispColors.Add(FLinearColor::Red);
		if(WispColor.G == 1)
			SubWispColors.Add(FLinearColor::Green);
		if(WispColor.B == 1)
			SubWispColors.Add(FLinearColor::Blue);

		FLinearColor ParticleColor1;
		FLinearColor ParticleColor2;
		if (SubWispColors.Num() == 2) 
		{
			ParticleColor1 = SubWispColors[0];
			ParticleColor2 = SubWispColors[1];
		}
		else if (SubWispColors.Num() == 3) 
		{
			ParticleColor1 = FLinearColor::White;
			ParticleColor2 = FLinearColor::Gray;
		}
		else 
		{
			ParticleColor1 = WispColor;
			ParticleColor2 = WispColor;
		}
		Multi_SpawnParticleAtActorLocationRPC(SplitNiagaraSystem, ParticleColor1, ParticleColor2);

		//Spawn all sub-wisps
		for (FLinearColor LinearColor : SubWispColors)
		{
			FVector WispsSpawnCoordinates = GetActorLocation();
			FRotator WispsSpawnRotator = FRotator(0, FMath::RandRange(0, 360), 0);
			FTransform WispsSpawnTransform = FTransform(WispsSpawnRotator, WispsSpawnCoordinates);

			AWisp* NewWisp = GetWorld()->SpawnActorDeferred<AWisp>(SplitSpawnClass, WispsSpawnTransform, (AActor*)nullptr, (APawn*)nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
			// Whatever between creation and spawning
			NewWisp->SetWispColor(LinearColor);
			NewWisp->FinishSpawning(WispsSpawnTransform);
			OnWispSplitSpawn.Broadcast(NewWisp);
		}
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SplitNiagaraSystem, GetMesh()->GetComponentLocation(), GetMesh()->GetComponentRotation());
		Destroy();
	}
}


void AWisp::Merge(AWisp* OtherWisp)
{
	if (CanMerge(OtherWisp)) 
	{
		OtherWisp->bMerging = true;
		bMerging = true;

		Cast<AWispsAIController>(GetController())->BrainComponent->StopLogic("Actor Destroyed.");
		Cast<AWispsAIController>(OtherWisp->GetController())->BrainComponent->StopLogic("Actor Destroyed.");
		
		FVector WispsSpawnCoordinates = GetActorLocation();
		FRotator WispsSpawnRotator = FRotator(0, FMath::RandRange(0, 360), 0);
		FTransform WispsSpawnTransform = FTransform(WispsSpawnRotator, WispsSpawnCoordinates);

		Multi_SpawnParticleAtActorLocationRPC(MergeNiagaraSystem, WispColor, OtherWisp->WispColor);
		AWisp* NewWisp = GetWorld()->SpawnActorDeferred<AWisp>(SplitSpawnClass, WispsSpawnTransform, (AActor*)nullptr, (APawn*)nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		// Whatever between creation and spawning
		for (const TPair<UMagicRopeMediator*, AMagicRope*>& pair : MagicRopeMediatorComp->Ropes)
		{
			NewWisp->InitialAttachFrom.Add(pair.Key);
		}
		for (const TPair<UMagicRopeMediator*, AMagicRope*>& pair : OtherWisp->MagicRopeMediatorComp->Ropes)
		{
			NewWisp->InitialAttachFrom.Add(pair.Key);
		}
		NewWisp->SetWispColor(FLinearColor(OtherWisp->WispColor.R + WispColor.R, OtherWisp->WispColor.G + WispColor.G, OtherWisp->WispColor.B + WispColor.B));
		NewWisp->FinishSpawning(WispsSpawnTransform);
		
		//particle TODO
		OtherWisp->Destroy();
		Destroy();
	}
}

bool AWisp::CanMerge(AWisp* OtherWisp)
{
	return 
		!bMerging && 
		!OtherWisp->bMerging && 
		GetWorld()->IsServer() && 
		OtherWisp->WispColor.R + WispColor.R < 2 && 
		OtherWisp->WispColor.G + WispColor.G < 2 && 
		OtherWisp->WispColor.B + WispColor.B < 2;
}

void AWisp::Multi_SpawnParticleAtActorLocationRPC_Implementation(UNiagaraSystem* NiagaraSystem, FLinearColor Color1, FLinearColor Color2)
{
	UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NiagaraSystem, GetActorLocation(), GetActorRotation(), FVector::OneVector, true, false);
	if (NiagaraComponent) 
	{
		NiagaraComponent->SetRenderCustomDepth(true);
		NiagaraComponent->SetCustomDepthStencilValue(255);
		NiagaraComponent->SetColorParameter(FName("Color_01"), Color1);
		NiagaraComponent->SetColorParameter(FName("Color_02"), Color2);
		NiagaraComponent->ActivateSystem();
	}
}


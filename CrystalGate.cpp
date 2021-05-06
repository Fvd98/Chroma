// Chroma

#include "Actors/CrystalGate.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ACrystalGate::ACrystalGate()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Crystal gate static mesh
	CrystalGateStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrystalGateStaticMesh"));
	CrystalGateStaticMesh->SetCustomDepthStencilValue(0);
	RootComponent = CrystalGateStaticMesh;

	// Front mist mesh
	CrystalGateMistStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrystalGateMistStaticMesh"));
	CrystalGateMistStaticMesh->SetCustomDepthStencilValue(0);
	CrystalGateMistStaticMesh->SetRelativeLocation(FVector(11.4f, 0, -53));
	CrystalGateMistStaticMesh->SetRelativeScale3D(FVector(1.25f, 1, 1));
	CrystalGateMistStaticMesh->SetupAttachment(RootComponent);	

	// Mist collider
	CrystalGateMistCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("CrystalGateMistCollider"));
	CrystalGateMistCollider->SetRelativeLocation(FVector(11.4f, 0, -53));
	CrystalGateMistCollider->SetRelativeScale3D(FVector(1.25f, 1, 1));
	CrystalGateMistCollider->BodyInstance.SetResponseToChannel(ECC_Pawn, ECR_Block);
	CrystalGateMistCollider->SetupAttachment(RootComponent);
	
	// Back mist mesh
	CrystalGateMistBackStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrystalGateMistBackStaticMesh"));
	CrystalGateMistBackStaticMesh->SetCustomDepthStencilValue(0);
	CrystalGateMistBackStaticMesh->SetRelativeLocation(FVector(-362, 0, -53));
	CrystalGateMistBackStaticMesh->SetRelativeRotation(FRotator(0, 180, 0));
	CrystalGateMistBackStaticMesh->SetRelativeScale3D(FVector(1.25f, 1, 1));
	CrystalGateMistBackStaticMesh->SetupAttachment(RootComponent);

	// Mist configurations
	CurrentMistOpacity = 1;
	TargetMistOpacity = 1;
	InterpSpeedMistOpacity = 3;

	bReplicates = true;
}

// Called when the game starts or when spawned
void ACrystalGate::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ACrystalGate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Updates mist
	if (!CrystalGateMistBackDynamicMaterial) 
		CrystalGateMistBackDynamicMaterial = CrystalGateMistBackStaticMesh->CreateAndSetMaterialInstanceDynamic(0);
	if (!CrystalGateMistDynamicMaterial)
		CrystalGateMistDynamicMaterial = CrystalGateMistStaticMesh->CreateAndSetMaterialInstanceDynamic(0);
	if (CurrentMistOpacity != TargetMistOpacity) 
	{
		CurrentMistOpacity = FMath::FInterpConstantTo(CurrentMistOpacity, TargetMistOpacity, DeltaTime, InterpSpeedMistOpacity);
		CrystalGateMistBackDynamicMaterial->SetScalarParameterValue(FName("opacity"), CurrentMistOpacity);
		CrystalGateMistDynamicMaterial->SetScalarParameterValue(FName("opacity"), CurrentMistOpacity);
	}
}

// Turn off the mist
void ACrystalGate::Server_DisableMistRPC_Implementation()
{
	bMistDisabled = true;
	OnRep_bMistDisabled();
}

void ACrystalGate::OnRep_bMistDisabled() 
{
	if (bMistDisabled) 
	{
		TargetMistOpacity = 0;
		CrystalGateMistCollider->BodyInstance.SetResponseToChannel(ECC_Pawn, ECR_Ignore);
	}
}

// Variables replication
void ACrystalGate::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACrystalGate, bMistDisabled);
}
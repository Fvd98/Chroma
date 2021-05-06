// Chroma


#include "Actors/CrystalSpawner.h"
#include "Landscape.h"
#include "Engine/World.h"
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>
#include "Engine/Selection.h"

// Sets default values
ACrystalSpawner::ACrystalSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Root collider
	InitCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("Rootcomponent"));
	RootComponent = InitCollider;
	InitCollider->SetBoxExtent(FVector(5, 5, 5));

	// Spawner mesh
	SpawnerStaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SpawnerStaticMeshComp->SetupAttachment(InitCollider);
	SpawnerStaticMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SpawnerStaticMeshComp->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
	SpawnerStaticMeshComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	SpawnerStaticMeshComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	// Radius configurations
	SpawningRadius = 150;

#if WITH_EDITOR
	this->bRunConstructionScriptOnDrag = true;
#endif

#if WITH_EDITORONLY_DATA
	USelection::SelectObjectEvent.AddUObject(this, &ACrystalSpawner::OnObjectSelected);
#endif
}

#if WITH_EDITORONLY_DATA
void ACrystalSpawner::OnObjectSelected(UObject* Object)
{
	if (Object == this)
	{
		OnConstruction(GetActorTransform());
	}
}
#endif

void ACrystalSpawner::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	SpawnerStaticMeshComp->SetStaticMesh(
		SpawnerStaticMesh ? SpawnerStaticMesh :
		Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), NULL, TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'")))
	);
	SpawnerStaticMeshComp->SetRelativeLocation(SpawnerStaticMesh ? FVector::ZeroVector : FVector(0, 0, 150));
	SpawnerStaticMeshComp->SetRelativeScale3D(SpawnerStaticMesh ? FVector::OneVector : FVector(1, 1, 1));
	SpawnerStaticMeshComp->SetMaterial(0,
		SpawnerStaticMesh ? nullptr :
		Cast<UMaterialInstance>(StaticLoadObject(UMaterialInstance::StaticClass(), NULL, TEXT("MaterialInstance'/Game/Geometry/Materials/Inst_Moon.INST_Moon'")))
	);

#if WITH_EDITOR
	if (GetWorld() && !GetWorld()->GetGameInstance())
	{
		UKismetSystemLibrary::FlushPersistentDebugLines(this);
		UKismetSystemLibrary::DrawDebugCylinder(this, GetActorLocation(), GetActorLocation() + FVector(0, 0, 150), SpawningRadius ? SpawningRadius : 5, 12, FLinearColor::Green, 10000, 5);
	}
#endif
}

// Called when the game starts or when spawned
void ACrystalSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (!SpawnerStaticMesh)
	{
		SpawnerStaticMeshComp->SetStaticMesh(nullptr);
	}
}

// Puts a random position within given OutResult vector from within the given radius
bool ACrystalSpawner::GetRandomSpawnPositionWhitinRadius(float Radius, FVector& OutResult)
{
	UWorld* World = GetWorld();

	// Gets a random point within the given radius around the spawner actor
	FVector SpawnerLocation = GetActorLocation();
	float SearchAngle = FMath::RandRange(0.f, 1.f) * 2 * PI;
	float SearchRadius = Radius * FMath::Sqrt(FMath::RandRange(0.f, 1.f));
	FVector RandomXYPointCircle = FVector(SearchRadius * FMath::Cos(SearchAngle), SearchRadius * FMath::Sin(SearchAngle), 0);
	RandomXYPointCircle += FVector(SpawnerLocation.X, SpawnerLocation.Y, 0);

	// Find the active landscape
	TArray<AActor*> FoundLandscapesT;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALandscape::StaticClass(), FoundLandscapesT);
	if (FoundLandscapesT.Num())
	{
		ALandscape* Landscape = (ALandscape*)FoundLandscapesT[0];

		FCollisionQueryParams collisionParams(FName(TEXT("FoliageClusterPlacementTrace")), true, this);

		// Traces a line from top to bottom returning the fist hit Static object point of collision
		FVector startTraceHeight = FVector(0, 0, 1000);
		const FVector startVector = RandomXYPointCircle + startTraceHeight;
		const FVector endVector = RandomXYPointCircle - startTraceHeight;

		FHitResult result(ForceInit);
		if (Landscape->ActorLineTraceSingle(result, startVector, endVector, ECC_Visibility, collisionParams)) {
			OutResult = result.ImpactPoint + FVector(0, 0, 50);
			return true;
		}
	}

	return false;
}

// Called every frame
void ACrystalSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Spawn a wisp of the given class and color
AWisp* ACrystalSpawner::SpawnWispDynamic(FLinearColor WispColor, TSubclassOf<AWisp> WispSpawnClass)
{
	// Figures where and how to spawn the Wisp (Transform)
	FVector WispSpawnCoordinates;
	if (bSpawnOnLandscape)
		GetRandomSpawnPositionWhitinRadius(SpawningRadius, WispSpawnCoordinates);
	else
		WispSpawnCoordinates = GetActorLocation() + FVector(0, 0, 50);
	FRotator WispSpawnRotator = FRotator(0, FMath::RandRange(0, 360), 0);
	FTransform WispSpawnTransform = FTransform(WispSpawnRotator, WispSpawnCoordinates);

	// Spawns the Wisp
	AWisp* NewWisp = GetWorld()->SpawnActorDeferred<AWisp>(WispSpawnClass, WispSpawnTransform, this, (APawn*)nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	// Sets preSpawn variables
	NewWisp->FinishSpawning(WispSpawnTransform);

	// Sets postSpawn variables
	NewWisp->SetWispColor(WispColor);
	
	return NewWisp;
}


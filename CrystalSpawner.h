// Chroma

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Chroma/Public/Ennemies/Wisp.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "CrystalSpawner.generated.h"

UCLASS()
class CHROMA_API ACrystalSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACrystalSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	void OnObjectSelected(UObject* Object);

	UBoxComponent* InitCollider;
	UStaticMeshComponent* SpawnerStaticMeshComp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
		UStaticMesh* SpawnerStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", DisplayName = "Spawn on landscape?")
		bool bSpawnOnLandscape = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (EditCondition = "bSpawnOnLandscape"))
		float SpawningRadius;

	bool GetRandomSpawnPositionWhitinRadius(float Radius, FVector& OutResult);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Spawner")
		AWisp* SpawnWispDynamic(FLinearColor WispColor, TSubclassOf<AWisp> WispSpawnClass);

};

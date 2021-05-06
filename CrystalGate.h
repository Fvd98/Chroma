// Chroma

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "CrystalGate.generated.h"

UCLASS()
class CHROMA_API ACrystalGate : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACrystalGate();

	// Components
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrystalGate")
		UStaticMeshComponent* CrystalGateStaticMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrystalGate")
		UStaticMeshComponent* CrystalGateMistStaticMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrystalGate")
		UStaticMeshComponent* CrystalGateMistBackStaticMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrystalGate")
		UBoxComponent* CrystalGateMistCollider;

	// Mist management
	UFUNCTION(NetMulticast, Reliable)
	void Server_DisableMistRPC();
		void Server_DisableMistRPC_Implementation();
	UPROPERTY(ReplicatedUsing = OnRep_bMistDisabled)
		bool bMistDisabled;
	UFUNCTION()
		void OnRep_bMistDisabled();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mist")
		UMaterialInstanceDynamic* CrystalGateMistBackDynamicMaterial;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mist")
		UMaterialInstanceDynamic* CrystalGateMistDynamicMaterial;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mist")
		float CurrentMistOpacity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mist")
		float TargetMistOpacity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mist")
		float InterpSpeedMistOpacity;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};

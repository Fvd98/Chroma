// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "ChromaPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPostProcessViewInputDelegate, FVector, ViewInputVector);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPawnPossessedDelegate, AChromaCharacter*, PossessedCharacter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPostProcessMoveInputDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInteractPressedDelegate);

UCLASS()
class AChromaPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AChromaPlayerController();

	UFUNCTION(BlueprintCallable, Category = "Runtime Inspector")
		FString GetCurrentViewMode();

	UPROPERTY(BlueprintAssignable, Category = "RightJoystick")
		FPostProcessViewInputDelegate OnPostProcessViewInput;

	UPROPERTY(BlueprintAssignable, Category = "Events")
		FPawnPossessedDelegate OnChromaCharacterPossessed;

	UPROPERTY(BlueprintAssignable, Category = "RightJoystick")
		FPostProcessMoveInputDelegate OnPostProcessMoveInput;

	UPROPERTY(BlueprintAssignable, Category = "Events")
		FInteractPressedDelegate OnInteractPressed;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Movement")
		bool bCanCharacterMove;


	virtual ASpectatorPawn* SpawnSpectatorPawn() override;
protected:

	// Begin PlayerController interface
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;

	// End PlayerController interface


	FVector LookInputVector;

	void ShowMenu();

	void GrabPressed();
	void GrabReleased();

	void MoveRight(float Value);
	void MoveForward(float Value);

	void ReadyPressed();
	void InteractPressed();

	void LookRight(float Value);
	void LookForward(float Value);

	void ShowRadialMenu();
	void HideRadialMenu();

public:
	void ShowPing(FLinearColor _color);
	void RemovePing(FLinearColor _color);

	void ShowLoadingScreen();
	void RemoveLoadingScreen();

	UFUNCTION(BlueprintCallable)
	void ReturnToMainMenu();
};



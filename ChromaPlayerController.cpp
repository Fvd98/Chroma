// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChromaPlayerController.h"
#include "Chroma/Public/Actors/ChromaCharacter.h"
#include "Engine/World.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Chroma/Public/ChromaPlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/MagicRopeMediator.h"
#include "GameFramework/HUD.h"
#include "Chroma/Public/UI/HUDBase.h"
#include "GameFramework/InputSettings.h"
#include "Engine/GameInstance.h"
#include "ChromaGameInstance.h"


AChromaPlayerController::AChromaPlayerController()
{
	bShowMouseCursor = true;
	bForceFeedbackEnabled = true;
	bCanCharacterMove = true;
}

FString AChromaPlayerController::GetCurrentViewMode()
{
	if (IsValid(this))
	{
		if(UGameViewportClient* GameViewportClient = GetWorld()->GetGameViewport())
		{
			if (ULocalPlayer* LocalPlayer = GetLocalPlayer()) 
			{
				bool ignore = GameViewportClient->IgnoreInput();
				EMouseCaptureMode capt = GameViewportClient->GetMouseCaptureMode();

				if (ignore == false && capt == EMouseCaptureMode::CaptureDuringMouseDown)
				{
					return FString("Game And UI");
				}
				else if (ignore == true && capt == EMouseCaptureMode::NoCapture)
				{
					return FString("UI Only");
				}
				else
				{
					return FString("Game Only");
				}
			}
		}
	}

	return FString("Weird crap happened");
}

ASpectatorPawn* AChromaPlayerController::SpawnSpectatorPawn()
{
	return Super::SpawnSpectatorPawn();
}

void AChromaPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
}

void AChromaPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	PlayerState->GetUniqueID();

	//	int32 id = GetLocalPlayer()->GetControllerId();

	InputComponent->BindAxis("MoveForward", this, &AChromaPlayerController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AChromaPlayerController::MoveRight);

	InputComponent->BindAxis("LookForward", this, &AChromaPlayerController::LookForward);
	InputComponent->BindAxis("LookRight", this, &AChromaPlayerController::LookRight);

	InputComponent->BindAction("OpenMenu", IE_Pressed, this, &AChromaPlayerController::ShowMenu);

	FInputBinding RadialMenuKeyboardUp = InputComponent->BindAction("RadialMenuKeyboard", IE_Pressed, this, &AChromaPlayerController::ShowRadialMenu);
	RadialMenuKeyboardUp.bConsumeInput = false;
	FInputBinding RadialMenuKeyboardDown = InputComponent->BindAction("RadialMenuKeyboard", IE_Released, this, &AChromaPlayerController::HideRadialMenu);
	RadialMenuKeyboardDown.bConsumeInput = false;

	FInputBinding UseGrabUp = InputComponent->BindAction("UseGrab", IE_Pressed, this, &AChromaPlayerController::GrabPressed);
	UseGrabUp.bConsumeInput = false;
	FInputBinding UseGrabDown = InputComponent->BindAction("UseGrab", IE_Released, this, &AChromaPlayerController::GrabReleased);
	UseGrabDown.bConsumeInput = false;

	//InputComponent->BindAction("Ready", IE_Pressed, this, &AChromaPlayerController::ReadyPressed);
	InputComponent->BindAction("Interact", IE_Pressed, this, &AChromaPlayerController::InteractPressed);
}

void AChromaPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AChromaPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	Super::PostProcessInput(DeltaTime, bGamePaused);
	OnPostProcessMoveInput.Broadcast();
	OnPostProcessViewInput.Broadcast(LookInputVector);
	LookInputVector = FVector::ZeroVector;
}

/* Crashes the editor */
void AChromaPlayerController::ShowMenu()
{
	if (GetPawn())
	{
		int32 id = GetPawn()->GetPlayerState()->GetPlayerId();

	//	UE_LOG(LogTemp, Warning, TEXT("ShowMenu, player ID : %d"), id);
	}

	if (AHUD* hud = GetHUD())
	{
		if (AHUDBase* hudingame = Cast<AHUDBase>(hud))
		{
			if (hudingame->_widget->GetIsVisible())
			{
				hudingame->RemoveMenu();
			}
			else
			{
				hudingame->ShowMenu();

			}
		}
	}
}

void AChromaPlayerController::ShowRadialMenu()
{
	/*if (GetPawn())
	{
		int32 id = GetPawn()->GetPlayerState()->GetPlayerId();

		UE_LOG(LogTemp, Warning, TEXT("ShowRadialMenu, player ID : %d"), id);
	}*/
	
	if (AHUD* hud = GetHUD())
	{
		if (AHUDBase* hudingame = Cast<AHUDBase>(hud))
		{
			hudingame->ShowRadialMenu();
		}
	}
}

void AChromaPlayerController::HideRadialMenu()
{
	TArray<FInputActionKeyMapping> RadialInputActionKeyMapping;
	UInputSettings::GetInputSettings()->GetActionMappingByName(FName("RadialMenuKeyboard"), RadialInputActionKeyMapping);
	bool bKeyStillPressed = false;
	for (FInputActionKeyMapping InputActionKeyMapping : RadialInputActionKeyMapping) 
	{
		if (IsInputKeyDown(InputActionKeyMapping.Key))
			bKeyStillPressed = true;
	}
	if (!bKeyStillPressed) 
	{
		if (AHUD* hud = GetHUD())
		{
			if (AHUDBase* hudingame = Cast<AHUDBase>(hud))
			{
				hudingame->RemoveRadialMenu();
			}
		}
	}
}

void AChromaPlayerController::ShowPing(FLinearColor _color)
{
	AChromaCharacter* cc = GetPawn<AChromaCharacter>();

	if (AHUD* hud = GetHUD())
	{
		if (AHUDBase* hudingame = Cast<AHUDBase>(hud))
		{
			hudingame->ShowPing(_color, cc->PingMap[_color]);
		}
	}
}

void AChromaPlayerController::RemovePing(FLinearColor _color)
{
	if (AHUD* hud = GetHUD())
	{
		if (AHUDBase* hudingame = Cast<AHUDBase>(hud))
		{
			hudingame->RemovePing(_color);
		}
	}
}



void AChromaPlayerController::ShowLoadingScreen()
{
	if (AHUD* hud = GetHUD())
	{
		if (AHUDBase* hudingame = Cast<AHUDBase>(hud))
		{
			hudingame->ShowLoadingScreen();
		}
	}
}

void AChromaPlayerController::RemoveLoadingScreen()
{
	if (AHUD* hud = GetHUD())
	{
		if (AHUDBase* hudingame = Cast<AHUDBase>(hud))
		{
			hudingame->RemoveLoadingScreen();
		}
	}
}

void AChromaPlayerController::ReturnToMainMenu()
{
	GetGameInstance()->ReturnToMainMenu();
}

void AChromaPlayerController::MoveForward(float Value)
{
	if (bCanCharacterMove && (this != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = this->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		if (this->GetPawn())
			this->GetPawn()->AddMovementInput(Direction, Value);
	}
}

void AChromaPlayerController::MoveRight(float Value)
{
	if (bCanCharacterMove && (this != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = this->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		if (this->GetPawn())
			this->GetPawn()->AddMovementInput(Direction, Value);
	}
}

void AChromaPlayerController::LookForward(float Value)
{
	if ((this != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = this->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		if (this->GetPawn())
			LookInputVector += Direction * Value;
	}
}

void AChromaPlayerController::LookRight(float Value)
{
	if ((this != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = this->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		// add movement in that direction
		if (this->GetPawn())
			LookInputVector += Direction * Value;
	}
}

void AChromaPlayerController::GrabPressed()
{
	if (this->GetPawn())
	{
		AChromaCharacter* character = (AChromaCharacter*)this->GetPawn();
		if (character) character->OnGrabPressed();
	}

}

void AChromaPlayerController::GrabReleased()
{
	if (this->GetPawn())
	{
		AChromaCharacter* ChromaCharacter = (AChromaCharacter*)GetPawn();
		if (ChromaCharacter)
		{
			ChromaCharacter->OnGrabReleased();
		} 
	}
}

void AChromaPlayerController::ReadyPressed()
{
	if (this->GetPawn())
	{
		AChromaCharacter* character = (AChromaCharacter*)this->GetPawn();
		if (character) character->Set_IsReady(!character->IsReady);
	}
}

void AChromaPlayerController::InteractPressed()
{
	if (AHUD* hud = GetHUD())
	{
		if (AHUDBase* hudingame = Cast<AHUDBase>(hud))
		{
			if (hudingame->_activePopUp != nullptr)
			{
				hudingame->ClosePopUp();
			}
			else
			{
				OnInteractPressed.Broadcast();
			}
		}
	}
}

void AChromaPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	AChromaCharacter* PossessedCharacter = Cast<AChromaCharacter>(InPawn);
	if (PossessedCharacter)
	{
	//	UE_LOG(LogTemp, Warning, TEXT("Possessed CC"));
		OnChromaCharacterPossessed.Broadcast(PossessedCharacter);
	}
}
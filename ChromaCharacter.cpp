// Copyright Epic Games, Inc. All Rights Reserved.

#include "Chroma/Public/Actors/ChromaCharacter.h"
#include "Components/WidgetComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include"Chroma/Public/UI/W_RadialMenu.h"
#include "Chroma/Public/Gamemode/ChromaNetworkGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Materials/MaterialInstanceConstant.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "Chroma/Public/ChromaGameState.h"
#include "Chroma/Public/ChromaPlayerController.h"
#include "Math/UnrealMathUtility.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Actors/PingActor.h"
#include "Components/DecalComponent.h"

AChromaCharacter::AChromaCharacter(): 
	grabZone_component(CreateDefaultSubobject<UGrabZone>(TEXT("GrabZone"))),
	InteractableComponent(CreateDefaultSubobject<UInteractableComponent>(TEXT("InteractableComponent"))),
	NotifComp(CreateDefaultSubobject<UTutorialNotificationComponent>(TEXT("TutorialNotificationComponent")))
{
	if (grabZone_component)
	{
		grabZone_component->SetupAttachment(RootComponent);

		grabZone_component->AssociatedColor = playerColor;
	}


	if (InteractableComponent)
	{
		InteractableComponent->SetupAttachment(RootComponent);
		InteractableComponent->sphere_component->SetSphereRadius(DefaultRadius);
	}

	if (NotifComp) 
	{
		// Tutorial notifications
		NotifComp->SetupAttachment(RootComponent);
	}

	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 90.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 1000.f; // camera distance
	CameraBoom->SetRelativeRotation(FRotator(-55.f, 0.f, 0.f)); // camera angle
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Magic rope mediator
	MagicRopeMediator = CreateDefaultSubobject<UMagicRopeMediator>(TEXT("MagicRopeMediator"));

	// Create a decal in the world to show the cursor's location
	PingFeedback = CreateDefaultSubobject<UDecalComponent>("CursorToWorld");
	PingFeedback->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> DecalMaterialAsset(TEXT("MaterialInstanceConstant'/Game/TopDownBP/Blueprints/INST_Cursor_Decal.INST_Cursor_Decal'"));
	if (DecalMaterialAsset.Succeeded())
	{
		PingFeedback->SetDecalMaterial(DecalMaterialAsset.Object);
	}
	PingFeedback->DecalSize = FVector(50.0f, DefaultRadius*2, DefaultRadius*2);
	PingFeedback->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f).Quaternion());
	PingFeedback->SetVisibility(false);
	CurrentPingFeedbackRadius = 1;
	PingFeedbackRadiusLerpSpeed = 0.75;

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	mana = MaxMana;
	ManaDrainPS = 0.0f;
	minimumMana = MaxMana * 10 / 100;

	CountdownTime = 1;
}

void AChromaCharacter::BeginPlay()
{
	Super::BeginPlay();
	FTransform SpawnLocation = FTransform(GetActorLocation());
	FRotator SpawnRotation = GetControlRotation();
	OnRep_PlayerMana();

	if (GetWorld()->IsServer())
		OnDestroyed.AddDynamic(this, &AChromaCharacter::DestroyHandler);
}



void AChromaCharacter::OnConstruction(const FTransform& Transform)
{
	PingFeedback->DecalSize = FVector(50.0f, DefaultRadius, DefaultRadius);
}

void AChromaCharacter::EndPlay(EEndPlayReason::Type)
{
	UE_LOG(LogTemp, Warning, TEXT("EndPlay"));

	//if (GetWorld()->IsServer())
	//{
	//	if (AChromaNetworkGameMode* acngm = Cast<AChromaNetworkGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	//	{

	//		acngm->AddColor(playerColor);

	//	}
	//}
}

void AChromaCharacter::SetManaTimer()
{
	SetCurrentMana(mana + 10);
}

void AChromaCharacter::ShowPing()
{
	if (bIsLocalPlayerCharacter)
	{
	}
}

void AChromaCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bIsLocalPlayerCharacter)
	{
		SetCurrentMana(mana - (ManaDrainPS * DeltaSeconds));
	}

	if (CurrentPingFeedbackRadius != 1)
	{
		CurrentPingFeedbackRadius = FMath::FInterpConstantTo(CurrentPingFeedbackRadius, 1, DeltaSeconds, PingFeedbackRadiusLerpSpeed);
		PingFeedbackColorMaterial->SetScalarParameterValue(FName("RingPositionRadius"), CurrentPingFeedbackRadius);
		if (CurrentPingFeedbackRadius == 1)
		{
			PingFeedback->SetVisibility(false);
		}
	}
}

void AChromaCharacter::OnGrabPressed()
{
	if (grabZone_component != nullptr && bIsLocalPlayerCharacter)
	{
		bIsGrabbing = true;
		Server_SetbIsGrabbingRPC(bIsGrabbing);
		grabZone_component->ActivateZone();
	}
}

void AChromaCharacter::OnGrabReleased()
{
	if (grabZone_component != nullptr && bIsLocalPlayerCharacter)
	{
		bIsGrabbing = false;
		Server_SetbIsGrabbingRPC(bIsGrabbing);
		grabZone_component->DeactivateZone();
		MagicRopeMediator->DetachAllRope();
	}
}

void AChromaCharacter::Server_SetbIsGrabbingRPC_Implementation(bool NewbIsGrabbing)
{
	bIsGrabbing = NewbIsGrabbing;
}

// Properties' replication
void AChromaCharacter::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicates playerColor.
	DOREPLIFETIME(AChromaCharacter, playerColor);
	DOREPLIFETIME(AChromaCharacter, mana);
	DOREPLIFETIME(AChromaCharacter, IsReady);
	DOREPLIFETIME(AChromaCharacter, DefaultRadius);
	DOREPLIFETIME(AChromaCharacter, bIsGrabbing);
}

// When pawn is assigned a player controller, assign it's color
void AChromaCharacter::Restart()
{
	Super::Restart();
	//UE_LOG(LogTemp, Warning, TEXT("Restart"));

	if (AChromaPlayerController* ChromaPlayerController = GetController<AChromaPlayerController>())
	{
		bIsLocalPlayerCharacter = UGameplayStatics::GetPlayerController(GetWorld(), 0) == ChromaPlayerController;
		//UE_LOG(LogTemp, Warning, TEXT("IsChromaPlayerController"));
		//[TODO] - RPC
		if (GetWorld()->IsServer()) 
		{
			if (GetPlayerState<AChromaPlayerState>()->playerColor == FLinearColor(1, 0, 0)
				|| GetPlayerState<AChromaPlayerState>()->playerColor == FLinearColor(0, 1, 0)
				|| GetPlayerState<AChromaPlayerState>()->playerColor == FLinearColor(0, 0, 1))
			{
				Set_PlayerColor(GetPlayerState<AChromaPlayerState>()->playerColor);
			}
			else
			{
				if (AChromaNetworkGameMode* acngm = Cast<AChromaNetworkGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
				{
					if(acngm->colorHeap.Num())
						Set_PlayerColor(acngm->colorHeap.Pop());
				}
			}
		}
		
		OnPlayerControllerAssigned.Broadcast(ChromaPlayerController);
	}
}

void AChromaCharacter::DestroyHandler(AActor* DestroyedActor)
{

	if (GetWorld()->IsServer())
	{
	//	UE_LOG(LogTemp, Warning, TEXT("IsServer BeginDestroy"));

		if (AChromaNetworkGameMode* acngm = Cast<AChromaNetworkGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
		{
			if ((playerColor.R + playerColor.G + playerColor.B) == 1)
			{
				acngm->AddColor(playerColor);
			}
		}
		OnPlayerDestroyed.Broadcast();
	}
}

void AChromaCharacter::ControllerVibrationHandler(TEnumAsByte<EDynamicForceFeedbackAction::Type> Action, float Intensity = 0)
{
	this->OnVibrationChange.Broadcast(Action, Intensity);
}

void AChromaCharacter::Ping(AChromaCharacter* _chromaCharacterSource, FLinearColor _color)
{

	TArray<AActor*> ArrayActor;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), StaticClass(), ArrayActor);

	if (bIsLocalPlayerCharacter)
	{
		if (!PingFeedbackColorMaterial)
		{
			PingFeedbackColorMaterial = PingFeedback->CreateDynamicMaterialInstance();
			PingFeedback->SetDecalMaterial(PingFeedbackColorMaterial);
		}
		if (PingFeedbackColorMaterial)
		{
			// Change mesh color to current playerColor 
			PingFeedbackColorMaterial->SetVectorParameterValue("Color", FLinearColor(FVector(_color.R, _color.G, _color.B)));

			CurrentPingFeedbackRadius = 0;
			PingFeedbackColorMaterial->SetScalarParameterValue(FName("RingPositionRadius"), CurrentPingFeedbackRadius);
			PingFeedback->SetVisibility(true);

		}
	}

	for (AActor* actor : ArrayActor)
	{
		AChromaCharacter* ChromaCharacter = Cast<AChromaCharacter>(actor);

		if (ChromaCharacter->playerColor == FLinearColor::Red && _color.R == 1
			|| ChromaCharacter->playerColor == FLinearColor::Green && _color.G == 1
			|| ChromaCharacter->playerColor == FLinearColor::Blue && _color.B == 1)
		{
			Server_SendPingRPC(_chromaCharacterSource, ChromaCharacter, playerColor);
			Server_SendPingRPC(_chromaCharacterSource, _chromaCharacterSource, _color);
		}

	}


}

void AChromaCharacter::Server_SendPingRPC_Implementation(AChromaCharacter* _chromaCharacterSource, AChromaCharacter* _chromaCharacterTarget, FLinearColor _color)
{
	_chromaCharacterTarget->Multi_ReceivePingRPC(_chromaCharacterSource,_chromaCharacterTarget,_color);
}


void AChromaCharacter::Multi_ReceivePingRPC_Implementation(AChromaCharacter* _chromaCharacterSource, AChromaCharacter* _chromaCharacterTarget, FLinearColor _color)
{
	if (this->bIsLocalPlayerCharacter /*&& _color != playerColor*/)
	{
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, _color.ToFColor(true), TEXT("ReceivePing From"));
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, playerColor.ToFColor(true), TEXT("ReceivePing To "));
		_chromaCharacterTarget->SpawnPing(_chromaCharacterSource->GetActorLocation(), _color, _chromaCharacterSource == _chromaCharacterTarget);

		_chromaCharacterTarget->GetController<AChromaPlayerController>()->ShowPing(_color);
	}
}


void AChromaCharacter::UpdatePingMap(AActor* _actor, FLinearColor _color)
{
	//UE_LOG(LogTemp, Warning, TEXT("ACtor : %s"), *_actor->GetName());

	this->GetController<AChromaPlayerController>()->RemovePing(_color);

	PingMap.Remove(_color);


}

AHUDBase* AChromaCharacter::GetHUD()
{
	AHUDBase* hud = Cast<AHUDBase>(UGameplayStatics::GetPlayerController(this, 0)->GetHUD());
	return hud;
}

void AChromaCharacter::ShowLoadingScreen()
{
	GetController<AChromaPlayerController>()->ShowLoadingScreen();
}

void AChromaCharacter::RemoveLoadingScreen()
{
	GetController<AChromaPlayerController>()->RemoveLoadingScreen();
}

void AChromaCharacter::AdvanceTimer()
{
	int scale = 1;

	--CountdownTime;

	if (CountdownTime < 1)
	{
		// We're done counting down, so stop running the timer.
		GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
		//Perform any special actions we want to do when the timer ends.
		CountdownHasFinished();
	}
} 

void AChromaCharacter::CountdownHasFinished_Implementation()
{
	PingFeedback->SetVisibility(false);
}

// Handler for color change
void AChromaCharacter::Set_PlayerColor(FLinearColor newColor)
{
	if (HasAuthority())
	{
		// Server
		playerColor = newColor;
		GetPlayerState<AChromaPlayerState>()->SetPlayerColor(playerColor);

		OnRep_PlayerColor();
	}
	else
	{
		// Client -> Server
		Server_ChangeColor(newColor);
	}
}

// Server RPC for color change
void AChromaCharacter::Server_ChangeColor_Implementation(FLinearColor newColor)
{
	playerColor = newColor;
	OnRep_PlayerColor();
}

void AChromaCharacter::SpawnPing(FVector Loc, FLinearColor PingColor, bool b)
{
	if (b)
	{
		FActorSpawnParameters SpawnParams;
		FRotator rotator;
		AActor* SpawnedActorRef = GetWorld()->SpawnActor<AActor>(PingActorFlag, Loc, rotator, SpawnParams);
		APingActor* SpawnedPingActor = Cast<APingActor>(SpawnedActorRef);
		float PingRed = PingColor.R == 1 ? PingColor.R : FMath::Max(PingColor.R, 0.2f);
		float PingGreen = PingColor.G == 1 ? PingColor.G : FMath::Max(PingColor.G, 0.2f);
		float PingBlue = PingColor.B == 1 ? PingColor.B : FMath::Max(PingColor.B, 0.2f);
		FLinearColor WashedPingColor = FLinearColor(PingRed, PingGreen, PingBlue);
		SpawnedPingActor->SetPingColor(WashedPingColor);
		SpawnedPingActor->ChromaCharacter = this;

		PingMap.Add(PingColor, SpawnedActorRef);
	}
	else
	{
		FActorSpawnParameters SpawnParams;
		FRotator rotator;
		AActor* SpawnedActorRef = GetWorld()->SpawnActor<AActor>(PingActor, Loc, rotator, SpawnParams);
		APingActor* SpawnedPingActor = Cast<APingActor>(SpawnedActorRef);
		float PingRed = PingColor.R == 1 ? PingColor.R : FMath::Max(PingColor.R, 0.2f);
		float PingGreen = PingColor.G == 1 ? PingColor.G : FMath::Max(PingColor.G, 0.2f);
		float PingBlue = PingColor.B == 1 ? PingColor.B : FMath::Max(PingColor.B, 0.2f);
		FLinearColor WashedPingColor = FLinearColor(PingRed, PingGreen, PingBlue);
		SpawnedPingActor->SetPingColor(WashedPingColor);
		SpawnedPingActor->ChromaCharacter = this;

		PingMap.Add(PingColor, SpawnedActorRef);
	}

}

void AChromaCharacter::TransferMana(float _mana)
{
	mana += _mana;
}

void AChromaCharacter::SetCurrentMana(float _mana)
{
	if (GetWorld()->IsServer())
	{
		// Server
		mana = FMath::Clamp(_mana, minimumMana, MaxMana);
		OnRep_PlayerMana();
	}
	else
	{
		// Client -> Server
		Server_ChangeManaRPC(_mana);
	}
}

void AChromaCharacter::OnRep_PlayerMana()
{
	if (MagicalGrabZone)
	{
		float PlayerColorExtent = (mana / 100) * DefaultRadius;
		if (UBoxComponent* CollisionZone = MagicalGrabZone->FindComponentByClass<UBoxComponent>())
		{
			CollisionZone->SetRelativeLocation(FVector(PlayerColorExtent/2 +30, 0, 0));
			CollisionZone->SetBoxExtent(FVector(PlayerColorExtent - 60, 200, 32));
		}

		InteractableComponent->sphere_component->SetSphereRadius((mana / 100) * PlayerColorExtent);
	}


}

void AChromaCharacter::Server_ChangeManaRPC_Implementation(float _mana)
{
	mana = FMath::Clamp(_mana, 0.f, MaxMana);
	OnRep_PlayerMana();
}

// On replication of player color, assign the color to the pawn material
void AChromaCharacter::OnRep_PlayerColor()
{
	if (!playerColorMaterial)
	{
		playerColorMaterial = GetMesh()->CreateAndSetMaterialInstanceDynamic(0);
	}
	if (playerColorMaterial)
	{
		// Change mesh color to current playerColor 
		playerColorMaterial->SetVectorParameterValue("emissive_color", playerColor);

		if (MagicalGrabZone)
			MagicalGrabZone->AssociatedColor = playerColor;

		if (MagicRopeMediator)
			MagicRopeMediator->RopeOriginColor = playerColor;
	}
}

void AChromaCharacter::OnRep_IsReady()
{
	OnPlayerReady.Broadcast(playerColor);
	if (IsReady)
	{
		UE_LOG(LogTemp, Warning, TEXT("Now ready!"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Now not ready!"));
	}
}

void AChromaCharacter::Set_IsReady(bool newReady)
{
	Server_SetIsReadyRPC(newReady);
}

void AChromaCharacter::Server_SetIsReadyRPC_Implementation(bool newReady)
{
	IsReady = newReady;
	OnRep_IsReady();
}





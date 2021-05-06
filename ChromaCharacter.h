// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Chroma/Public/MagicalGrabZone.h"
#include "Chroma/Public/ChromaPlayerState.h"
#include "Components/MagicRopeMediator.h"
#include "Components/GrabZone.h"
#include "Components/TutorialNotificationComponent.h"
#include "Components/InteractableComponent.h"
#include "UI/HUDBase.h"
#include "ChromaCharacter.generated.h"	

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVibrationChangeDelegate, TEnumAsByte<EDynamicForceFeedbackAction::Type>, ActionVibration, float, Intensity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerControllerAssignedDelegate, AChromaPlayerController*, ChromaPlayerController);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerReadyDelegate, FLinearColor, ReadyPlayerColor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPlayerDestroyed);


UCLASS(Blueprintable)
class AChromaCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AChromaCharacter();
	UPROPERTY(Replicated)
	int32 DefaultRadius = 450;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UGrabZone* grabZone_component;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UInteractableComponent* InteractableComponent;

	UPROPERTY(BlueprintAssignable, Category = "Network")
		FPlayerDestroyed OnPlayerDestroyed;

	UPROPERTY(BlueprintAssignable, Category = "Feedback")
		FOnVibrationChangeDelegate OnVibrationChange;

	UPROPERTY(EditAnywhere)
		TSubclassOf<AActor> ActorToSpawn;

	UPROPERTY(BlueprintAssignable, Category = "Network")
		FPlayerControllerAssignedDelegate OnPlayerControllerAssigned;

	UPROPERTY(BlueprintAssignable, Category = "Player")
		FPlayerReadyDelegate OnPlayerReady;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network")
		bool bIsLocalPlayerCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<AMagicalGrabZone> MagicalGrabZoneRef; //<- la puerta

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UMagicRopeMediator* MagicRopeMediator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UTutorialNotificationComponent* NotifComp;

	UFUNCTION(BlueprintCallable)
		void BeginPlay();

	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION()
		void DestroyHandler(AActor* DestroyedActor);

	virtual void EndPlay(EEndPlayReason::Type) override;

	void SetManaTimer();

	UFUNCTION(BlueprintCallable)
		void OnGrabPressed();
	UFUNCTION(BlueprintCallable)
		void OnGrabReleased();
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
		bool bIsGrabbing;
	UFUNCTION(Server, Reliable)
		void Server_SetbIsGrabbingRPC(bool NewbIsGrabbing);
	void Server_SetbIsGrabbingRPC_Implementation(bool NewbIsGrabbing);

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly)
		class UWidgetComponent* widgetComponent;

	UPROPERTY(ReplicatedUsing = OnRep_IsReady, VisibleAnywhere, BlueprintReadWrite)
		bool IsReady;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerColor, VisibleAnywhere, BlueprintReadWrite)
		FLinearColor playerColor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Color")
		UMaterialInstanceDynamic* playerColorMaterial;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerMana, EditAnywhere, BlueprintReadWrite)
		float mana;

	float minimumMana;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> PingActor;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> PingActorFlag;
	
	bool isFlag;

	void SpawnPing(FVector Loc, FLinearColor PingColor, bool b );


	UFUNCTION(BlueprintCallable)
		void TransferMana(float _mana);

	UPROPERTY(EditAnywhere)
		float ManaDrainPS;

	const float MaxMana = 100;
	void SetCurrentMana(float _mana);
	UFUNCTION()
		void OnRep_PlayerMana();

	UFUNCTION(Server, Reliable)
		void Server_ChangeManaRPC(float _mana);
	void Server_ChangeManaRPC_Implementation(float _mana);

	UFUNCTION()
		void OnRep_IsReady();
	UFUNCTION(BlueprintCallable)
		void Set_IsReady(bool newReady);
	UFUNCTION(Server, Reliable)
		void Server_SetIsReadyRPC(bool newReady);
	void Server_SetIsReadyRPC_Implementation(bool newReady);

	UFUNCTION()
		void OnRep_PlayerColor();
	UFUNCTION(BlueprintCallable)
		void Set_PlayerColor(FLinearColor newColor);

	UFUNCTION(Server, Reliable)
		void Server_ChangeColor(FLinearColor newColor);
	void Server_ChangeColor_Implementation(FLinearColor newColor);

	void ShowPing();

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	// Called when controller is assigned
	virtual void Restart() override;


	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	UFUNCTION(BlueprintCallable)
		void ControllerVibrationHandler(TEnumAsByte<EDynamicForceFeedbackAction::Type> Action, float Intensity);

	UFUNCTION()
	void Ping(AChromaCharacter* _chromaCharacterSource, FLinearColor _color);


	UFUNCTION(Server, Reliable)
	void Server_SendPingRPC(AChromaCharacter* _chromaCharacterSource, AChromaCharacter* _chromaCharacterTarget, FLinearColor _color);
	void Server_SendPingRPC_Implementation(AChromaCharacter* _chromaCharacterSource, AChromaCharacter* _chromaCharacterTarget, FLinearColor _color);


	UFUNCTION(NetMulticast, Reliable)
	void Multi_ReceivePingRPC(AChromaCharacter* _chromaCharacterSource, AChromaCharacter* _chromaCharacterTarget, FLinearColor _color);
	void Multi_ReceivePingRPC_Implementation(AChromaCharacter* _chromaCharacterSource, AChromaCharacter* _chromaCharacterTarget, FLinearColor _color);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FLinearColor, AActor*> PingMap;

	UFUNCTION()
	void UpdatePingMap(AActor* _actor, FLinearColor _color);

	AHUDBase* GetHUD();

	UFUNCTION(BlueprintCallable)
	void ShowLoadingScreen();
	UFUNCTION(BlueprintCallable)
	void RemoveLoadingScreen();

	/** Returns CursorToWorld subobject **/
	FORCEINLINE class UDecalComponent* GetPingFeedback() { return PingFeedback; }

	FTimerHandle CountdownTimerHandle;

	UPROPERTY(EditAnywhere)
	int32 CountdownTime;
	
	void AdvanceTimer();

	UFUNCTION(BlueprintNativeEvent)
		void CountdownHasFinished();
	virtual void CountdownHasFinished_Implementation();

private:
	/** A decal that projects to the player location. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PingFeedback", meta = (AllowPrivateAccess = "true"))
	class UDecalComponent* PingFeedback;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PingFeedback", meta = (AllowPrivateAccess = "true"))
		UMaterialInstanceDynamic* PingFeedbackColorMaterial;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PingFeedback", meta = (AllowPrivateAccess = "true"))
		float CurrentPingFeedbackRadius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PingFeedback", meta = (AllowPrivateAccess = "true"))
		float PingFeedbackRadiusLerpSpeed;

	AMagicalGrabZone* MagicalGrabZone;

	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini Map", meta = (AllowPrivateAccess = "true"))
	//class USpringArmComponent* MiniMapBoom;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini Map", meta = (AllowPrivateAccess = "true"))
	//class USceneCaptureComponent2D* CaptureComponent;

};



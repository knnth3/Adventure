// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <deque>
#include "Basics.h"
#include "Grid/GridEntity.h"
#include "MapPawnStatSheet.h"
#include "./Components/HoverArrowComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MapPawn.generated.h"

USTRUCT(BlueprintType)
struct FCameraSettings
{
	GENERATED_BODY()

	//Units in Degrees
	UPROPERTY(BlueprintReadWrite)
	float AngularVelocity;

	//Units in m/s
	UPROPERTY(BlueprintReadWrite)
	float ZoomSpeed;

	UPROPERTY(BlueprintReadWrite)
	float MaxUpRotation;

	UPROPERTY(BlueprintReadWrite)
	float MaxDownRotation;

	UPROPERTY(BlueprintReadWrite)
	float MaxInZoom;

	UPROPERTY(BlueprintReadWrite)
	float MaxOutZoom;

	UPROPERTY(BlueprintReadWrite)
	float FinalCameraZoom;

};

UCLASS()
class ADVENTURE_API AMapPawn : public APawn, public IGridEntity
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMapPawn();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;
	virtual int GetClassIndex_Implementation() const override;
	virtual int GetObjectID_Implementation() const override;

	void ServerOnly_SetClassIndex(const int Index);

	// Owner Properties
	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	void OnCelebrationAnimEnd();

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	void OnAttackAnimEnd();

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	void ServerOnly_SetOwnerID(const int ID);

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	int GetPawnID()const;

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	int GetOwnerID()const;

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	FMapPawnStatSheet GetStatSheet()const;

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	bool IsMoving()const;

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	bool IsCelebrating()const;

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	bool IsAttacking()const;

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	bool IsDead()const;

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	void Celebrate(int AnimationIndex);

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	void Attack(int AttackIndex, const FVector& TargetLocation);

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	void KillPawn();

	// Camera Controlls
	const FVector GetCameraLocation() const;
	void RotateCameraPitch(const float& AxisValue, const float& DeltaTime);
	void RotateCameraYaw(const float& AxisValue, const float& DeltaTime);
	void ZoomCamera(const float& AxisValue, const float& DeltaTime);

	// Server Only Functions
	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	void ServerOnly_SetDestination(const FGridCoordinate& Destination);

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	void ServerOnly_SetTargetLocation(const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	void ServerOnly_SetStatusEffect(int EffectID);

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	void ServerOnly_SetEquipedWeaponType(WEAPON_TYPE type);

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	FVector ServerOnly_GetDesiredForwardVector() const;

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	void SetFocusToPawn(float TransitionTime);

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Skeletal Mesh")
	bool bIsFrozen;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Skeletal Mesh")
	UHoverArrowComponent* ArrowComponent;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Skeletal Mesh")
	class USpringArmComponent* CameraBoom;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Skeletal Mesh")
	class UCameraComponent* FollowCamera;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Skeletal Mesh")
	class USceneComponent* Scene;

	UFUNCTION(BlueprintImplementableEvent, Category = "Status Changed")
	void OnStatusChanged(int StatusID);

	UFUNCTION(BlueprintImplementableEvent, Category = "Status Changed")
	void OnAttackInitiated(int AttackIndex, const FVector& TargetLocation);

	UFUNCTION(BlueprintImplementableEvent, Category = "Status Changed")
	void OnPawnKilled();

private:

	bool bMovePawn;
	bool bRotatePawn;
	bool bHasTarget;
	bool bAttacking;
	bool bPlayCelebrationAnim;
	bool bIsDead;
	int m_ClassIndex;
	int m_CelebrationAnimIndex;
	int m_AttackAnimIndex;

	FVector m_ForwardVector;
	FVector m_Destination;
	FVector m_TargetedLocation;
	FCameraSettings m_CameraSettings;
	std::deque<FGridCoordinate> m_MoveQueue;

	// Server Property/Functions
	UPROPERTY(Replicated)
	int m_PawnID;

	UPROPERTY(Replicated)
	int m_OwnerID;

	UPROPERTY(Replicated)
	FMapPawnStatSheet m_StatSheet;

	void RotatePawn(float DeltaTime);
	void MovePawn(float DeltaTime);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetActiveDestination(const FVector& Location);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ApplyNewStatus(int StatusID);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Celebrate(int AnimationIndex);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Attack(int AttackIndex, const FVector& TargetLocation);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_KillPawn();

	static int GetNewID();
};

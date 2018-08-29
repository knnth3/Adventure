// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "Basics.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ConnectedPlayer.generated.h"

UENUM()
enum class TURN_BASED_STATE : uint8
{
	FREE_ROAM,
	STANDBY,
	ACTIVE
};

UENUM()
enum class CONNECTED_PLAYER_TYPE : uint8
{
	NONE,
	HOST,
	CLIENT
};

UENUM()
enum class CONNECTED_PLAYER_CAMERA : uint8
{
	NONE,
	CHARACTER,
	OVERVIEW
};

UCLASS()
class ADVENTURE_API AConnectedPlayer : public APawn
{
	GENERATED_BODY()

public:
	AConnectedPlayer();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Accessor Methods
	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	int GetPlayerID()const;

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	bool GetPawnLocation(FVector& Location) const;

	// Client Public Functions
	UFUNCTION(Client, reliable)
	void ClientChangeState(const TURN_BASED_STATE CurrentState);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void ServerScaleHead(const FVector& Size);

protected:
	virtual void BeginPlay() override;

	// Client State Change Functions
	UFUNCTION(BlueprintImplementableEvent, Category = "Connected Player")
	void OnPlayerStatusChanged(const TURN_BASED_STATE CurrentState);

	UFUNCTION(BlueprintImplementableEvent, Category = "Connected Player")
	void OnBeginCombat();

	UFUNCTION(BlueprintImplementableEvent, Category = "Connected Player")
	void OnEndCombat();

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	void MovePlayer(const FVector& Location, const int PawnID = 0);

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	void Attack(AMapPawnAttack* Attack, const FVector& EndLocation);

	// Camera Functions
	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	void SwapCameraView(const float& time);

	UFUNCTION(BlueprintCallable, Category = "MapPawn Camera")
	void RotatePawnCameraUpDown(const float& AxisValue, const float& DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "MapPawn Camera")
	void RotatePawnCameraLeftRight(const float& AxisValue, const float& DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "MapPawn Camera")
	void ZoomPawnCameraInOut(const float& AxisValue, const float& DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "MapPawn Camera")
	CONNECTED_PLAYER_CAMERA GetCameraType()const;

	// Player Components
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	class USceneComponent* Scene;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FollowCamera;

private:

	CONNECTED_PLAYER_CAMERA CameraType;
	class AMapPawn* SelectedMapPawn;
	std::vector<int> OwningPawns;

	UPROPERTY(ReplicatedUsing = OnSpectateReplicated)
	int SpectatingPawnID;

	UPROPERTY(Replicated)
	CONNECTED_PLAYER_TYPE PlayerType;

	// Server Private Functions
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerBeginTurnBasedMechanics();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEndTurnBasedMechanics();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerMovePlayer(const FVector& Location, const int PawnID);

	UFUNCTION()
	void OnSpectateReplicated();

};

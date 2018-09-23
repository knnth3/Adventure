// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "Basics.h"
#include "MapPawnStatSheet.h"
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

	void SetPlayerState(const TURN_BASED_STATE currentState);
	void AddNewCharacter(const int PawnID, bool IgnoreCameraLogic = false);
	void AdjustCameraToMap(const FGridCoordinate GridDimensions);
	void SetPlayerID(int newID);

	// Accessor Methods
	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	int GetPlayerID()const;

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	bool GetPawnLocation(FVector& Location) const;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RegisterPlayer();

protected:

	UPROPERTY(Replicated, BlueprintReadOnly)
	TURN_BASED_STATE CurrentState;

	// Player Components
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	class USceneComponent* Scene;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FollowCamera;

	// Client State Change Functions
	UFUNCTION(BlueprintImplementableEvent, Category = "Connected Player")
	void OnPlayerStatusChanged(const TURN_BASED_STATE state);

	// Camera Functions
	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	void SwapCameraView();

	UFUNCTION(BlueprintCallable, Category = "MapPawn Camera")
	void RotatePawnCameraUpDown(const float& AxisValue, const float& DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "MapPawn Camera")
	void RotatePawnCameraLeftRight(const float& AxisValue, const float& DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "MapPawn Camera")
	void ZoomPawnCameraInOut(const float& AxisValue, const float& DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "MapPawn Camera")
	CONNECTED_PLAYER_CAMERA GetCameraType()const;

	// Accessor Functions

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	int GetNumOwningPawns() const;

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	bool GetSelectedActorLocation(FVector& Location) const;

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	bool GetSelectedActorStats(FMapPawnStatSheet& Stats) const;

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	int GetSpectatingPawnID() const;
	
	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	void MovePlayer(const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	void SetPawnTargetLocation(const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	void ClearPawnTargetLocation();

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	void Attack(AMapPawnAttack* Attack, const FVector& EndLocation);

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	void SetSpectatingPawn(const int Index);

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	void BeginCombat(const TArray<int>& PlayerOrder);

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	void EndCombat();

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	void AddCharacter(int PlayerID, int PawnTypeIndex, bool OverrideSpawner , FVector NewLocation);

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:

	bool bRegistered;
	CONNECTED_PLAYER_CAMERA CameraType;
	class AMapPawn* SelectedPawn;
	float CameraTransitionAcceleration;

	void SetCameraToOverview();
	void SetCameraToCharacter();

	UPROPERTY(Replicated)
	int UniqueID;

	UPROPERTY(Replicated)
	int SpectatingPawnID;

	UPROPERTY(Replicated)
	CONNECTED_PLAYER_TYPE PlayerType;

	UPROPERTY(Replicated)
	TArray<int> OwningPawns;

	// Server Private Functions
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_BeginTurnBasedMechanics(const TArray<int>& Order);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_EndTurnBasedMechanics();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetSpectatingPawn(const int PawnIndex, bool setFocus = false);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_MovePlayer(const FVector& Location, const int PawnID);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Attack(AMapPawnAttack* Attack, const FVector & EndLocation);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ScaleHead(const FVector& Size);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_AddCharacter(int PlayerID, int PawnTypeIndex, bool OverrideSpawner, FVector NewLocation);

	//Client Private Functions
	UFUNCTION(Client, Reliable)
	void Client_SetFocusToSelectedPawn(bool setFocus = false);

	UFUNCTION(Client, Reliable)
	void Client_NotifyStateChange();

};

// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "Basics.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ConnectedPlayer.generated.h"

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
	// Sets default values for this pawn's properties
	AConnectedPlayer();

	UFUNCTION(Client, reliable)
	void UpdateStatus(const FString& Status);

	UFUNCTION(Client, reliable)
	void BeginCombat();

	UFUNCTION(Client, reliable)
	void EndCombat();

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	int PlayerID()const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Connected Player")
	void Server_BeginTurnBasedMechanics();

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Connected Player")
	void Server_EndTurnBasedMechanics();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Connected Player")
	void OnPlayerStatusChanged(const FString& Status);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Connected Player")
	void OnBeginCombat();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Connected Player")
	void OnEndCombat();

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	bool IsPlayersTurn()const;

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	void MovePlayer(const FVector& Location, const int PawnID = 0);

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	void SetPawnBodyArmor(const int BodyIndex, const int PawnID = 0);

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	void SetPawnHead(const int HeadIndex, const bool bBoy, const int PawnID = 0);

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

	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
	void Server_EndTurn();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	class USceneComponent* Scene;

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FollowCamera;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	void SpectateMapPawn(const int ID = -1);

private:

	class AMapPawn* GetMapPawn();

	UFUNCTION()
	void OnNewSpectateFocus();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_MovePlayer(const int PlayerID, const FVector& Location, const int PawnID);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetPawnBodyArmor(const int PlayerID, const int PawnID, const int BodyIndex);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetPawnHead(const int PlayerID, const int PawnID, const int HeadIndex, const bool bBoy);

	UPROPERTY(Replicated)
	CONNECTED_PLAYER_TYPE PlayerType;

	UPROPERTY(ReplicatedUsing = OnNewSpectateFocus)
	int SpectateMapPawnID;

	UPROPERTY(Replicated)
	bool bIsActive;

	CONNECTED_PLAYER_CAMERA CameraType;

	bool bSpectateNewPawn;
	class AWorldGrid* WorldGrid;
	class AMapPawn* SelectedMapPawn;
};

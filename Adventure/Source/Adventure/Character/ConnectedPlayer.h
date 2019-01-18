// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "Basics.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ConnectedPlayer.generated.h"


UENUM(BlueprintType)
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
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	bool GetPawnLocation(FVector& Location) const;

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	class AMapPawn* GetSelectedPawn() const;

	UFUNCTION(BlueprintCallable, Category = "MapPawn Camera")
	CONNECTED_PLAYER_CAMERA GetCameraType()const;

protected:

	// Called at actor start
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Player Components
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	class USceneComponent* Scene;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FollowCamera;

	// Camera Functions
	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	void SwapCameraView();

	UFUNCTION(BlueprintCallable, Category = "MapPawn Camera")
	void RotatePawnCameraUpDown(const float& AxisValue, const float& DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "MapPawn Camera")
	void RotatePawnCameraLeftRight(const float& AxisValue, const float& DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "MapPawn Camera")
	void ZoomPawnCameraInOut(const float& AxisValue, const float& DeltaTime);

	// Accessor Functions

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	bool GetSelectedActorLocation(FVector& Location) const;

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	class UStatisticsComponent* GetSelectedActorStats()const;

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	int GetSpectatingPawnID() const;
	
	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	void MovePlayer(const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	void SetPawnTargetLocation(const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	void ClearPawnTargetLocation();

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	void SetSpectatingPawn(const int PawnID);

	UFUNCTION(BlueprintImplementableEvent, Category = "Connected Player")
	void OnCameraTypeChanged(CONNECTED_PLAYER_CAMERA Type);

private:

	float m_Time;
	bool m_MapDNE;
	bool m_bMapFirstBuild;
	class AMapPawn* m_SelectedPawn;
	class AWorldGrid* m_WorldGrid;
	float m_CameraTransitionAcceleration;
	CONNECTED_PLAYER_CAMERA m_CameraType;

	void SetCameraToOverview();
	void SetCameraToCharacter();

	UPROPERTY(Replicated)
	int SpectatingPawnID;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_MovePlayer(const int PawnID, const FVector& Location, const FVector& Destination);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetPawnTargetLocation(const int PawnID, const FVector& Location);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ClearPawnTargetLocation();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_BeginMapConstruction();

};

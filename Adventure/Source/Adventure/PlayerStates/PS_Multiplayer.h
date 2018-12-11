// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "Basics.h"
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PS_Multiplayer.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class TURN_BASED_STATE : uint8
{
	FREE_ROAM,
	STANDBY,
	ACTIVE
};

UCLASS()
class ADVENTURE_API APS_Multiplayer : public APlayerState
{
	GENERATED_BODY()
	
public:
	APS_Multiplayer();
	void ServerOnly_SetGameID(const int ID);

	UFUNCTION(BlueprintCallable, Category = "Player State")
	int GetGameID() const;

	UFUNCTION(BlueprintCallable, Category = "Player State")
	void ServerOnly_SetPlayerState(const TURN_BASED_STATE state);

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn-Based Settings")
	TArray<int> InitiativePlayerOrder;

	UFUNCTION(BlueprintCallable, Category = "Turn-Based Settings")
	void OverrideCurrentPlayersTurn(const int ID);

	UFUNCTION(BlueprintCallable, Category = "Turn-Based Settings")
	TURN_BASED_STATE GetCurrentState() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Player State")
	void OnStateChanged();

private:

	UPROPERTY(Replicated)
	int m_GameID;

	UPROPERTY(ReplicatedUsing = OnStateChanged)
	TURN_BASED_STATE m_CurrentState;

	UPROPERTY(Replicated)
	int m_CurrentPlayerActive;
};

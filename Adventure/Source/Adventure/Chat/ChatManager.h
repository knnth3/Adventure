// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include <vector>
#include "Basics.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChatManager.generated.h"

UCLASS()
class ADVENTURE_API AChatManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AChatManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void AddText(FString text);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	bool HasNewEntries();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_AddText(const FString& text);

private:

	UPROPERTY(Replicated)
	TArray<FString> m_Log;

	UPROPERTY(Replicated)
	bool m_bHasNewItems;
	
};

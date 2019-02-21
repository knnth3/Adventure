// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include <memory>
#include <map>
#include "Basics.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DownloadManager.generated.h"

/**
 * 
 */

USTRUCT()
struct ADVENTURE_API FDownloadChunk
{
	GENERATED_BODY()
public:

	UPROPERTY()
	int PacketID = 0;

	UPROPERTY()
	TArray<uint8> Data;
};

UCLASS()
class ADVENTURE_API ADownloadManager : public AActor
{
	GENERATED_BODY()
	
public:
	ADownloadManager();

	UFUNCTION(BlueprintCallable, Category="Data Settings")
	void ServerOnly_SetData(const TArray<uint8>& data);

	UFUNCTION(BlueprintCallable, Category = "Data Settings")
	void Subscribe(const FString addr, int port);

	UFUNCTION(BlueprintCallable, Category = "Data Settings")
	TArray<uint8> GetUnpackedData() const;

private:

	UFUNCTION()
	void OnDataReceived();

	UFUNCTION()
	void OnDownloadRequested();

	UPROPERTY(ReplicatedUsing = OnDownloadRequested)
	int m_dataSize;

	TArray<FDownloadChunk> m_data;
};
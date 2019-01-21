// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "NetConnectionVerifyNode.h"
#include "../Adventure.h"

// Sets default values
ANetConnectionVerifyNode::ANetConnectionVerifyNode()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	m_ElapsedTime = 0.0f;
	m_PingRecieved = false;
}

// Called when the game starts or when spawned
void ANetConnectionVerifyNode::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogNotice, Warning, TEXT("<NetConnectionVerifyNode>: Spawn Sucess!"));
}

// Called every frame
void ANetConnectionVerifyNode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		m_ElapsedTime += DeltaTime;
		if (m_ElapsedTime > 1)
		{
			m_ElapsedTime = 0.0f;

			if (!m_PingRecieved)
			{
				Multicast_PingConnectedPlayers();
			}
		}

	}
}

void ANetConnectionVerifyNode::Multicast_PingConnectedPlayers_Implementation()
{
	UE_LOG(LogNotice, Warning, TEXT("<NetConnectionVerifyNode>: Ping from server"));
}


// Copyright 2019 Eric Marquez
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
// http ://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "NetConnectionVerifyNode.h"
#include "../Adventure.h"

// Sets default values
ANetConnectionVerifyNode::ANetConnectionVerifyNode()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	m_ElapsedTime = 0.0f;
	m_PingRecieved = false;
	bNetLoadOnClient = true;
	bReplicates = true;
	bAlwaysRelevant = true;
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


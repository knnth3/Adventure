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

#include "Attack.h"
#include "Adventure.h"


// Sets default values
AAttack::AAttack()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AAttack::Initialize(FAttackReuqest Request)
{
	Damage = Request.Damage;
	Radius = Request.Radius;
	Destination = Request.Location;
	OnInitialized();
}

// Called when the game starts or when spawned
void AAttack::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAttack::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


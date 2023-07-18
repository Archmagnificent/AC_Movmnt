// Copyright Epic Games, Inc. All Rights Reserved.

#include "AC_MovmntCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"


//////////////////////////////////////////////////////////////////////////
// AAC_MovmntCharacter

AAC_MovmntCharacter::AAC_MovmntCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Disable collision capsule physics by default
	GetCapsuleComponent()->SetSimulatePhysics(false);
	
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	//Setup Player Movement Variables
	
	WalkSpeed = 600.f;
	BoostForce = 1200.f;
	BoostForceUp = 1200.0f;
	QuickBoostForce = 1500.0f;

	BoostUpTimeWindow = 0.5f; 

	DirMovementReceivesInput = false;
	IsBoosting = false;
	CanBoostUp = false;
	IsHovering = false;

	CharacterMass = GetCharacterMovement()->Mass;

	//Setup Player Energy Variables
	CurrentEnergy = 10000.0f;
	MaxEnergy = 10000.0f;
	EnergyRechargeRate =  100.0f;

	EnergyDrainRate = 0.0f;

	BoostDrainRate = 150.0f;
	QuickBoostDrain = 500.0f;
	HoverDrainRate = 120.0f;

	CanEnergyRecharge = true;
	CanDrainEnergy = false;
	
	
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AAC_MovmntCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	bUseControllerRotationYaw = true;

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AAC_MovmntCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//Check if Energy can be drained 
	if(EnergyDrainRate > 0.0f)
	{
		CanDrainEnergy = true;
	} else
	{
		CanDrainEnergy = false;
	}
	
	//Sets the energy value to 0 in case it reaches negative value
	if(CurrentEnergy < 0.0f)
	{
		CurrentEnergy = 0.0f;
	}

	//Handles Energy Charging/Draining functionality
	if(CanDrainEnergy)
	{
		CurrentEnergy = FMath::FInterpConstantTo(CurrentEnergy, 0.0f, DeltaSeconds, EnergyDrainRate);

		if(CurrentEnergy <= 0.0f)
		{
			AllEnergyDepleted();
		}
	}
	if(CurrentEnergy < MaxEnergy)
		{
			if(CanEnergyRecharge)
			{
				CurrentEnergy = FMath::FInterpConstantTo(CurrentEnergy, MaxEnergy, DeltaSeconds, EnergyRechargeRate);
			}
		}
	}


//////////////////////////////////////////////////////////////////////////
// Input

void AAC_MovmntCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAC_MovmntCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AAC_MovmntCharacter::StopMoving);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAC_MovmntCharacter::Look);

		//Boosting
		EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Triggered, this, &AAC_MovmntCharacter::Boost);
		EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Completed, this, &AAC_MovmntCharacter::StopBoosting);
		EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Started, this, &AAC_MovmntCharacter::OnBoostStart);

		//Quick Boosting
		EnhancedInputComponent->BindAction(QuickBoostAction, ETriggerEvent::Started, this, &AAC_MovmntCharacter::QuickBoost);

		//Hover Mode
		EnhancedInputComponent->BindAction(HoverAction, ETriggerEvent::Triggered, this, &AAC_MovmntCharacter::Hover);
		EnhancedInputComponent->BindAction(HoverAction, ETriggerEvent::Completed, this, &AAC_MovmntCharacter::StopHover);
	}

}

void AAC_MovmntCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	const FVector2D MovementVector = Value.Get<FVector2D>();
	
	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward and right vector for walking
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		
        // get forward and right vectors to apply force when boosting
		const FVector ForwardBoostDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X) * MovementVector.Y;
		const FVector RightBoostDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y) * MovementVector.X;
		

		// Store forward and right vectors to use in quick boost
		QuickBoostForwardDirection = ForwardBoostDirection;
		QuickBoostRightDirection = RightBoostDirection;
		
		// add movement

		//Confirm that function receives input
		DirMovementReceivesInput = true;
		
		if(!IsBoosting)
		{
			AddMovementInput(ForwardDirection, MovementVector.Y);
			AddMovementInput(RightDirection, MovementVector.X);
		} else
		{
			GetCharacterMovement()->AddForce(ForwardBoostDirection * BoostForceUp * CharacterMass);
			GetCharacterMovement()->AddForce(RightBoostDirection * BoostForceUp * CharacterMass);
		}
	}
}

void AAC_MovmntCharacter::StopMoving()
{
	//Confirm that movement function no longer receives input
	DirMovementReceivesInput = false;
}



void AAC_MovmntCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AAC_MovmntCharacter::Boost()
{
	if (Controller != nullptr)
	{
		if(CurrentEnergy > BoostDrainRate)
		{
			if(!DirMovementReceivesInput)
			{
				IsBoosting = true;
				//Gets the "Up" Direction
				const FVector UpDirection = GetActorUpVector();
				//Applies constant force in the up direction
				GetCharacterMovement()->AddForce(UpDirection * BoostForceUp * CharacterMass);
			} else
			{
				if(CanBoostUp || GetCharacterMovement()->IsFalling())
				{
					IsBoosting = true;
					//Gets the "Up" Direction
					const FVector UpDirection = GetActorUpVector();
					//Applies constant force in the up direction
					GetCharacterMovement()->AddForce(UpDirection * BoostForceUp * CharacterMass);
				}
			}
		}
	}
}

void AAC_MovmntCharacter::StopBoosting()
{
	if (Controller != nullptr)
	{
		IsBoosting = false;
		CanBoostUp = true;
		EnergyDrainRate = EnergyDrainRate - BoostDrainRate;

		GetWorld()->GetTimerManager().SetTimer(BoostUpFromGroundTimerHandle, this, &AAC_MovmntCharacter::DisableBoostUp, BoostUpTimeWindow , false);
	}
}

void AAC_MovmntCharacter::OnBoostStart()
{
	EnergyDrainRate = EnergyDrainRate + BoostDrainRate;
	GetWorld()->GetTimerManager().PauseTimer(BoostUpFromGroundTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(BoostUpFromGroundTimerHandle);
}

void AAC_MovmntCharacter::DisableBoostUp()
{
	CanBoostUp = false;
}



void AAC_MovmntCharacter::QuickBoost()
{
	if (Controller != nullptr)
	{
		if(CurrentEnergy > QuickBoostDrain)
		{
			if(DirMovementReceivesInput)
			{
				GetCharacterMovement()->AddImpulse(QuickBoostForwardDirection * QuickBoostForce * CharacterMass);
				GetCharacterMovement()->AddImpulse(QuickBoostRightDirection * QuickBoostForce * CharacterMass);
				CurrentEnergy = CurrentEnergy - QuickBoostDrain;
			} else
			{
				const FVector ForwardDirection = GetActorForwardVector();
				GetCharacterMovement()->AddImpulse(ForwardDirection * QuickBoostForce * CharacterMass);
				CurrentEnergy = CurrentEnergy - QuickBoostDrain;
			}
		}
	}
}

void AAC_MovmntCharacter::Hover()
{
	if(Controller != nullptr)
	{
		if(CurrentEnergy > HoverDrainRate)
		{
			if(!IsHovering && GetCharacterMovement()->IsFalling())
			{
				OnHover();
				IsHovering = true;
				GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
				EnergyDrainRate = EnergyDrainRate + HoverDrainRate;
			}
			else
			{
				StopHover();
				EnergyDrainRate = EnergyDrainRate -HoverDrainRate;
			}
		}
	}
}

void AAC_MovmntCharacter::StopHover()
{
	if(Controller != nullptr)
	{
		OnStopHover();
		IsHovering = false;
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	}
} 

void AAC_MovmntCharacter::AllEnergyDepleted()
{
	StopBoosting();
	StopHover();
}

void AAC_MovmntCharacter::EnableEnergycharging()
{
	
}









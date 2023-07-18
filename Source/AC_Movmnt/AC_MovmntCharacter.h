// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "AC_MovmntCharacter.generated.h"

class UInputMappingContext;

UCLASS(config=Game)
class AAC_MovmntCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	/** Boost Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* BoostAction;

	/** QuickBoost Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* QuickBoostAction;

	/** Hover Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* HoverAction;
	

	
public:
	AAC_MovmntCharacter();
	

protected:

	/**----------- Movement Functions___________________ **/

	/** Called for movement input */
	void Move(const FInputActionValue& Value);
	void StopMoving();

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for Boosting Input **/
	void Boost();
	void StopBoosting();
	/* Called at the start of boosting input once */
	void OnBoostStart();
	void DisableBoostUp();
	

	/** Called for QuickBoosting Input **/
	void QuickBoost();

	/* Called for Hover Input */
	void Hover();
	void StopHover();

	/**----------- Blueprint Implementable Functions___________________ **/

	UFUNCTION(BlueprintImplementableEvent)
	void OnHover();
	UFUNCTION(BlueprintImplementableEvent)
	void OnStopHover();
		
	/** Movement Variables **/
	
	//floats
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = MovementVariables)
	float WalkSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = MovementVariables)
	float BoostForce;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = MovementVariables)
	float BoostForceUp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = MovementVariables)
	float QuickBoostForce;

	float CharacterMass;
	//booleans
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = MovementVariables)
	bool DirMovementReceivesInput;
	bool IsBoosting;
	bool CanBoostUp;
	bool IsHovering;
	bool CanQuickBoost;

	//Timers
	FTimerHandle BoostUpFromGroundTimerHandle;
	float BoostUpTimeWindow;

	//Vectors
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector QuickBoostForwardDirection;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector QuickBoostRightDirection;
	
	/**----------- Energy Functions___________________ **/

	//Enables energy charging
	void EnableEnergycharging();
	//Gets Called onece the energy reaches 0
	void AllEnergyDepleted();
	
	/** Energy Variables **/

	//floats
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = EnergyVariables)
	float CurrentEnergy;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = EnergyVariables)
	float MaxEnergy;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = EnergyVariables)
	float EnergyRechargeRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = EnergyVariables)
	float EnergyDrainRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = EnergyVariables)
	float BoostDrainRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = EnergyVariables)
	float QuickBoostDrain;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = EnergyVariables)
	float HoverDrainRate;


	//booleans 
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = EnergyVariables)
	bool CanEnergyRecharge;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = EnergyVariables)
	bool CanDrainEnergy;
	
public:

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay() override;

	//Called every frame
	virtual void Tick(float DeltaSeconds) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};


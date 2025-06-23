#include "Game/BasePlayer.h"

#include "MyPlayer.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ABasePlayer::ABasePlayer()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	GetCharacterMovement()->bRunPhysicsWithNoController = true;

	PlayerInfo = new Protocol::PosInfo();
	DestInfo = new Protocol::PosInfo();
}

ABasePlayer::~ABasePlayer()
{
	delete PlayerInfo;
	delete DestInfo;
	PlayerInfo = nullptr;
	DestInfo = nullptr;
}

bool ABasePlayer::IsMyPlayer()
{
	return Cast<AMyPlayer>(this) != nullptr;
}

void ABasePlayer::BeginPlay()
{
	Super::BeginPlay();

	{
		FVector Location = GetActorLocation();
		DestInfo->set_x(Location.X);
		DestInfo->set_y(Location.Y);
		DestInfo->set_z(Location.Z);
		DestInfo->set_yaw(GetControlRotation().Yaw);

		SetMoveState(Protocol::MOVE_STATE_IDLE);
	}
}

void ABasePlayer::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const FVector Location = GetActorLocation();

	{
		PlayerInfo->set_x(Location.X);
		PlayerInfo->set_y(Location.Y);
		PlayerInfo->set_z(Location.Z);
		PlayerInfo->set_yaw(GetControlRotation().Yaw);
	}

	if (IsMyPlayer() == false)
	{
		/*const auto DestLocation = FVector(DestInfo->x(), DestInfo->y(), DestInfo->z());
		FVector MoveDir = DestLocation - Location;
		const float DistToDest = MoveDir.Length();
		MoveDir.Normalize();

		float MoveDist = (MoveDir * 600.f * DeltaSeconds).Length();
		MoveDist = FMath::Min(MoveDist, DistToDest);
		const FVector NextLocation = Location + MoveDir * MoveDist;

		SetActorLocation(NextLocation);*/

		const Protocol::MoveState State = PlayerInfo->state();
		if (State == Protocol::MOVE_STATE_RUN)
		{
			// TODO : Rotator 도 보정값을 줘서 천천히 돌아가도록
			SetActorRotation(FRotator(0, DestInfo->yaw(), 0));
			AddMovementInput(GetActorForwardVector());

			// TODO : 서버에서 보내는 위치값과 많이 틀어지면 여기서 위치 강제 보정
		}
		else
		{
			// TODO: Run 상태에서 여기로 진입했을때 남은 회전 & 이동 값이 있으면 여기서 해줘	
		}
	}
}

void ABasePlayer::SetPlayerInfo(const Protocol::PosInfo& Info)
{
	if (PlayerInfo->object_id() != 0)
	{
		assert(PlayerInfo->object_id() == Info.object_id());
	}

	PlayerInfo->CopyFrom(Info);

	const FVector Location(Info.x(), Info.y(), Info.z());
	SetActorLocation(Location);
}

void ABasePlayer::SetDestInfo(const Protocol::PosInfo& Info) const
{
	if (PlayerInfo->object_id() != 0)
	{
		assert(PlayerInfo->object_id() == Info.object_id());
	}

	DestInfo->CopyFrom(Info);

	// 상태 적용
	SetMoveState(Info.state());
}

void ABasePlayer::SetMoveState(const Protocol::MoveState& State) const
{
	if (PlayerInfo->state() == State)
		return;

	PlayerInfo->set_state(State);
}


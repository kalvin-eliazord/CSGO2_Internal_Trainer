#include "Aimbot.h"

bool Aimbot::IsTargetInFov(Vector3& pTargetAngle)
{
	Vector3 localPlayerAngle{ LocalPlayer::GetPawn().vAngEyeAngle };
	Vector3 deltaAngle{ localPlayerAngle - pTargetAngle };
	NormalizeYaw(deltaAngle.y);

	const float distFromCursor{ GetMagnitude(deltaAngle) };

	// Checking if target is in FOV
	if (distFromCursor < ConsoleMenu::fovValue)
		return true;

	return false;
}

bool Aimbot::IsSpotted(Entity pCurrEnt)
{
	Entity localPlayerEnt{ LocalPlayer::GetEntity()};
	std::bitset<64> lpSpottedId{ localPlayerEnt.GetPawnBase().bSpottedMask };
	std::bitset<64> currEntSpottedId{ pCurrEnt.GetPawnBase().bSpottedMask };

	// Not spotted by entity AND entity not spotted me
	if (!(lpSpottedId.test(pCurrEnt.GetIndex()) || currEntSpottedId.test(localPlayerEnt.GetIndex())))
		return false;

	return true;
}

Entity Aimbot::GetNearestTarget(std::vector<Entity> pTargetsEnt)
{
	float oldCoef{ FLT_MAX };
	Entity nearestTarget{};

	Pawn lpPawn{ LocalPlayer::GetPawn() };

	for (const auto& currCTarget : pTargetsEnt)
	{
		Entity currEnt(currCTarget);

		// Get angle distance
		const Vector3 currTargetAngle{ GetTargetAngle(currEnt.GetPawnBase().vLastCameraPos) };
		const Vector3 deltaAngle{ lpPawn.vAngEyeAngle - currTargetAngle };
		const float angleDistance{ GetMagnitude(deltaAngle) };

		// Get body position distance
		const Vector3 deltaPosition{ lpPawn.vLastCameraPos - currEnt.GetPawnBase().vLastCameraPos };
		const float bodyPosDist{ GetMagnitude(deltaPosition) };

		const float currDistCoef{ angleDistance * 0.8f + bodyPosDist * 0.2f };

		if (oldCoef > currDistCoef)
		{
			oldCoef = currDistCoef;
			nearestTarget = currEnt;
		}
	}

	return nearestTarget;
}

Entity Aimbot::GetEntTarget(const std::vector<Entity>& pTargets)
{
	Entity target{};

	if (pTargets.size() > 1)
		target = GetNearestTarget(pTargets);
	else
		target = pTargets[0];

	if (IsSpotted(target)) return target;

	return Entity();
}

void Aimbot::NormalizePitch(float& pPitch)
{
	pPitch = (pPitch < -89.0f) ? -89.0f : pPitch;

	pPitch = (pPitch > 89.f) ? 89.0f : pPitch;
}

void Aimbot::NormalizeYaw(float& pYaw)
{
	while (pYaw > 180.f) pYaw -= 360.f;

	while (pYaw < -180.f) pYaw += 360.f;
}

float Aimbot::GetMagnitude(const Vector3& pVec)
{
	return ::sqrtf((pVec.x * pVec.x) +
		(pVec.y * pVec.y) +
		(pVec.z * pVec.z));
}
Vector3 Aimbot::GetTargetAngle(Vector3 pTargetPos)
{
	Vector3 targetAngle{};
	Vector3 lpPos{ LocalPlayer::GetPawn().vLastCameraPos };

	const Vector3 deltaPos{ pTargetPos - lpPos };

	const float distPos{ GetMagnitude(deltaPos) };

	constexpr float radToDegree{ 57.295776f };
	targetAngle.x = -asinf(deltaPos.z / distPos) * radToDegree;
	targetAngle.y = atan2f(deltaPos.y, deltaPos.x) * radToDegree;

	NormalizePitch(targetAngle.x);

	return targetAngle;
}

bool Aimbot::Start(const std::vector<Entity>& pTargets)
{
	if (pTargets.empty()) return false;

	Entity cTarget{ GetEntTarget(pTargets) };
	if (!cTarget.IsEntInit()) return false;

	// Updating the target only when the feature is off
	if (!ConsoleMenu::bTargetLock)
		cTargetLocked = cTarget;

	// Target locking feature
	if (ConsoleMenu::bTargetLock)
		ShotLockedTarget();
	else
		ShotTarget(cTarget);

	return true;
}

bool Aimbot::ShotTarget(const Entity& pCTarget)
{
	Entity entTarget(pCTarget);
	Vector3 targetAngle{};

	// Bone target
	if (ConsoleMenu::bHeadPos)
		targetAngle = Aimbot::GetTargetAngle(entTarget.GetBonePos(Bone::head));
	else
		targetAngle = Aimbot::GetTargetAngle(entTarget.GetBonePos(Bone::pelvis));

	// FOV
	if (!Aimbot::IsTargetInFov(targetAngle))
		return false;

	// Smoothing
	if (ConsoleMenu::smoothValue)
		LocalPlayer::SetSmoothViewAngles(targetAngle, ConsoleMenu::smoothValue);
	else
		LocalPlayer::SetViewAngles(targetAngle);

	return true;
}

bool Aimbot::ShotLockedTarget()
{
	Entity entTargetLocked(Aimbot::cTargetLocked);
	Pawn entPawnLocked{ entTargetLocked.GetPawnBase() };
	Vector3 targetLockedAngle{};

	// Bone target
	if (ConsoleMenu::bHeadPos)
		targetLockedAngle = Aimbot::GetTargetAngle(entTargetLocked.GetBonePos(Bone::head));
	else
		targetLockedAngle = Aimbot::GetTargetAngle(entTargetLocked.GetBonePos(Bone::pelvis));

	// FOV
	if (!Aimbot::IsTargetInFov(targetLockedAngle))
		return false;

	// Smoothing
	if (ConsoleMenu::smoothValue)
		LocalPlayer::SetSmoothViewAngles(targetLockedAngle, ConsoleMenu::smoothValue);
	else
		LocalPlayer::SetViewAngles(targetLockedAngle);

	// Locking at target until he die
	if (entPawnLocked.iHealth < 1)
		cTargetLocked = Entity();

	return true;
}
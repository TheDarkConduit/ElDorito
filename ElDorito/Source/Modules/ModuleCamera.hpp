#pragma once

#include "ModuleBase.hpp"
#include "../Patch.hpp"
#include "../Blam/BlamTypes.hpp"
#include <unordered_map>

namespace Modules
{
	class ModuleCamera : public Utils::Singleton<ModuleCamera>, public ModuleBase
	{
	public:
		struct CameraPosistion
		{
		public:
			float hLookAngle;
			float vLookAngle;
			float xPos;
			float yPos;
			float zPos;
		};

		Command* VarCameraCrosshair;
		Command* VarCameraFov;
		Command* VarCameraHideHud;
		Command* VarCameraMode;
		Command* VarCameraSpeed;
		Command* VarCameraSave;
		Command* VarCameraLoad;
		Command* VarCameraPosition;
		Command* VarCameraShowCoordinates;

		// patches to stop camera mode from changing
		Patch Debug1CameraPatch;
		Patch Debug2CameraPatch;
		Patch ThirdPersonPatch;
		Patch FirstPersonPatch;
		Patch DeadPersonPatch;

		Patch StaticILookVectorPatch;
		Patch StaticKLookVectorPatch;

		Patch HideHudPatch;
		Patch CenteredCrosshairFirstPersonPatch;
		Patch CenteredCrosshairThirdPersonPatch;

		Patch ShowCoordinatesPatch;

		Hook CameraPermissionHook;
		Hook CameraPermissionHookAlt1;
		Hook CameraPermissionHookAlt2;
		Hook CameraPermissionHookAlt3;

		ModuleCamera();

		CameraPosistion GetPosition();

		void SetPosition(CameraPosistion position);

		void UpdatePosition();

		CameraPosistion lastPosition;
	};
}
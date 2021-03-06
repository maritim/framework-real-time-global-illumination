#include "FPSCameraController.h"

#include <glm/geometric.hpp>

#include "Systems/Input/Input.h"
#include "Systems/Time/Time.h"

#include "Managers/CameraManager.h"

void FPSCameraController::Start ()
{
	_pitch = _yaw = 0;

	// Camera::Main ()->SetPosition (glm::vec3 (170, 100, 170));
}

void FPSCameraController::Update ()
{
	Camera* camera = CameraManager::Instance ()->GetActive ();

	float cameraVelocity = 200.0f;
	glm::vec3 velocity = glm::vec3 (0.0f);

	glm::vec3 Forward = camera->GetForward ();
	Forward.y = 0;
	glm::vec3 Left = camera->GetLeft ();
	Forward = glm::normalize (Forward); Left = glm::normalize (Left);

	if (Input::GetKey (InputKey::W) || Input::GetKey (InputKey::UP) || Input::GetJoystickAxis (0).y < 0) {
		velocity += Forward * cameraVelocity * Time::GetDeltaTime();
	}

	if (Input::GetKey (InputKey::S) || Input::GetKey (InputKey::DOWN) || Input::GetJoystickAxis (0).y > 0) {
		velocity -= Forward * cameraVelocity * Time::GetDeltaTime();
	}

	if (Input::GetKey (InputKey::A) || Input::GetKey (InputKey::LEFT) || Input::GetJoystickAxis (0).x < 0) {
		velocity += Left * cameraVelocity * Time::GetDeltaTime();
	}

	if (Input::GetKey (InputKey::D) || Input::GetKey (InputKey::RIGHT) || Input::GetJoystickAxis (0).x > 0) {
		velocity -= Left * cameraVelocity * Time::GetDeltaTime();
	}

	glm::vec3 camPos = camera->GetPosition () + velocity;
	camera->SetPosition (camPos);

	static bool leftButtonDown = false;
	static int iLastMouseMoveX = 0;
	static int iLastMouseMoveY = 0;

	glm::ivec2 mousePosition = Input::GetMousePosition ();

	if (Input::GetMouseButtonUp (MOUSE_BUTTON_LEFT)) {
		leftButtonDown = false;
	}
	else if (Input::GetMouseButtonDown (MOUSE_BUTTON_LEFT)) {
		iLastMouseMoveX = mousePosition.x;
		iLastMouseMoveY = mousePosition.y;

		leftButtonDown = true;
	}

	if (leftButtonDown) {
		int iMoveDistanceX = mousePosition.x - iLastMouseMoveX;
		int iMoveDistanceY = mousePosition.y - iLastMouseMoveY;

		float mouseSensitivity = 0.005f;

		_pitch += iMoveDistanceX * mouseSensitivity;
		_yaw += iMoveDistanceY * mouseSensitivity;

		camera->SetRotation (glm::quat ());
		camera->Rotate (_pitch, glm::vec3 (0.0f, 1.0f, 0.0f));
		camera->Rotate (_yaw, glm::vec3(1.0f, 0.0f, 0.0f));

		iLastMouseMoveX = mousePosition.x;
		iLastMouseMoveY = mousePosition.y;
	}

	if (Input::GetJoystickButton (0) || Input::GetJoystickButton (1) || 
		Input::GetJoystickButton (2) || Input::GetJoystickButton (3)) {

		int iMoveDistanceX = (int)Input::GetJoystickButton (1) + -(int)Input::GetJoystickButton (2);
		int iMoveDistanceY = (int)Input::GetJoystickButton (0) + -(int)Input::GetJoystickButton (3);

		float cameraRotationVelocity = 2.0f;

		_pitch += Time::GetDeltaTime () * iMoveDistanceX * cameraRotationVelocity;
		_yaw += Time::GetDeltaTime () * iMoveDistanceY * cameraRotationVelocity;

		camera->SetRotation (glm::quat ());
		camera->Rotate (_pitch, glm::vec3 (0.0f, 1.0f, 0.0f));
		camera->Rotate (_yaw, glm::vec3(1.0f, 0.0f, 0.0f));
	}
}
#include "RigidBodyComponent.h"

#include "Systems/Physics/PhysicsManager.h"
#include "Systems/Physics/MotionState.h"

RigidBodyComponent::RigidBodyComponent () :
	_colliderComponent (nullptr),
	_motionState (nullptr),
	_rigidBody (nullptr)
{

}

RigidBodyComponent::~RigidBodyComponent ()
{
	delete _motionState;
	delete _rigidBody;
}

void RigidBodyComponent::Awake ()
{
	/*
	 * Check transform exists
	*/

	_motionState = new MotionState (_parent->GetTransform (), glm::vec3 (0.0f)); //_collider->GetOffset ());

	/*
	 * Initialize rigidbody
	*/

	btVector3 localInertia = btVector3 (0.0f, 0.0f, 0.0f);

	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI (
		_mass,
		_motionState,
		nullptr,
		localInertia
	);

	_rigidBody = new btRigidBody (rigidBodyCI);

	/*
	 * Disable debug data
	*/

	_rigidBody->setCollisionFlags (_rigidBody->getCollisionFlags ()
		| btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT);

	OnAttachedToScene ();
}

void RigidBodyComponent::Update ()
{
	btTransform transform;

	glm::vec3 offset = _colliderComponent == nullptr ? glm::vec3 (0.0f) :
		_parent->GetTransform ()->GetRotation () * _colliderComponent->GetOffset ();

	glm::vec3 position = _parent->GetTransform ()->GetPosition () + offset;
	glm::quat rotation = _parent->GetTransform ()->GetRotation ();

	transform.setOrigin (btVector3 (position.x, position.y, position.z));
	transform.setRotation (btQuaternion (rotation.x, rotation.y, rotation.z, rotation.w));

	_rigidBody->setWorldTransform (transform);

	/*
	 * Set collision shape scaling
	*/

	if (_colliderComponent != nullptr) {
		glm::vec3 scale = _parent->GetTransform ()->GetScale ();

		_colliderComponent->GetCollisionShape ()->setLocalScaling (btVector3 (scale.x, scale.y, scale.z));
	}

	/*
	 * Update collider
	*/

	if (_colliderComponent != nullptr) { // && _colliderComponent->IsDirty ()) {
		UpdateCollider ();
	}
}

void RigidBodyComponent::SetActive (bool isActive)
{
	int flags = _rigidBody->getFlags ();
	flags = isActive ? flags &~ ISLAND_SLEEPING : flags | ISLAND_SLEEPING;

	_rigidBody->setFlags (flags);
}

void RigidBodyComponent::OnAttachedToScene ()
{
	if (_rigidBody == nullptr) {
		return;
	}

	UpdateCollider ();

	/*
	 * Attach rigid body to physics system
	*/

	PhysicsManager::Instance ()->AttachRigidbody (_rigidBody);
}

void RigidBodyComponent::OnDetachedFromScene ()
{
	/*
	 * Detach rigid body from the physics system
	*/

	PhysicsManager::Instance ()->DetachRigidbody (_rigidBody);
}

float RigidBodyComponent::GetMass () const
{
	return _mass;
}

glm::vec3 RigidBodyComponent::GetVelocity () const
{
	btVector3 velocity = _rigidBody->getLinearVelocity ();

	return glm::vec3 (velocity.getX (), velocity.getY (), velocity.getZ ());
}

glm::vec3 RigidBodyComponent::GetAngularVelocity () const
{
	btVector3 velocity = _rigidBody->getAngularVelocity ();

	return glm::vec3 (velocity.getX (), velocity.getY (), velocity.getZ ());
}

void RigidBodyComponent::SetMass (float mass)
{
	_mass = mass;

	btVector3 localInertia = btVector3 (0.0f, 0.0f, 0.0f);

	if (_colliderComponent != nullptr) {
		_colliderComponent->GetCollisionShape ()->calculateLocalInertia (_mass, localInertia);
	}

	_rigidBody->setMassProps (mass, localInertia);
}

void RigidBodyComponent::SetVelocity (const glm::vec3& velocity)
{
	_rigidBody->setLinearVelocity (btVector3 (velocity.x, velocity.y, velocity.z));
}

void RigidBodyComponent::SetAngularVelocity (const glm::vec3& velocity)
{
	_rigidBody->setAngularVelocity (btVector3 (velocity.x, velocity.y, velocity.z));
}

void RigidBodyComponent::UpdateCollider ()
{
	if (_colliderComponent == nullptr) {
		return;
	}

	if (_colliderComponent->GetCollisionShape () == nullptr) {
		return;
	}

	/*
	 * Set collision shape scaling
	*/

	glm::vec3 scale = _parent->GetTransform ()->GetScale ();

	_colliderComponent->GetCollisionShape ()->setLocalScaling (btVector3 (scale.x, scale.y, scale.z));

	/*
	 * Compute local inertia based on collision shape
	 *
	 * Default local inertia is vec3 (0, 0, 0),
	 * which is fixed in respect to any axis rotation
	*/

	btVector3 localInertia = btVector3 (0.0f, 0.0f, 0.0f);

	_colliderComponent->GetCollisionShape ()->calculateLocalInertia (_mass, localInertia);

	/*
	 * Update rigidbody local inertia
	*/

	_rigidBody->setMassProps (_mass, localInertia);

	/*
	 * Update rigidbody collision shape
	*/

	_rigidBody->setCollisionShape (_colliderComponent->GetCollisionShape ());
}

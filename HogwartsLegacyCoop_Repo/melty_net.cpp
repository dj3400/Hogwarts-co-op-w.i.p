#include "melty_net.h"

// Helper for Linear Interpolation
float Lerp(float start, float end, float alpha) {
    return start + (end - start) * alpha;
}

// Update the Ghost Companion's position and rotation based on the Host
void UpdateGhostCompanion(PlayerPuppet& host, PlayerPuppet& ghost) {
    // Target Position: Slightly behind (-1.5x) and to the right (+2.0z) of the Host
    float targetX = host.position.x - 1.5f;
    float targetZ = host.position.z + 2.0f;
    float targetY = host.position.y;

    // Smoothly move the Ghost toward its target (Interpolation factor of 0.1)
    ghost.position.x += (targetX - ghost.position.x) * 0.15f;
    ghost.position.z += (targetZ - ghost.position.z) * 0.15f;
    ghost.position.y += (targetY - ghost.position.y) * 0.15f;

    // Sync Rotation: The Ghost looks slightly more toward the Host's direction
    ghost.rotation.yaw = host.rotation.yaw;
    ghost.rotation.pitch = host.rotation.pitch;
}

// Main Network Tick - Called every frame (e.g., 60 times per second)
void UpdateNetworkTick(MeltySession& session, float deltaTime) {
    // 1. Process Host Input and Physics
    HostMoveInput(session.host, deltaTime);
    ApplyPhysics(session.host, deltaTime);

    // 2. Update the Ghost Companion (ID: 100)
    for (auto& puppet : session.puppets) {
        if (puppet.id == 100) {
            UpdateGhostCompanion(session.host, puppet);
            ApplyPhysics(puppet, deltaTime);
        } else {
            // 3. Replicate and Update all other players
            ReplicatePuppet(puppet, deltaTime);
            ApplyPhysics(puppet, deltaTime);
        }
    }

    session.totalTime += deltaTime;
}

// Host Movement Logic: Handles the primary player's input with acceleration
void HostMoveInput(PlayerPuppet& host, float deltaTime) {
    float targetVelocityX = 5.0f; // Target speed on X axis
    float targetVelocityZ = 2.0f; // Target speed on Z axis

    host.velocity.x += (targetVelocityX - host.velocity.x) * host.acceleration * deltaTime;
    host.velocity.z += (targetVelocityZ - host.velocity.z) * host.acceleration * deltaTime;

    host.position.x += host.velocity.x * deltaTime;
    host.position.z += host.velocity.z * deltaTime;

    float currentSpeed = std::sqrt(std::pow(host.velocity.x, 2) + std::pow(host.velocity.z, 2));
    if (currentSpeed > 0.1f && currentSpeed < host.maxSpeed * 0.8f) {
        host.stance = 1; // Walking
    } else if (currentSpeed >= host.maxSpeed * 0.8f) {
        host.stance = 2; // Running
    } else {
        host.stance = 0; // Idle
    }

    float animationSpeed = currentSpeed * 2.0f;
    host.animationProgress += (animationSpeed * deltaTime);
    if (host.animationProgress > 1.0f) host.animationProgress -= 1.0f;
}

// Remote Puppet Replication: Handles how other players move and animate
void ReplicatePuppet(PlayerPuppet& puppet, float deltaTime) {
    float personalitySpeed = (puppet.id % 3 + 1) * 0.5f;
    float targetVelX = personalitySpeed * 4.0f;
    float targetVelZ = personalitySpeed * 2.0f;

    puppet.velocity.x += (targetVelX - puppet.velocity.x) * puppet.acceleration * deltaTime;
    puppet.velocity.z += (targetVelZ - puppet.velocity.z) * puppet.acceleration * deltaTime;

    puppet.position.x += puppet.velocity.x * deltaTime;
    puppet.position.z += puppet.velocity.z * deltaTime;

    float currentSpeed = std::sqrt(std::pow(puppet.velocity.x, 2) + std::pow(puppet.velocity.z, 2));
    float animationSpeed = currentSpeed * 2.0f;
    puppet.animationProgress += (animationSpeed * deltaTime);
    if (puppet.animationProgress > 1.0f) puppet.animationProgress -= 1.0f;

    if (currentSpeed > 0.1f && currentSpeed < puppet.maxSpeed * 0.8f) {
        puppet.stance = 1; // Walking
    } else if (currentSpeed >= puppet.maxSpeed * 0.8f) {
        puppet.stance = 2; // Running
    } else {
        puppet.stance = 0; // Idle
    }
}

// Physics Engine: Handles Friction and Gravity
void ApplyPhysics(PlayerPuppet& puppet, float deltaTime) {
    if (std::abs(puppet.velocity.x) > 0.1f) {
        puppet.velocity.x *= Lerp(1.0f, 0.95f, puppet.friction * deltaTime);
    } else {
        puppet.velocity.x = 0;
    }

    if (std::abs(puppet.velocity.z) > 0.1f) {
        puppet.velocity.z *= Lerp(1.0f, 0.95f, puppet.friction * deltaTime);
    } else {
        puppet.velocity.z = 0;
    }

    if (puppet.stance == 3 || puppet.position.y < 10.0f) {
        puppet.velocity.y -= 9.81f * deltaTime;
        puppet.position.y += puppet.velocity.y * deltaTime;
    } else if (puppet.stance == 3 && puppet.position.y >= 10.0f) {
        puppet.position.y = 10.0f;
        puppet.velocity.y = 0;
    }
}

// Handle World Transition: Solves the M2.1 "Generation" problem
void HandleWorldTransition(MeltySession& session, uint32_t newGeneration, Vector3 newOrigin) {
    session.currentGeneration = newGeneration;
    
    host.generationId = newGeneration;
    host.relativePosition = host.position - newOrigin;

    for (auto& puppet : session.puppets) {
        puppet.generationId = newGeneration;
        puppet.relativePosition = puppet.position - newOrigin;
    }
}

// Smooth Rotation: Prevents characters from snapping instantly to face directions
void SmoothRotation(PlayerPuppet& puppet, float targetYaw, float deltaTime) {
    float rotationSpeed = 5.0f; // Degrees per second
    puppet.rotation.yaw += (targetYaw - puppet.rotation.yaw) * (rotationSpeed * deltaTime);
}

// Version 1.0 Final Polish Functions
void UpdateAnimationSync(PlayerPuppet& puppet, float deltaTime) {
    float currentSpeed = std::sqrt(std::pow(puppet.velocity.x, 2) + std::pow(puppet.velocity.z, 2));
    float animationSpeed = currentSpeed * 2.0f;
    puppet.animationProgress += (animationSpeed * deltaTime);
    if (puppet.animationProgress > 1.0f) puppet.animationProgress -= 1.0f;

    if (currentSpeed > 0.1f && currentSpeed < puppet.maxSpeed * 0.8f) {
        puppet.stance = 1; // Walking
    } else if (currentSpeed >= puppet.maxSpeed * 0.8f) {
        puppet.stance = 2; // Running
    } else {
        puppet.stance = 0; // Idle
    }
}

void SyncGhostToHost(PlayerPuppet& host, PlayerPuppet& ghost) {
    float targetX = host.position.x - 1.5f;
    float targetZ = host.position.z + 2.0f;
    float targetY = host.position.y;

    ghost.position.x += (targetX - ghost.position.x) * 0.15f;
    ghost.position.z += (targetZ - ghost.position.z) * 0.15f;
    ghost.position.y += (targetY - ghost.position.y) * 0.15f;

    ghost.rotation.yaw = host.rotation.yaw;
    ghost.rotation.pitch = host.rotation.pitch;
}

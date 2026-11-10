#pragma once

#include <string>
#include <vector>
#include <cmath>

// 3D Vector for position, velocity, and scale
struct Vector3 {
    float x, y, z;
};

// Rotation (Euler angles)
struct Vector3Rotation {
    float pitch, yaw, roll;
};

// The core Puppet structure - every character in the game is a puppet.
struct PlayerPuppet {
    uint32_t id;               // Unique ID for this specific instance
    std::string name;          // Character name (e.g., "Host", "Companion")
    Vector3 position;           // Current X, Y, Z
    Vector3Rotation rotation;   // Current Pitch, Yaw, Roll
    
    // Movement Physics
    Vector3 velocity;           // Current speed and direction per axis
    float acceleration;         // How quickly we reach top speed
    float friction;             // How quickly we slow down
    float maxSpeed;             // Top speed (e.g., 5.0 for running)

    float movementSpeed;        // Normalized speed (0 to 1)
    float animationProgress;    // Progress through the current animation cycle (0.0 to 1.0)
    int stance;                 // 0: Idle, 1: Walking, 2: Running, 3: Jumping

    // World & Transition Logic
    uint32_t generationId;     // Unique ID for the current level/area
    Vector3 relativePosition;   // Position relative to the start of the current area

    // Helper to calculate distance from another point
    float DistanceTo(Vector3 other) const {
        return std::sqrt(std::pow(position.x - other.x, 2) + 
                         std::pow(position.y - other.y, 2) + 
                         std::pow(position.z - other.z, 2));
    }
};

// The Network Session state
struct MeltySession {
    uint32_t sessionId;
    bool isHost;
    PlayerPuppet host;
    std::vector<PlayerPuppet> puppets; // All other players + the Ghost Companion
    float totalTime;
    uint32_t currentGeneration;       // The shared generation ID for all puppets
};

// Core Network Functions
void UpdateGhostCompanion(PlayerPuppet& host, PlayerPuppet& ghost);
void UpdateNetworkTick(MeltySession& session, float deltaTime);
void HostMoveInput(PlayerPuppet& host, float deltaTime);
void ReplicatePuppet(PlayerPuppet& puppet, float deltaTime);
void ApplyPhysics(PlayerPuppet& puppet, float deltaTime);
void HandleWorldTransition(MeltySession& session, uint32_t newGeneration, Vector3 newOrigin);
void SmoothRotation(PlayerPuppet& puppet, float targetYaw, float deltaTime);

// Version 1.0 Final Polish Functions
void UpdateAnimationSync(PlayerPuppet& puppet, float deltaTime);
void SyncGhostToHost(PlayerPuppet& host, PlayerPuppet& ghost);

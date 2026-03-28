#ifndef PHYSICS_WORLD_HPP
#define PHYSICS_WORLD_HPP

#include <memory>
#include <vector>

#include "IPhysicsBody.hpp"
#include "StaticBody.hpp"

struct RopeConstraint {
    IPhysicsBody* bodyA   = nullptr;
    IPhysicsBody* bodyB   = nullptr;
    float         maxLen  = 200.f;
    float         friction = 0.5f;
    // TODO implement force resolution in StepRopes().
};

// ------------------------------------------------------------
// PhysicsWorld
// Central coordinator for the two-phase physics pipeline:
//
//   Phase 1  : StepPhysicsUpdate()
//               - Calls PhysicsUpdate() on every active, non-frozen body.
//               - Bodies read input, apply gravity, and produce a desired
//                 delta via GetDesiredDelta(). Positions are NOT changed.
//
//   Middle   : StepResolveAndApply()
//               - Detects riding relationships (stacking) from the current
//                 pre-move positions.
//               - Resolves each body's delta against all solid geometry,
//                 separating overlaps with horizontal-then-vertical passes.
//               - Propagates the support body's horizontal delta to riders
//                 automatically (platform following, cat-on-cat stacking).
//               - Calls ApplyResolvedDelta() on every body.
//
//   Callbacks: StepCollisionCallbacks()
//               - Calls OnCollision() on each body that encountered a
//                 solid during resolution, supplying the contact normal.
//
// Freeze:
//   FreezeAll() / UnfreezeAll() toggle the frozen flag on every registered
//   body, enabling game-pause without destroying simulation state.
//
// Scene usage pattern:
//   1. OnEnter: Register() player cats + PushableBoxes.
//              AddStaticBoundary() for floor, walls, ceiling.
//   2. Update: Set moveDir / call Jump() on actors (input phase).
//              Then call m_World.Update() once per frame.
//   3. OnExit: m_World.Clear().
// ------------------------------------------------------------
class PhysicsWorld {
public:
    PhysicsWorld()  = default;
    ~PhysicsWorld() = default;

    PhysicsWorld(const PhysicsWorld&)            = delete;
    PhysicsWorld& operator=(const PhysicsWorld&) = delete;

    // Register a body for inclusion in every future Update() step.
    void Register(const std::shared_ptr<IPhysicsBody>& body);

    // Remove a specific body (matched by pointer identity).
    void Unregister(const IPhysicsBody* body);

    // Unregister all bodies and remove all rope constraints.
    // Owned StaticBodies created via AddStaticBoundary are also cleared.
    void Clear();

    // Create a StaticBody, take ownership, and register it.
    // Returns a raw non-owning pointer for identification if needed.
    StaticBody* AddStaticBoundary(glm::vec2 center, glm::vec2 halfSize,
                                   BodyType type = BodyType::STATIC_BOUNDARY);

    // Rope constraints (structural wiring only; step not yet implemented).
    void AddRope(IPhysicsBody* a, IPhysicsBody* b,
                 float maxLen = 200.f, float friction = 0.5f);
    void RemoveRope(IPhysicsBody* a, IPhysicsBody* b);
    std::vector<RopeConstraint*> GetRopesOf(const IPhysicsBody* body);

    // Run one complete physics step (Phase 1 -> resolve -> apply -> callbacks).
    void Update();

    // Freeze / unfreeze all registered bodies (for game pause).
    void FreezeAll() const ;
    void UnfreezeAll() const ;

    // Count the number of CHARACTER bodies that are actively pushing toward
    // "target" in direction "dir" (+1 = right, -1 = left), including chained
    // pushers and box intermediaries.
    // Used by PushableBox::PhysicsUpdate() to determine net push force.
    [[nodiscard]] int CountCharactersPushing(const IPhysicsBody* target, int dir) const;

    // Return all registered active bodies of the given type.
    [[nodiscard]] std::vector<IPhysicsBody*> GetBodiesOfType(BodyType type) const;

private:
    // Per-frame working data for one body during resolution.
    struct BodyInfo {
        std::shared_ptr<IPhysicsBody> body;
        glm::vec2 desired      = {0.f, 0.f};
        glm::vec2 resolved     = {0.f, 0.f};
        int       supportIdx   = -1;    // index in the snapshot vector (-1 = none)
        bool      resolvedFlag = false; // true once this body's delta is finalised
        // Contact tracking for OnCollision callbacks.
        IPhysicsBody* collidedH  = nullptr;   // body that caused horizontal separation
        IPhysicsBody* collidedV  = nullptr;   // body that caused vertical separation
        glm::vec2     normalH    = {0.f, 0.f};
        glm::vec2     normalV    = {0.f, 0.f};
    };

    // Phase 1: call PhysicsUpdate() on all active non-frozen bodies.
    // CHARACTER bodies are updated before PUSHABLE_BOX bodies so that
    // CountCharactersPushing() sees current-frame move directions.
    void StepPhysicsUpdate() const;

    // Middle + Phase 2: full resolution pipeline (detect riding, resolve, apply).
    void StepResolveAndApply() const ;

    // Detect which body in the snapshot is being "ridden" by each dynamic body,
    // based on the pre-move positions. Populates BodyInfo::supportIdx.
    static void DetectRiding(std::vector<BodyInfo>& infos);

    // Resolve body at index "idx" with the given "desired" delta against every
    // solid body in the snapshot. Returns the safe resolved delta and populates
    // contact tracking fields in infos[idx].
    static glm::vec2 ResolveBody(int idx, glm::vec2 desired,
                                  std::vector<BodyInfo>& infos);

    // Fire OnCollision() on each body that recorded a contact during resolution.
    static void StepCollisionCallbacks(const std::vector<BodyInfo>& infos);

    // Standard AABB overlap test (touching edges are not considered overlapping).
    static bool AabbOverlaps(const IPhysicsBody* a, const IPhysicsBody* b);

    // Remove expired weak_ptr entries (runs every kPurgeInterval frames).
    void PurgeExpired();

    // Recursive helper for CountCharactersPushing().
    int CountCharactersPushingImpl(const IPhysicsBody* target,
                                    int dir,
                                    std::vector<const IPhysicsBody*>& visited) const;

    std::vector<std::weak_ptr<IPhysicsBody>> m_Bodies;
    std::vector<std::shared_ptr<StaticBody>> m_OwnedStatics; // owned by world
    std::vector<RopeConstraint>              m_Ropes;

    int m_FrameCount = 0;
    static constexpr int kPurgeInterval = 60;
    static constexpr float kRidingTolerance = 5.f;
};

#endif // PHYSICS_WORLD_HPP
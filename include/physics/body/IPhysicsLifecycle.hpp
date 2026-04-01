#ifndef IPHYSICS_LIFECYCLE_HPP
#define IPHYSICS_LIFECYCLE_HPP

class IPhysicsLifecycle {
public:
    virtual ~IPhysicsLifecycle() = default;

    [[nodiscard]] virtual bool IsActive() const = 0;
    virtual void SetActive(bool active) = 0;

    [[nodiscard]] virtual bool IsFrozen() const = 0;
    virtual void Freeze() = 0;
    virtual void Unfreeze() = 0;

    virtual void PostUpdate() = 0;
};

#endif // IPHYSICS_LIFECYCLE_HPP


#ifndef IPHYSICS_PUSH_REACTIVE_HPP
#define IPHYSICS_PUSH_REACTIVE_HPP

class IPhysicsPushReactive {
public:
    virtual ~IPhysicsPushReactive() = default;

    [[nodiscard]] virtual int GetMoveDir() const = 0;
    virtual void NotifyPush() = 0;
};

#endif // IPHYSICS_PUSH_REACTIVE_HPP


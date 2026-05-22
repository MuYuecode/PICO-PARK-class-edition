#ifndef IPHYSICS_MATERIAL_HPP
#define IPHYSICS_MATERIAL_HPP

class IPhysicsMaterial {
public:
    virtual ~IPhysicsMaterial() = default;

    [[nodiscard]] virtual bool IsSolid() const = 0;
    [[nodiscard]] virtual bool IsKinematic() const = 0;
};

#endif // IPHYSICS_MATERIAL_HPP


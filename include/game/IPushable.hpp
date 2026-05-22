//
// Created by cody2 on 2026/3/28.
//

#ifndef IPUSHABLE_HPP
#define IPUSHABLE_HPP
class IPushable {
public:
    virtual ~IPushable() = default;
    [[nodiscard]] virtual int GetRequiredPushers() const = 0;
};

#endif // IPUSHABLE_HPP

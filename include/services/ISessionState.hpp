#ifndef I_SESSION_STATE_HPP
#define I_SESSION_STATE_HPP

#include <array>

#include "gameplay/PlayerKeyConfig.hpp"

class ISessionState {
public:
    virtual ~ISessionState() = default;

    virtual int GetSelectedPlayerCount() const = 0;
    virtual void SetSelectedPlayerCount(int count) = 0;

    virtual int GetCooperativePushPower() const = 0;
    virtual void SetCooperativePushPower(int power) = 0;

    virtual const std::array<PlayerKeyConfig, 8>& GetAppliedKeyConfigs() const = 0;
    virtual std::array<PlayerKeyConfig, 8>& MutableAppliedKeyConfigs() = 0;
    virtual int GetConfiguredPlayerCount() const = 0;

    virtual bool ShouldQuit() const = 0;
    virtual void RequestQuit() = 0;
    virtual void ClearQuitRequest() = 0;
};

#endif // I_SESSION_STATE_HPP



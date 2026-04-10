#ifndef SESSION_STATE_HPP
#define SESSION_STATE_HPP

#include <array>

#include "services/ISessionState.hpp"

class SessionState final : public ISessionState {
public:
    int GetSelectedPlayerCount() const override { return m_SelectedPlayerCount; }
    void SetSelectedPlayerCount(int count) override { m_SelectedPlayerCount = count; }

    int GetCooperativePushPower() const override { return m_CooperativePushPower; }
    void SetCooperativePushPower(int power) override { m_CooperativePushPower = power; }

    const std::array<PlayerKeyConfig, 8>& GetAppliedKeyConfigs() const override {
        return m_AppliedKeyConfigs;
    }

    std::array<PlayerKeyConfig, 8>& MutableAppliedKeyConfigs() override {
        return m_AppliedKeyConfigs;
    }

    int GetConfiguredPlayerCount() const override {
        int count = 0;
        for (const auto& cfg : m_AppliedKeyConfigs) {
            if (static_cast<int>(cfg.AllKeys().size()) >= 4) {
                ++count;
            }
        }
        return count;
    }

    bool ShouldQuit() const override { return m_ShouldQuit; }
    void RequestQuit() override { m_ShouldQuit = true; }
    void ClearQuitRequest() override { m_ShouldQuit = false; }

private:
    int m_SelectedPlayerCount = 2;
    int m_CooperativePushPower = 1;
    std::array<PlayerKeyConfig, 8> m_AppliedKeyConfigs{};
    bool m_ShouldQuit = false;
};

#endif // SESSION_STATE_HPP



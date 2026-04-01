#ifndef VISUAL_THEME_SERVICE_HPP
#define VISUAL_THEME_SERVICE_HPP

#include <array>
#include <memory>

#include "gameplay/actors/Character.hpp"
#include "services/theme/IVisualThemeService.hpp"

class VisualThemeService final : public IVisualThemeService {
public:
    explicit VisualThemeService(std::shared_ptr<Character> background);

    void ApplyBackgroundByIndex(int idx) override;
    void RestoreAppliedBackground() override;

    int GetAppliedBackgroundIndex() const override { return m_AppliedBackgroundIndex; }
    int ClampBackgroundIndex(int idx) const override;

private:
    std::shared_ptr<Character> m_Background;
    int m_AppliedBackgroundIndex = 0;

    static const std::array<const char*, 3> kBackgroundPaths;
};

#endif // VISUAL_THEME_SERVICE_HPP



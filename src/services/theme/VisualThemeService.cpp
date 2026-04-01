#include "services/theme/VisualThemeService.hpp"

const std::array<const char*, 3> VisualThemeService::kBackgroundPaths = {
    GA_RESOURCE_DIR "/Image/Background/white_background.png",
    GA_RESOURCE_DIR "/Image/Background/cream_background.png",
    GA_RESOURCE_DIR "/Image/Background/dark_background.png",
};

VisualThemeService::VisualThemeService(std::shared_ptr<Character> background)
    : m_Background(std::move(background)) {}

int VisualThemeService::ClampBackgroundIndex(int idx) const {
    if (idx < 0) {
        return 0;
    }

    const int maxIndex = static_cast<int>(kBackgroundPaths.size()) - 1;
    if (idx > maxIndex) {
        return maxIndex;
    }

    return idx;
}

void VisualThemeService::ApplyBackgroundByIndex(int idx) {
    m_AppliedBackgroundIndex = ClampBackgroundIndex(idx);
    if (m_Background != nullptr) {
        m_Background->SetImage(kBackgroundPaths[static_cast<size_t>(m_AppliedBackgroundIndex)]);
    }
}

void VisualThemeService::RestoreAppliedBackground() {
    ApplyBackgroundByIndex(m_AppliedBackgroundIndex);
}



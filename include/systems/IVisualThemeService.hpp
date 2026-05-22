#ifndef I_VISUAL_THEME_SERVICE_HPP
#define I_VISUAL_THEME_SERVICE_HPP

class IVisualThemeService {
public:
    virtual ~IVisualThemeService() = default;

    virtual void ApplyBackgroundByIndex(int idx) = 0;

    virtual int ClampBackgroundIndex(int idx) const = 0;
};

#endif // I_VISUAL_THEME_SERVICE_HPP



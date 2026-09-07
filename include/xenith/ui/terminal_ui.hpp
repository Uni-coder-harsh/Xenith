#ifndef XENITH_UI_TERMINAL_UI_HPP
#define XENITH_UI_TERMINAL_UI_HPP

#include <string>
#include <vector>
#include <utility>
#include <chrono>
#include <functional>

namespace xenith::ui {

enum class ColorScheme {
    NEON_CYAN,
    NEON_MAGENTA,
    NEON_GREEN,
    GOLD,
    PURPLE,
    RED,
    GRAY,
    WHITE,
    RESET
};

class TerminalUI {
public:
    static bool isColorSupported();
    static std::string colorize(const std::string& text, ColorScheme scheme, bool bold = false);
    
    // Core ASCII Logo & Subtitle
    static void printLogo();
    
    // Step & Spinner progress indicators
    static void printStepHeader(int step, int total, const std::string& title);
    static void printStepSuccess(int step, int total, const std::string& title, double duration_ms);
    static void printStepError(int step, int total, const std::string& title, const std::string& error_msg);
    
    static void showAnimatedSpinner(const std::string& message, double duration_ms);

    // Box & Dashboard Rendering
    struct KeyValue {
        std::string key;
        std::string value;
        ColorScheme val_color{ColorScheme::WHITE};
        bool val_bold{false};
    };

    static void printBox(const std::string& title,
                         const std::vector<KeyValue>& items,
                         ColorScheme border_color = ColorScheme::NEON_CYAN);

    static void printStatusBadge(const std::string& status_label, bool is_success);

    static void printErrorBox(const std::string& error_type, const std::vector<std::string>& details);
};

} // namespace xenith::ui

#endif // XENITH_UI_TERMINAL_UI_HPP

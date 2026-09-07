#include "xenith/ui/terminal_ui.hpp"
#include <iostream>
#include <iomanip>
#include <thread>
#include <cmath>
#include <algorithm>
#include <unistd.h>

namespace xenith::ui {

bool TerminalUI::isColorSupported() {
    static bool checked = false;
    static bool supported = false;
    if (!checked) {
        const char* no_color = std::getenv("NO_COLOR");
        const char* term = std::getenv("TERM");
        bool is_tty = isatty(fileno(stdout)) != 0;
        bool dumb_term = (term && std::string(term) == "dumb");
        supported = is_tty && !no_color && !dumb_term;
        checked = true;
    }
    return supported;
}

std::string TerminalUI::colorize(const std::string& text, ColorScheme scheme, bool bold) {
    if (!isColorSupported()) {
        return text;
    }

    std::string prefix = "\033[";
    if (bold) {
        prefix += "1;";
    }

    switch (scheme) {
        case ColorScheme::NEON_CYAN:    prefix += "38;2;0;243;255m"; break;
        case ColorScheme::NEON_MAGENTA: prefix += "38;2;255;0;127m"; break;
        case ColorScheme::NEON_GREEN:   prefix += "38;2;0;255;127m"; break;
        case ColorScheme::GOLD:         prefix += "38;2;255;215;0m"; break;
        case ColorScheme::PURPLE:       prefix += "38;2;168;85;247m"; break;
        case ColorScheme::RED:          prefix += "38;2;239;68;68m"; break;
        case ColorScheme::GRAY:         prefix += "90m"; break;
        case ColorScheme::WHITE:        prefix += "97m"; break;
        case ColorScheme::RESET:        return "\033[0m" + text;
    }

    return prefix + text + "\033[0m";
}

void TerminalUI::printLogo() {
    std::cout << "\n";
    std::string line1 = "  ██╗  ██╗███████╗███╗   ██╗██╗████████╗██╗  ██╗";
    std::string line2 = "  ╚██╗██╔╝██╔════╝████╗  ██║██║╚══██╔══╝██║  ██║";
    std::string line3 = "   ╚███╔╝ █████╗  ██╔██╗ ██║██║   ██║   ███████║";
    std::string line4 = "   ██╔██╗ ██╔══╝  ██║╚██╗██║██║   ██║   ██╔══██║";
    std::string line5 = "  ██╔╝ ██╗███████╗██║ ╚████║██║   ██║   ██║  ██║";
    std::string line6 = "  ╚═╝  ╚═╝╚══════╝╚═╝  ╚═══╝╚═╝   ╚═╝   ╚═╝  ╚═╝";

    std::cout << colorize(line1, ColorScheme::NEON_CYAN, true) << "\n";
    std::cout << colorize(line2, ColorScheme::NEON_CYAN, true) << "\n";
    std::cout << colorize(line3, ColorScheme::NEON_MAGENTA, true) << "\n";
    std::cout << colorize(line4, ColorScheme::NEON_MAGENTA, true) << "\n";
    std::cout << colorize(line5, ColorScheme::PURPLE, true) << "\n";
    std::cout << colorize(line6, ColorScheme::PURPLE, true) << "\n";
    
    std::string subtitle = "  ⚡ Sovereign Mathematical Optimization Solver Engine";
    std::string version = " [v0.3.0 LP Revised Simplex]";
    std::cout << colorize(subtitle, ColorScheme::GOLD, true) 
              << colorize(version, ColorScheme::GRAY) << "\n\n";
}

void TerminalUI::printStepHeader(int step, int total, const std::string& title) {
    std::string step_str = "[" + std::to_string(step) + "/" + std::to_string(total) + "]";
    std::cout << colorize(step_str, ColorScheme::PURPLE, true) << " "
              << colorize("⠋ " + title + "...", ColorScheme::NEON_CYAN) << std::flush;
}

void TerminalUI::printStepSuccess(int step, int total, const std::string& title, double duration_ms) {
    std::string step_str = "[" + std::to_string(step) + "/" + std::to_string(total) + "]";
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << duration_ms << " ms";
    
    std::cout << "\r\033[K" 
              << colorize(step_str, ColorScheme::PURPLE, true) << " "
              << colorize("✔ ", ColorScheme::NEON_GREEN, true)
              << colorize(title, ColorScheme::WHITE, true) << " "
              << colorize("(" + ss.str() + ")", ColorScheme::GRAY) << "\n";
}

void TerminalUI::printStepError(int step, int total, const std::string& title, const std::string& error_msg) {
    std::string step_str = "[" + std::to_string(step) + "/" + std::to_string(total) + "]";
    std::cout << "\r\033[K"
              << colorize(step_str, ColorScheme::PURPLE, true) << " "
              << colorize("✖ ", ColorScheme::RED, true)
              << colorize(title + " FAILED: " + error_msg, ColorScheme::RED, true) << "\n";
}

void TerminalUI::showAnimatedSpinner(const std::string& message, double duration_ms) {
    if (!isColorSupported()) {
        std::cout << message << "\n";
        return;
    }

    const std::vector<std::string> frames = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};
    auto start = std::chrono::steady_clock::now();
    size_t frame_idx = 0;

    while (true) {
        auto now = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(now - start).count();
        if (elapsed >= duration_ms) break;

        std::cout << "\r\033[K" 
                  << colorize(frames[frame_idx % frames.size()], ColorScheme::NEON_CYAN, true) << " "
                  << colorize(message, ColorScheme::WHITE) << std::flush;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
        frame_idx++;
    }
}

void TerminalUI::printBox(const std::string& title,
                         const std::vector<KeyValue>& items,
                         ColorScheme border_color) {
    size_t max_k_len = title.length();
    size_t max_v_len = 0;

    for (const auto& item : items) {
        max_k_len = std::max(max_k_len, item.key.length());
        max_v_len = std::max(max_v_len, item.value.length());
    }

    size_t box_width = std::max(max_k_len + max_v_len + 6, size_t(56));

    // Border strings
    std::string top_border = "┌";
    std::string title_sep = "├";
    std::string bot_border = "└";
    for (size_t i = 0; i < box_width - 2; ++i) {
        top_border += "─";
        title_sep += "─";
        bot_border += "─";
    }
    top_border += "┐";
    title_sep += "┤";
    bot_border += "┘";

    std::cout << colorize(top_border, border_color, true) << "\n";

    // Title line
    std::cout << colorize("│ ", border_color, true)
              << colorize(title, ColorScheme::GOLD, true);
    size_t padding = box_width - 4 - title.length();
    std::cout << std::string(padding, ' ')
              << colorize(" │", border_color, true) << "\n";

    std::cout << colorize(title_sep, border_color, true) << "\n";

    // Key-value lines
    for (const auto& item : items) {
        std::cout << colorize("│ ", border_color, true);
        std::cout << colorize(item.key, ColorScheme::NEON_CYAN, false);
        
        size_t k_pad = max_k_len + 2 - item.key.length();
        std::cout << std::string(k_pad, ' ');

        std::string val_str = colorize(item.value, item.val_color, item.val_bold);
        std::cout << val_str;

        size_t total_content_len = item.key.length() + k_pad + item.value.length();
        size_t r_pad = (box_width > total_content_len + 4) ? (box_width - 4 - total_content_len) : 0;
        std::cout << std::string(r_pad, ' ');

        std::cout << colorize(" │", border_color, true) << "\n";
    }

    std::cout << colorize(bot_border, border_color, true) << "\n\n";
}

void TerminalUI::printStatusBadge(const std::string& status_label, bool is_success) {
    if (is_success) {
        std::cout << colorize("STATUS: ", ColorScheme::GRAY)
                  << colorize("[ " + status_label + " ✔ ]", ColorScheme::NEON_GREEN, true) << "\n\n";
    } else {
        std::cout << colorize("STATUS: ", ColorScheme::GRAY)
                  << colorize("[ " + status_label + " ✖ ]", ColorScheme::RED, true) << "\n\n";
    }
}

void TerminalUI::printErrorBox(const std::string& error_type, const std::vector<std::string>& details) {
    std::cout << colorize("┌── " + error_type + " ─────────────────────────────────────────┐", ColorScheme::RED, true) << "\n";
    for (const auto& detail : details) {
        std::cout << colorize("│ ", ColorScheme::RED, true)
                  << colorize(detail, ColorScheme::WHITE) << "\n";
    }
    std::cout << colorize("└──────────────────────────────────────────────────────────┘", ColorScheme::RED, true) << "\n\n";
}

} // namespace xenith::ui

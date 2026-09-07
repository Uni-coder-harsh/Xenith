#ifndef XENITH_TEST_HARNESS_HPP
#define XENITH_TEST_HARNESS_HPP

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <cmath>
#include <sstream>
#include <utility>
#include <type_traits>
#include <chrono>
#include <iomanip>
#include "xenith/ui/terminal_ui.hpp"

namespace xenith::test {

struct TestCase {
    std::string name;
    std::function<void()> func;
};

class TestRunner {
public:
    static TestRunner& instance() {
        static TestRunner runner;
        return runner;
    }

    void addTest(const std::string& name, std::function<void()> func) {
        m_tests.push_back({name, func});
    }

    int runAll() {
        using namespace xenith::ui;

        int passed = 0;
        int failed = 0;

        std::cout << "\n" 
                  << TerminalUI::colorize("┌─────────────────────────────────────────────────────────────┐", ColorScheme::PURPLE, true) << "\n"
                  << TerminalUI::colorize("│ ⚡ XENITH TEST SUITE EXECUTION                              │", ColorScheme::GOLD, true) << "\n"
                  << TerminalUI::colorize("├─────────────────────────────────────────────────────────────┤", ColorScheme::PURPLE, true) << "\n"
                  << TerminalUI::colorize("│ Total Test Cases: ", ColorScheme::PURPLE, true) 
                  << TerminalUI::colorize(std::to_string(m_tests.size()), ColorScheme::WHITE, true) << "\n"
                  << TerminalUI::colorize("└─────────────────────────────────────────────────────────────┘", ColorScheme::PURPLE, true) << "\n\n";

        auto t_suite_start = std::chrono::high_resolution_clock::now();

        for (const auto& test : m_tests) {
            std::cout << TerminalUI::colorize("[ RUN      ] ", ColorScheme::NEON_CYAN, true) 
                      << TerminalUI::colorize(test.name, ColorScheme::WHITE, true) << std::endl;
            
            m_currentTestFailed = false;
            m_failureMessage.clear();
            
            auto t_start = std::chrono::high_resolution_clock::now();
            try {
                test.func();
            } catch (const std::exception& ex) {
                m_currentTestFailed = true;
                m_failureMessage = std::string("Unhandled exception: ") + ex.what();
            } catch (...) {
                m_currentTestFailed = true;
                m_failureMessage = "Unhandled unknown exception";
            }
            auto t_end = std::chrono::high_resolution_clock::now();
            double duration_ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();

            std::stringstream ss_dur;
            ss_dur << std::fixed << std::setprecision(2) << duration_ms << " ms";

            if (!m_currentTestFailed) {
                std::cout << TerminalUI::colorize("[       OK ] ", ColorScheme::NEON_GREEN, true) 
                          << TerminalUI::colorize(test.name, ColorScheme::WHITE, true) << " "
                          << TerminalUI::colorize("(" + ss_dur.str() + ")", ColorScheme::GRAY) << std::endl;
                passed++;
            } else {
                std::cout << TerminalUI::colorize("[  FAILED  ] ", ColorScheme::RED, true) 
                          << TerminalUI::colorize(test.name, ColorScheme::RED, true) << " "
                          << TerminalUI::colorize("(" + ss_dur.str() + ")", ColorScheme::GRAY) << std::endl;
                std::cout << TerminalUI::colorize("             ✖ ", ColorScheme::RED, true) 
                          << TerminalUI::colorize(m_failureMessage, ColorScheme::WHITE) << std::endl;
                failed++;
            }
        }

        auto t_suite_end = std::chrono::high_resolution_clock::now();
        double total_ms = std::chrono::duration<double, std::milli>(t_suite_end - t_suite_start).count();

        std::stringstream ss_total;
        ss_total << std::fixed << std::setprecision(2) << total_ms << " ms";

        std::cout << "\n";
        std::vector<TerminalUI::KeyValue> items = {
            {"Total Tests Executed", std::to_string(m_tests.size()), ColorScheme::WHITE},
            {"Passed Test Cases", std::to_string(passed), ColorScheme::NEON_GREEN, true},
            {"Failed Test Cases", std::to_string(failed), (failed == 0 ? ColorScheme::NEON_GREEN : ColorScheme::RED), true},
            {"Suite Execution Time", ss_total.str(), ColorScheme::GRAY},
            {"Suite Status", (failed == 0 ? "100% PASSED ✔" : "FAILURES DETECTED ✖"), 
                            (failed == 0 ? ColorScheme::NEON_GREEN : ColorScheme::RED), true}
        };

        TerminalUI::printBox("✦ TEST SUITE SUMMARY DASHBOARD", items, (failed == 0 ? ColorScheme::NEON_GREEN : ColorScheme::RED));

        return (failed == 0) ? 0 : 1;
    }

    void recordFailure(const std::string& msg) {
        m_currentTestFailed = true;
        if (!m_failureMessage.empty()) {
            m_failureMessage += " | ";
        }
        m_failureMessage += msg;
    }

private:
    std::vector<TestCase> m_tests;
    bool m_currentTestFailed{false};
    std::string m_failureMessage;
};

inline int registerAndRun(const std::string& name, std::function<void()> func) {
    TestRunner::instance().addTest(name, func);
    return 0;
}

template <typename T, typename U>
inline bool check_equal_helper(const T& a, const U& b) {
    if constexpr (std::is_integral_v<T> && std::is_integral_v<U>) {
        return std::cmp_equal(a, b);
    } else {
        return a == b;
    }
}

} // namespace xenith::test

#define XENITH_TEST(test_name) \
    void test_name(); \
    static int auto_register_##test_name = ::xenith::test::registerAndRun(#test_name, test_name); \
    void test_name()

#define XENITH_CHECK(condition) \
    do { \
        if (!(condition)) { \
            std::ostringstream _oss; \
            _oss << "Check failed: (" #condition ") at " << __FILE__ << ":" << __LINE__; \
            ::xenith::test::TestRunner::instance().recordFailure(_oss.str()); \
            return; \
        } \
    } while (0)

#define XENITH_CHECK_EQ(val1, val2) \
    do { \
        auto _v1 = (val1); \
        auto _v2 = (val2); \
        if (!::xenith::test::check_equal_helper(_v1, _v2)) { \
            std::ostringstream _oss; \
            _oss << "Equality check failed: (" #val1 " == " #val2 ") [" << _v1 << " vs " << _v2 << "] at " << __FILE__ << ":" << __LINE__; \
            ::xenith::test::TestRunner::instance().recordFailure(_oss.str()); \
            return; \
        } \
    } while (0)

#define XENITH_CHECK_NEAR(val1, val2, tol) \
    do { \
        double _v1 = static_cast<double>(val1); \
        double _v2 = static_cast<double>(val2); \
        double _diff = std::abs(_v1 - _v2); \
        if (_diff > (tol)) { \
            std::ostringstream _oss; \
            _oss << "Near check failed: (" #val1 " ~= " #val2 ") |" << _v1 << " - " << _v2 << "| = " << _diff << " > tol(" << (tol) << ") at " << __FILE__ << ":" << __LINE__; \
            ::xenith::test::TestRunner::instance().recordFailure(_oss.str()); \
            return; \
        } \
    } while (0)

#define XENITH_CHECK_THROW(expression, ExceptionType) \
    do { \
        bool _caught = false; \
        try { \
            expression; \
        } catch (const ExceptionType&) { \
            _caught = true; \
        } catch (...) { \
            _caught = false; \
        } \
        if (!_caught) { \
            std::ostringstream _oss; \
            _oss << "Expected exception " #ExceptionType " was not thrown at " << __FILE__ << ":" << __LINE__; \
            ::xenith::test::TestRunner::instance().recordFailure(_oss.str()); \
            return; \
        } \
    } while (0)

#endif // XENITH_TEST_HARNESS_HPP

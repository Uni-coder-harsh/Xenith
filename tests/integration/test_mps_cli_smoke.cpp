#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <filesystem>
#include "tests/test_harness.hpp"

namespace fs = std::filesystem;

static std::string findExecutable(const std::string& exe_name) {
    std::vector<std::string> candidates = {
        "./" + exe_name,
        "./bin/" + exe_name,
        "../" + exe_name,
        "../bin/" + exe_name,
        "../../" + exe_name,
        "../../bin/" + exe_name
    };

    for (const auto& path : candidates) {
        if (fs::exists(path)) {
            return path;
        }
    }
    return exe_name;
}

static std::string resolveFixturePath(const std::string& relative_path) {
    std::vector<std::string> candidates = {
        relative_path,
        "../" + relative_path,
        "../../" + relative_path
    };

    for (const auto& candidate : candidates) {
        if (fs::exists(candidate)) {
            return candidate;
        }
    }
    return relative_path;
}

XENITH_TEST(TestCliValidMps) {
    std::string exe = findExecutable("xenith_mps");
    std::string fixture = resolveFixturePath("tests/data/mps/afiro.mps");
    
    std::string cmd = exe + " " + fixture + " > /dev/null 2>&1";
    int ret = std::system(cmd.c_str());
    XENITH_CHECK(ret == 0);
}

XENITH_TEST(TestCliNonexistentFile) {
    std::string exe = findExecutable("xenith_mps");
    std::string cmd = exe + " nonexistent_file_xyz.mps > /dev/null 2>&1";
    int ret = std::system(cmd.c_str());
    XENITH_CHECK(ret != 0);
}

XENITH_TEST(TestCliMalformedFile) {
    std::string exe = findExecutable("xenith_mps");
    std::string fixture = resolveFixturePath("tests/data/mps/malformed_missing_rows.mps");
    
    std::string cmd = exe + " " + fixture + " > /dev/null 2>&1";
    int ret = std::system(cmd.c_str());
    XENITH_CHECK(ret != 0);
}

int main() {
    return xenith::test::TestRunner::instance().runAll();
}

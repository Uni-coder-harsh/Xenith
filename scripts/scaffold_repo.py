#!/usr/bin/env python3
"""
XENITH Repository Scaffolding Script
Creates directory tree and initial files for the XENITH optimization solver.
"""

import sys
from pathlib import Path

# Base directories to create
DIRECTORIES = [
    "docs/architecture",
    "docs/input",
    "docs/algorithms/lp",
    "docs/validation",
    "docs/benchmarks",
    "docs/engineering",
    "docs/decisions",
    "include/xenith/core",
    "include/xenith/model",
    "include/xenith/io",
    "include/xenith/presolve",
    "include/xenith/solver/common",
    "include/xenith/solver/lp",
    "include/xenith/solver/mip",
    "include/xenith/solver/qp",
    "include/xenith/numerics",
    "include/xenith/runtime",
    "include/xenith/solution",
    "include/xenith/common",
    "src/xenith/core",
    "src/xenith/model",
    "src/xenith/io",
    "src/xenith/presolve",
    "src/xenith/solver/common",
    "src/xenith/solver/lp",
    "src/xenith/solver/mip",
    "src/xenith/solver/qp",
    "src/xenith/numerics",
    "src/xenith/runtime",
    "src/xenith/solution",
    "src/xenith/common",
    "tests/unit/model",
    "tests/unit/io",
    "tests/unit/presolve",
    "tests/unit/numerics",
    "tests/unit/solver",
    "tests/unit/solution",
    "tests/integration",
    "tests/regression",
    "benchmarks/instances",
    "benchmarks/configs",
    "benchmarks/scripts",
    "benchmarks/results",
    "examples/lp",
    "examples/mps",
    "examples/api",
    "tools/benchmark_runner",
    "tools/model_inspector",
    "tools/solution_checker",
    "scripts",
    "python/xenith",
]

# Initial files to ensure exist
FILES_TO_TOUCH = [
    "README.md",
    "LICENSE",
    "CMakeLists.txt",
    "CMakePresets.json",
    ".gitignore",
    ".clang-format",
    ".clang-tidy",
    "docs/README.md",
    "docs/architecture/overview.md",
    "docs/architecture/canonical_model.md",
    "docs/architecture/solver_kernel.md",
    "docs/architecture/numerical_architecture.md",
    "docs/architecture/runtime_architecture.md",
    "docs/architecture/extensibility.md",
    "docs/input/mps_design.md",
    "docs/algorithms/lp/overview.md",
    "docs/algorithms/lp/revised_simplex.md",
    "docs/algorithms/lp/basis.md",
    "docs/algorithms/lp/factorization.md",
    "docs/validation/solution_validation.md",
    "docs/benchmarks/benchmark_strategy.md",
    "docs/engineering/engineering_principles.md",
    "docs/engineering/coding_standards.md",
    "docs/engineering/testing_strategy.md",
    "docs/engineering/development_workflow.md",
    "docs/decisions/README.md",
    "docs/decisions/ADR-001-cpp20.md",
    "docs/decisions/ADR-002-cmake.md",
    "docs/decisions/ADR-003-canonical-model.md",
    "docs/decisions/ADR-004-bounded-row-model.md",
    "docs/decisions/ADR-005-solver-numerics-separation.md",
    "docs/decisions/ADR-006-sparse-matrix-abstraction.md",
    "docs/decisions/ADR-007-lp-first.md",
    "docs/decisions/ADR-008-mps-input.md",
    "docs/decisions/ADR-009-platform-independent-core.md",
    "docs/decisions/ADR-010-benchmarking.md",
    "docs/roadmap.md",
    "docs/status.md",
    "benchmarks/instances/README.md",
    "benchmarks/results/README.md",
    "python/xenith/README.md",
    "include/xenith/core/.gitkeep",
    "include/xenith/model/.gitkeep",
    "include/xenith/io/.gitkeep",
    "include/xenith/presolve/.gitkeep",
    "include/xenith/solver/common/.gitkeep",
    "include/xenith/solver/lp/.gitkeep",
    "include/xenith/solver/mip/.gitkeep",
    "include/xenith/solver/qp/.gitkeep",
    "include/xenith/numerics/.gitkeep",
    "include/xenith/runtime/.gitkeep",
    "include/xenith/solution/.gitkeep",
    "include/xenith/common/.gitkeep",
    "src/xenith/core/.gitkeep",
    "src/xenith/model/.gitkeep",
    "src/xenith/io/.gitkeep",
    "src/xenith/presolve/.gitkeep",
    "src/xenith/solver/common/.gitkeep",
    "src/xenith/solver/lp/.gitkeep",
    "src/xenith/solver/mip/.gitkeep",
    "src/xenith/solver/qp/.gitkeep",
    "src/xenith/numerics/.gitkeep",
    "src/xenith/runtime/.gitkeep",
    "src/xenith/solution/.gitkeep",
    "src/xenith/common/.gitkeep",
    "tests/unit/model/.gitkeep",
    "tests/unit/io/.gitkeep",
    "tests/unit/presolve/.gitkeep",
    "tests/unit/numerics/.gitkeep",
    "tests/unit/solver/.gitkeep",
    "tests/unit/solution/.gitkeep",
    "tests/integration/.gitkeep",
    "tests/regression/.gitkeep",
    "benchmarks/configs/.gitkeep",
    "benchmarks/scripts/.gitkeep",
    "examples/lp/.gitkeep",
    "examples/mps/.gitkeep",
    "examples/api/.gitkeep",
    "tools/benchmark_runner/.gitkeep",
    "tools/model_inspector/.gitkeep",
    "tools/solution_checker/.gitkeep",
]

def main():
    root = Path(__file__).resolve().parent.parent
    print(f"Scaffolding XENITH repository at: {root}")

    created_dirs = 0
    created_files = 0
    skipped_files = 0

    for d in DIRECTORIES:
        path = root / d
        if not path.exists():
            path.mkdir(parents=True, exist_ok=True)
            created_dirs += 1

    for f in FILES_TO_TOUCH:
        path = root / f
        if not path.parent.exists():
            path.parent.mkdir(parents=True, exist_ok=True)
        if not path.exists():
            path.touch()
            created_files += 1
        else:
            skipped_files += 1

    print(f"Scaffolding complete.")
    print(f"  Directories created: {created_dirs}")
    print(f"  Files created:      {created_files}")
    print(f"  Files existing:     {skipped_files}")

if __name__ == "__main__":
    main()

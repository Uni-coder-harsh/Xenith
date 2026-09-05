# XENITH C++20 Coding Standards

XENITH is authored in modern ISO C++20. All code must conform to high standards of memory safety, performance, and clarity.

---

## 🛠️ Language Standard & Guidelines

- **Language Standard**: Modern C++20 (`-std=c++20`).
- **Standard Library Primitives**: Prefer standard containers (`std::vector`, `std::span`, `std::optional`, `std::variant`) over custom memory ownership wrappers.
- **Resource Management (RAII)**: Raw pointers (`T*`) are strictly non-owning. Use `std::unique_ptr` for exclusive ownership and standard RAII containers for dynamic array management.

---

## 🏷️ Naming Conventions

| Construct | Convention | Example |
| :--- | :--- | :--- |
| **Classes / Structs** | PascalCase | `CanonicalModel`, `BasisManager` |
| **Functions / Methods** | camelCase | `addVariable()`, `computeFactorization()` |
| **Variables** | snake_case | `num_variables`, `pivot_row` |
| **Member Variables** | `m_` prefix + snake_case | `m_numVariables`, `m_sparseMatrix` |
| **Constants / Enums** | UPPER_CASE | `MAX_PIVOT_TOLERANCE`, `OPTIMAL` |
| **Namespaces** | lowercase | `xenith::model`, `xenith::numerics` |

---

## 🚫 Prohibited Practices

1. **No Silent Exception Swallowing**: Catching exceptions in empty catch blocks is forbidden.
2. **No Global State**: Global variables or static non-const state are disallowed.
3. **No Non-Const Raw Reference Mutation Across Subsystems**: Prefer explicit return types or non-null pointers for out-parameters.
4. **No Raw Pointer Ownership**: Use standard smart pointers or containers.

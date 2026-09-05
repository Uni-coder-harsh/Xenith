# ADR-003: Central Canonical Mathematical Model

- **Status**: Accepted
- **Date**: 2026-09-05

---

## 📌 Context
Multiple input formats (MPS, LP files, Python APIs, C++ APIs) can feed mathematical models into the solver. Letting algorithms couple directly to specific file format data structures causes architectural fragility and prevents reusable model processing.

## 💡 Decision
We establish a **Central Canonical Model** (`xenith::model::CanonicalModel`) as the single authoritative internal intermediate representation. All input parsers must output a `CanonicalModel`, and all presolve/solver engines must consume a `CanonicalModel`.

## ⚖️ Consequences
### Positive:
- Total decoupling of input readers from solver algorithms.
- Unified model validation, checking, and presolve transformations.
- Clean foundational isolation.

### Negative:
- Single intermediate conversion step required during file loading.

## 🔄 Alternatives Considered
- Direct solver execution from parsed file structs (Monolithic coupling, rejected).

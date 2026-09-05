# ADR-008: MPS Format as Initial Input and Interchange Interface

- **Status**: Accepted
- **Date**: 2026-09-05

---

## 📌 Context
To validate XENITH against industry benchmark problems (e.g., Netlib), the solver must ingest standard mathematical model files.

## 💡 Decision
We select **MPS (Mathematical Programming System) format** as the first standardized input reader interface to be implemented for XENITH.

## ⚖️ Consequences
### Positive:
- Direct support for Netlib LP and MIPLIB benchmark test sets.
- Universally supported interchange format across optimization tools.

### Negative:
- Text parsing overhead; fixed-field legacy MPS rules require careful handling.

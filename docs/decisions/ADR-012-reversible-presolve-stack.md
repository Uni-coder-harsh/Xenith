# ADR-012: Reversible Stack-Based Presolve Architecture

- **Status**: Accepted
- **Date**: 2026-09-05

---

## 📌 Context
Presolve transformations reduce problem size prior to solve entry. If presolve mutations overwrite the original model in a one-way destructive fashion, exact primal and dual postsolve reconstruction becomes impossible or error-prone.

## 💡 Decision
We mandate a **Reversible Stack-Based Presolve Architecture** (`xenith::presolve::ReversiblePresolveStack`). Presolve steps push atomic reversible transformation objects onto a stack; Postsolve unwinds the stack in reverse LIFO order to map reduced solutions back to original model space.

## ⚖️ Consequences
### Positive:
- Guarantees exact mathematical postsolve reconstruction ($x_{\text{reduced}}^* \to x_{\text{original}}^*$).
- Preserves original `CanonicalModel` immutability.
- Simplifies debugging of individual presolve rules.

### Negative:
- Memory overhead to retain the presolve transformation stack during solve execution (negligible compared to solver benefit).

# MPS Test Fixtures

This directory stores small, deterministic test fixtures used to verify `xenith::io::mps::MpsReader`.

---

## 📄 Fixtures Index

- `minimal_lp.mps`: Standard 2x2 LP problem (Fixed Format).
- `equality_row.mps`: Tests equality constraint (`E` row).
- `greater_than_row.mps`: Tests greater-than constraint (`G` row).
- `bound_types.mps`: Tests `LO`, `UP`, `FX`, `FR`, `MI`, `PL`, `BV` bound cards.
- `integer_markers.mps`: Tests MPS integer block markers (`'MARK0000'`, `'INTORG'`, `'INTEND'`).
- `duplicate_coeffs.mps`: Tests coefficient entry accumulation.
- `free_format.mps`: Tests space-delimited free format parsing.
- `malformed_missing_rows.mps`: Malformed MPS file missing required `ROWS` section.

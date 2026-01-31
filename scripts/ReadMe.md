## `scripts/` notes (dev tooling)

This folder contains local developer helpers (not production code).

### Expectations
- Scripts should be safe to run multiple times (idempotent where practical).
- Prefer `set -e` and clear error messages for missing prerequisites.

### Canonical entrypoint
- `scripts/dev.sh` is the default helper for `build`, `run`, and `clean`.

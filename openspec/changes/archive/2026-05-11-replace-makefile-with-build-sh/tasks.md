## 1. OpenSpec Tracking

- [x] 1.1 Create proposal, design, spec, and tasks for replacing `Makefile` with `build.sh`.

## 2. Build Script

- [x] 2.1 Add executable `build.sh` with default help output and command dispatch.
- [x] 2.2 Port active firmware build behavior to `./build.sh firmware`.
- [x] 2.3 Port active VPI bridge build behavior to `./build.sh vpi`.
- [x] 2.4 Port active simulation launch behavior to `./build.sh sim`.
- [x] 2.5 Add `./build.sh all` and `./build.sh clean`.
- [x] 2.6 Preserve active environment override behavior.

## 3. Documentation and Removal

- [x] 3.1 Update `README.md`, `docs/arch.md`, and `TODO.md` to use `build.sh` commands.
- [x] 3.2 Remove `Makefile` after `build.sh` passes the current case.

## 4. Verification

- [x] 4.1 Run `./build.sh` and confirm help exits successfully.
- [x] 4.2 Run `git diff --check`.
- [x] 4.3 Run `./build.sh clean && ./build.sh all` with required modules loaded and confirm zero compare failures.
- [x] 4.4 Confirm active outputs are generated under `build/firmware` and `build/src`.
- [x] 4.5 Review OpenSpec status and final diff.

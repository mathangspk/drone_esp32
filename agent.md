# C++ Programming Standards and Agent Guidelines

This document governs all C++ development within this codebase. All modifications, architectural designs, and implementation steps must strictly adhere to these principles.

---

## 1. Build and Environment
- **Toolchain**: PlatformIO is the official build system.
- **Environments**:
  - `esp32dev`: Target environment for real ESP32-WROOM hardware.
  - `native`: Target environment for running simulations and unit tests on the host OS (Windows).
- All platform-dependent code must be encapsulated behind platform-independent abstract interfaces.

## 2. Coding Standards
- **Standard**: Modern C++17.
- **Naming Conventions**:
  - Classes and Structs: `PascalCase` (e.g., `PIDController`).
  - Methods and Functions: `camelCase` (e.g., `calculateOutput`) or `snake_case`. Let's stick to `camelCase` for clean OOP interfaces, or `snake_case` for hardware helpers if matching Arduino styles. Let's use `camelCase` consistently for OOP classes.
  - Variables and parameters: `camelCase` (e.g., `motorInput1`).
  - Constants: `UPPER_CASE_WITH_UNDERSCORES` (e.g., `PWM_FREQUENCY`).
  - Interfaces: Prefix with `I` (e.g., `IIMU`).
- Avoid global variables and global state. Encapsulate all logic within classes.

## 3. File Size Constraints (Strict Rule)
- **Absolutely no C++ header (.h) or source (.cpp) file may exceed 100 lines of code.**
- If a file approaches 100 lines, you must immediately refactor it by:
  - Splitting helper functions into separate utility classes.
  - Breaking down complex classes into smaller, single-responsibility objects.
  - Keeping implementations focused and highly modular.
- Smaller files improve code readability, simplify testing, and guarantee decoupled designs.

## 4. Documentation & Language
- **English Only**: The entire codebase (variable names, functions, classes), comments, and docstrings must be written 100% in English.
- Comments should explain **why** something is done, not **what** is done (which is visible in the code itself).

## 5. Memory Management & Safety
- Prefer stack allocation and pass-by-reference-to-const where possible.
- If dynamic memory is required, use smart pointers (`std::unique_ptr`, `std::shared_ptr`) rather than raw `new` and `delete`.
- Strictly avoid raw pointers unless necessary for interface hooks (e.g. mock objects). Check all pointers for `nullptr` before dereferencing.

## 6. Error & Failsafe Handling
- Flight stability relies on continuous operation. Never use raw `throw` on embedded targets.
- Use robust state indicators, failsafe flags, or standard C++ return patterns (e.g. `std::optional` or boolean status returns) to notify calling layers of sensor or receiver failures.

## 7. Test-Driven Development (TDD Workflow)
- **Core Principle**: AI must not implement new feature code unless the corresponding Unit Test has been written first and approved by the user.
- **Testing Framework**: `doctest` is used for lightweight, fast host-based testing.
- **Location**: All TDD test files must reside in the `tests/tdd/` folder.
- **TDD Steps**:
  1. Write the test cases showing target requirements and edge cases inside `tests/tdd/`.
  2. Present the tests to the user for logical and criteria approval.
  3. Upon approval, implement the minimal class logic to pass the test cases.
  4. Run PlatformIO native tests to verify:
     ```bash
     pio test -e native
     ```

## 8. Architecture and Handoff Synchronizations
- Maintain `architecture.md` containing tree structure and Mermaid graphs showing active components and relationships.
- Update `handoff.md` at the root of the workspace upon completing any functional milestone.

## 9. Karpathy Core Principles
1. **Think Before Coding**: Avoid assumptions. Clearly state tradeoffs.
2. **Simplicity First**: Write minimal code. No premature abstractions or over-engineering.
3. **Surgical Changes**: Touch only necessary lines. Do not reformat unrelated code.
4. **Goal-Driven Execution**: Deliver working, compilable code without console warnings or errors.

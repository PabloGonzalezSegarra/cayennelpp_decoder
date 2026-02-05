# Contributing to Cayene LPP Decoder

Thank you for your interest in contributing to this project! This document provides guidelines and instructions for contributing.

## 🌟 Ways to Contribute

- Report bugs
- Suggest new features
- Improve documentation
- Submit bug fixes
- Add new sensor types
- Write tests
- Review pull requests

## 🐛 Reporting Bugs

Before creating a bug report:
1. Check existing issues to avoid duplicates
2. Use the latest version of the code
3. Verify the bug is reproducible

Use the **Bug Report** template when creating an issue. Include:
- Clear description of the bug
- Steps to reproduce
- Expected vs actual behavior
- Environment details (OS, compiler, CMake version)
- Relevant log output

## 💡 Suggesting Features

Use the **Feature Request** template when proposing new features. Include:
- Problem you're trying to solve
- Proposed solution
- Alternative solutions considered
- Additional context

## 🔨 Pull Request Process

### 1. Fork and Clone

```bash
# Fork the repository on GitHub, then:
git clone https://github.com/YOUR_USERNAME/cayennelpp_decoder.git
cd cayennelpp_decoder
```

### 2. Create a Branch

```bash
git checkout -b feature/my-new-feature
# or
git checkout -b fix/my-bug-fix
```

Branch naming conventions:
- `feature/` - New features
- `fix/` - Bug fixes
- `docs/` - Documentation updates
- `refactor/` - Code refactoring
- `test/` - Test additions/updates

### 3. Make Your Changes

Follow the existing code style:
- Use C++20 features where appropriate
- Follow existing naming conventions
- Add comments for complex logic
- Update documentation as needed

### 4. Format Your Code

```bash
./cayene.sh format
# or manually:
# find src include tests examples -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i
```

### 5. Test Your Changes

```bash
./cayene.sh test
# or manually:
# cmake --preset release
# cmake --build build/release
# ctest --test-dir build/release --output-on-failure
```

### 6. Commit Your Changes

Write clear commit messages:
```bash
git add .
git commit -m "Add support for custom sensor type XYZ"
```

Commit message guidelines:
- Use present tense ("Add feature" not "Added feature")
- Use imperative mood ("Move cursor to..." not "Moves cursor to...")
- Limit first line to 72 characters
- Reference issues: "Fixes #123" or "Relates to #456"

### 7. Push and Create Pull Request

```bash
git push origin feature/my-new-feature
```

Then create a Pull Request on GitHub:
- Fill out the PR template completely
- Link to related issues
- Describe what changed and why
- Mark the checklist items as complete

### 8. Code Review

Your PR will be reviewed by maintainers. They may:
- Request changes
- Ask questions
- Suggest improvements
- Approve and merge

**Important**: All PRs require approval from the repository owner before merging.

## ✅ Pull Request Requirements

Before your PR can be merged:

- [ ] All CI checks pass (build, tests, formatting)
- [ ] Code follows project style guidelines
- [ ] Tests added for new features
- [ ] Documentation updated if needed
- [ ] No merge conflicts with main branch
- [ ] At least one approval from repository owner
- [ ] All review conversations resolved

## 🧪 Testing Guidelines

### Writing Tests

- Add tests in `tests/decoder_test.cpp`
- Use Google Test framework
- Test both success and error cases
- Aim for good coverage of new code

Example:
```cpp
TEST(DecoderTest, DecodeCustomType) {
    cayene::Decoder decoder;
    // Your test code here
}
```

### Running Tests

```bash
# All tests
./cayene.sh test

# Specific test
cd build/release
./tests/decoder_test --gtest_filter=DecoderTest.DecodeCustomType
```

## 📝 Documentation Guidelines

- Update README.md if adding features
- Add inline comments for complex code
- Update API documentation
- Include code examples where helpful

## 🎯 Code Style

This project uses:
- **clang-format** for code formatting
- **clang-tidy** for static analysis

Configuration files:
- `.clang-format` - Formatting rules
- `.clang-tidy` - Linting rules

Format before committing:
```bash
./cayene.sh format
```

## 🔒 Branch Protection

The `main` branch is protected:
- Direct pushes are not allowed
- All changes must go through pull requests
- PRs require approval from repository owner
- All status checks must pass
- Conversations must be resolved
- Force pushes are blocked

## 📜 License

By contributing, you agree that your contributions will be licensed under the GNU General Public License v2 (GPLv2).

## ❓ Questions?

- Open a [Discussion](https://github.com/PabloGonzalezSegarra/cayennelpp_decoder/discussions)
- Review existing [Issues](https://github.com/PabloGonzalezSegarra/cayennelpp_decoder/issues)

## 🙏 Thank You!

Your contributions help make this project better for everyone!

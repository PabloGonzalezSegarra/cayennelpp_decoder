# Security Policy

## Supported Versions

We release patches for security vulnerabilities. Which versions are eligible for receiving such patches depends on the CVSS v3.0 Rating:

| Version | Supported          |
| ------- | ------------------ |
| main    | :white_check_mark: |

## Reporting a Vulnerability

If you discover a security vulnerability, please report it by emailing the repository owner. Please do not open public issues for security vulnerabilities.

Please include the following information in your report:
- Type of issue (e.g., buffer overflow, SQL injection, cross-site scripting, etc.)
- Full paths of source file(s) related to the manifestation of the issue
- The location of the affected source code (tag/branch/commit or direct URL)
- Any special configuration required to reproduce the issue
- Step-by-step instructions to reproduce the issue
- Proof-of-concept or exploit code (if possible)
- Impact of the issue, including how an attacker might exploit the issue

We will acknowledge your email within 48 hours and will send a more detailed response within 96 hours indicating the next steps in handling your report.

## Security Features

This repository has the following security features enabled:
- **Dependabot Alerts**: Automated vulnerability scanning for dependencies
- **Branch Protection**: Main branch is protected against direct pushes
- **Code Scanning**: Automated security analysis of code changes

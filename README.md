<!--
SPDX-FileCopyrightText: 2024 Davide Paro <dparo@outlook.it>, et al.
SPDX-FileContributor: Davide Paro <dparo@outlook.it>

SPDX-License-Identifier: Apache-2.0
-->

![GitHub release (latest by date)](https://img.shields.io/github/v/release/dparo/chaddy?style=for-the-badge)
![GitHub](https://img.shields.io/github/license/dparo/chaddy?style=for-the-badge)
![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/dparo/chaddy/ci.yml?branch=master&style=for-the-badge)

# Chaddy: A uFE/BE stack in C11

The project is still under development.

## TODO

### Code Architecture

- [ ] Integration with Linux `io_uring` for handling async HTTP requests
- [ ] PostgreSQL async integration trough `io_uring`
- [ ] CURL async integration trough `io_uring`
- [ ] HTTP requests parsing trough `llhttp` (streaming HTTP parsing library)
- [ ] Thread pool for serving application layer login in each requests
- [ ] `minicoro` library integration for freeing thread execution to do other work when waiting for IO.

### Features

- [ ] Implement Proper memory allocation strategies
- [ ] HTTP Router implementation (employ code generation from a specification .yml file ?)
  - [ ] File based router?
- [ ] Mounts/VFS support. Ability to serve static contents trough Native FileSystem, Virtual Mounts, Embedded files (in the data section of the ELF)
- [ ] JSON parser for handling JSON requests
- [ ] Middleware support for blocking/handling/modifying requests.
- [ ] RestController + Controller support


#!/usr/bin/env bash

# SPDX-FileCopyrightText: 2024 Davide Paro <dparo@outlook.it>, et al.
# SPDX-FileContributor: Davide Paro <dparo@outlook.it>
#
# SPDX-License-Identifier: Apache-2.0

# -*- coding: utf-8 -*-

set -eou pipefail
set -x

pipx run reuse annotate --license=Apache-2.0 \
    --copyright="Davide Paro <dparo@outlook.it>, et al." \
    --contributor="$(git config user.name) <$(git config user.email)>" \
    --merge-copyrights \
    --recursive \
    --skip-unrecognised \
    "$@"


# SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
# SPDX-License-Identifier: LGPL-2.1-only
cmake_minimum_required(VERSION 3.21)
project(MagicDoodleBoardConsumer LANGUAGES C)
find_package(Board CONFIG REQUIRED)
find_package(Magic CONFIG REQUIRED)
find_package(Doodle CONFIG REQUIRED)
add_executable(mdb_consumer main.c)
target_link_libraries(mdb_consumer PRIVATE Board::Board Magic::Magic Doodle::Doodle)

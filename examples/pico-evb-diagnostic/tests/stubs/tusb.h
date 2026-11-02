// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
bool tusb_init();
void tud_task_ext(std::uint32_t, bool);
bool tud_cdc_connected();
void tud_cdc_read_flush();
void tud_cdc_write_clear();
std::uint32_t tud_cdc_write_flush();
std::uint32_t tud_cdc_write_available();
std::uint32_t tud_cdc_read(void *, std::uint32_t);
std::uint32_t tud_cdc_write(void const *, std::uint32_t);

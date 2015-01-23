/*
 * Copyright (C) 2015 Simon Aittamaa
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UNICODE_H_
#define UNICODE_H_

#include <stdint.h>
#include <stddef.h>

int utf8_trailing_bytes(uint8_t);
int utf8_to_ucs4_can_convert(const uint8_t *, size_t);
size_t utf8_to_ucs4(const uint8_t *, size_t, uint32_t *);
size_t ucs4_to_utf8(uint32_t, uint8_t *, size_t);
size_t utf8_to_ucs2(const uint8_t *, size_t, uint16_t *);
size_t ucs2_to_utf8(uint16_t, uint8_t *, size_t);
size_t utf16_to_ucs4(uint16_t *, size_t, uint32_t *);

#endif

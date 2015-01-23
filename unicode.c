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

#include "unicode.h"

int utf8_trailing_bytes(uint8_t c) {
        int cnt = 0;
        while (c & 0x80) {
                c <<= 1;
                cnt++;
        }
        return cnt?cnt-1:cnt;
}

int utf8_to_ucs4_can_convert(const uint8_t *from, size_t n) {
        return utf8_trailing_bytes(*from) < n;
}

size_t utf8_to_ucs4(const uint8_t *from, size_t n, uint32_t *to) {
        size_t trailing_bytes = utf8_trailing_bytes(*from);
        uint32_t converted;
        if (trailing_bytes >= n) {
                return 0;
        } else if (trailing_bytes == 0) {
                converted = (uint32_t)from[0];
        } else if (trailing_bytes == 1) {
                converted  = ((uint32_t)from[0] & 0x1f) << 6;
                converted |= ((uint32_t)from[1] & 0x3f);
        } else if (trailing_bytes == 2) {
                converted  = ((uint32_t)from[0] & 0x0f) << 12;
                converted |= ((uint32_t)from[1] & 0x3f) << 6;
                converted |= ((uint32_t)from[2] & 0x3f);
        } else if (trailing_bytes == 3) {
                converted  = ((uint32_t)from[0] & 0x07) << 18;
                converted |= ((uint32_t)from[1] & 0x3f) << 12;
                converted |= ((uint32_t)from[2] & 0x3f) << 6;
                converted |= ((uint32_t)from[3] & 0x3f);
        } else {
                return 0;
        }

        *to = converted;
        return trailing_bytes + 1;
}

size_t ucs4_to_utf8(uint32_t from, uint8_t *to, size_t n) {
        if (from <= 0x7f) {
                if (n < 1) {
                        return 0;
                }
                to[0] = (uint8_t)from;
                return 1;
        } else if (from <= 0x7ff) {
                if (n < 2) {
                        return 0;
                }
                to[0] = (from >> 6) | 0xc0;
                to[1] = (from & 0x3f) | 0x80;
                return 2;
        } else if (from <= 0xffff) {
                if (n < 3) {
                        return 0;
                }
                to[0] = (from >> 12) | 0xe0;
                to[1] = ((from >> 6) & 0x3f) | 0x80;
                to[2] = (from & 0x3f) | 0x80;
                return 3;
        } else if (from <= 0x1fffff) {
                if (n < 4) {
                        return 0;
                }
                to[0] = (from >> 18) | 0xf0;
                to[1] = ((from >> 12) & 0x3f) | 0x80;
                to[2] = ((from >> 6) & 0x3f) | 0x80;
                to[3] = (from & 0x3f) | 0x80;
                return 4;
        } else {
                return 0;
        }
}

size_t utf8_to_ucs2(const uint8_t *from, size_t n, uint16_t *to) {
        uint32_t ucs4;
        size_t len = utf8_to_ucs4(from, n, &ucs4);
        if (ucs4 > 0xffff) {
                return 0;
        }

        *to = (uint16_t)ucs4;
        return len;
}

size_t ucs2_to_utf8(uint16_t from, uint8_t *to, size_t n) {
        return ucs4_to_utf8(from, to, n);
}

size_t utf16_to_ucs4(uint16_t *from, size_t n, uint32_t *to) {
        uint32_t converted;
        size_t len;
        if ((from[0] & 0xdc00) == 0xdc00) {
                return 0;
        } else if ((from[0] & 0xd800) == 0xd800) {
                if (n < 2) {
                        return 0;
                }
                if ((from[1] & 0xdc00) != 0xdc00) {
                        return 0;
                } else {
                        converted = 0x10000;
                        converted |= ((uint32_t)(from[0] & ~0xd800)) << 10;
                        converted |= (uint32_t)(from[1] & ~0xdc00);
                        len = 2;
                }
        } else{
                if (n < 1) {
                        return 0;
                }
                converted = (uint32_t)from[0];
                len = 1;
        }

        *to = converted;
        return len;
}

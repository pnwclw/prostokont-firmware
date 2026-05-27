/**
 * @file GraphicsDefs.h
 * @author Fran Fodor for Soldered
 * @brief LUTs for graphics.
 *
 * https://github.com/SolderedElectronics/Display-Esp-library
 * For more info about the product, please check:
 * https://docs.soldered.com/inkplate/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "sdkconfig.h"

#if defined(CONFIG_INKPLATE_BOARD_INKPLATE2) ||                                \
    defined(CONFIG_INKPLATE_BOARD_INKPLATE6COLOR) ||                           \
    defined(CONFIG_INKPLATE_BOARD_INKPLATE13) ||                               \
    defined(CONFIG_WAVESHARE_BOARD_WAVESHARE13)
#define BLACK 0
#define WHITE 1
#else
#define BLACK 1
#define WHITE 0
#endif

#include <stdint.h>

static const uint8_t LUT2[16] = {0xAA, 0xA9, 0xA6, 0xA5, 0x9A, 0x99,
                                 0x96, 0x95, 0x6A, 0x69, 0x66, 0x65,
                                 0x5A, 0x59, 0x56, 0x55};
static const uint8_t LUTW[16] = {0xFF, 0xFE, 0xFB, 0xFA, 0xEF, 0xEE,
                                 0xEB, 0xEA, 0xBF, 0xBE, 0xBB, 0xBA,
                                 0xAF, 0xAE, 0xAB, 0xAA};
static const uint8_t LUTB[16] = {0xFF, 0xFD, 0xF7, 0xF5, 0xDF, 0xDD,
                                 0xD7, 0xD5, 0x7F, 0x7D, 0x77, 0x75,
                                 0x5F, 0x5D, 0x57, 0x55};

static const uint8_t pixelMaskLUT[8] = {0x1,  0x2,  0x4,  0x8,
                                        0x10, 0x20, 0x40, 0x80};
static const uint8_t pixelMaskGLUT[2] = {0xF, 0xF0};

static const uint8_t discharge[16] = {0xFF, 0xFC, 0xF3, 0xF0, 0xCF, 0xCC,
                                      0xC3, 0xC0, 0x3F, 0x3C, 0x33, 0x30,
                                      0x0F, 0x0C, 0x03, 0x00};

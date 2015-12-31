/*
 * emdViewer, a program for working with electron microscopy dataset 
 * (emd) files.
 * Copyright (C) 2015  Phil Ophus
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef EMD_STYLECONFIG_H
#define EMD_STYLECONFIG_H

#define STRINGIFY2(a) #a
#define STRINGIFY(a) STRINGIFY2(a)
#define _S(a) STRINGIFY2(a)

#define COLOUR_HIGHLIGHT_1 (220, 240, 255)
#define COLOUR_BUTTON_PRESSED (34, 85, 221)

#define PRIMARY_COLOUR "#2240B5"
#define PRIMARY_HIGHLIGHT "#DCF0FF"
#define PRIMARY_SHADOW "#113377"

#define LIGHT_GRAY "#E0E0E0"
#define DARK_GRAY "202020"

#define BTN_BG_UNCHK_CLR "white"
#define BTN_BG_UNCHK_HVR_CLR PRIMARY_HIGHLIGHT
#define BTN_BG_CHK_CLR PRIMARY_COLOUR
#define BTN_BG_DSBLD_CLR LIGHT_GRAY
#define BTN_TXT_DSBLD_CLR DARK_GRAY

#define WDGT_BG_CLR "white"

#endif

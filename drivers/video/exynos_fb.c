/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * Author: InKi Dae <inki.dae@samsung.com>
 * Author: Donghwa Lee <dh09.lee@samsung.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <common.h>
#include <lcd.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>
#include <asm/arch/clk.h>
#include <asm/arch/mipi_dsim.h>
#include <asm/arch/dp_info.h>
#include <asm/arch/system.h>

#include "exynos_fb.h"

DECLARE_GLOBAL_DATA_PTR;

static unsigned int panel_width, panel_height;

static void exynos_lcd_init_mem(void *lcdbase, vidinfo_t *vid)
{
	unsigned long palette_size;
	unsigned int fb_size;

	fb_size = vid->vl_row * vid->vl_col * (NBITS(vid->vl_bpix) >> 3);

	palette_size = NBITS(vid->vl_bpix) == 8 ? 256 : 16;

	exynos_fimd_lcd_init_mem((unsigned long)lcdbase,
			(unsigned long)fb_size, palette_size);
}

static void exynos_lcd_init(vidinfo_t *vid)
{
	exynos_fimd_lcd_init(vid);

	/* Enable flushing after LCD writes if requested */
	lcd_set_flush_dcache(1);
}

#ifdef CONFIG_CMD_BMP
static void draw_logo(void)
{
	int x, y;
	ulong addr;

	if (panel_width >= panel_info.logo_width) {
		x = ((panel_width - panel_info.logo_width) >> 1);
	} else {
		x = 0;
		printf("Warning: image width is bigger than display width\n");
	}

	if (panel_height >= panel_info.logo_height) {
		y = ((panel_height - panel_info.logo_height) >> 1) - 4;
	} else {
		y = 0;
		printf("Warning: image height is bigger than display height\n");
	}

	addr = panel_info.logo_addr;
	bmp_display(addr, x, y);
}
#endif

static void lcd_panel_on(vidinfo_t *vid)
{
	udelay(vid->init_delay);

	if (vid->backlight_reset)
		vid->backlight_reset();

	if (vid->cfg_gpio)
		vid->cfg_gpio();

	if (vid->lcd_power_on)
		vid->lcd_power_on();

	udelay(vid->power_on_delay);

	if (vid->dp_enabled)
		exynos_init_dp();

	if (vid->reset_lcd) {
		vid->reset_lcd();
		udelay(vid->reset_delay);
	}

	if (vid->backlight_on)
		vid->backlight_on(1);

	if (vid->cfg_ldo)
		vid->cfg_ldo();

	if (vid->enable_ldo)
		vid->enable_ldo(1);

	if (vid->mipi_enabled)
		exynos_mipi_dsi_init();
}

void lcd_ctrl_init(void *lcdbase)
{
	set_system_display_ctrl();
	set_lcd_clk();

	/* initialize parameters which is specific to panel. */
	init_panel_info(&panel_info);

	panel_width = panel_info.vl_width;
	panel_height = panel_info.vl_height;

	exynos_lcd_init_mem(lcdbase, &panel_info);

	exynos_lcd_init(&panel_info);
}

void lcd_enable(void)
{
	if (panel_info.logo_on) {
		memset((void *) gd->fb_base, 0, panel_width * panel_height *
				(NBITS(panel_info.vl_bpix) >> 3));
#ifdef CONFIG_CMD_BMP
		draw_logo();
#endif
	}

	lcd_panel_on(&panel_info);
}

/* dummy function */
void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue)
{
	return;
}

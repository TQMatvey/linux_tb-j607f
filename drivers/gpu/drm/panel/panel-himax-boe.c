// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2023 TQMatvey

#include <linux/backlight.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/of.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>

struct himax_boe {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct gpio_desc *reset_gpio;
	bool prepared;
};

static inline struct himax_boe *to_himax_boe(struct drm_panel *panel)
{
	return container_of(panel, struct himax_boe, panel);
}

static void himax_boe_reset(struct himax_boe *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	msleep(20);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	msleep(20);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	msleep(50);
}

static int himax_boe_on(struct himax_boe *ctx)
{
	struct mipi_dsi_device *dsi = ctx->dsi;
	struct device *dev = &dsi->dev;
	int ret;

	mipi_dsi_dcs_write_seq(dsi, 0xb9, 0x83, 0x10, 0x2e);
	mipi_dsi_dcs_write_seq(dsi, 0xe9, 0xcd);
	mipi_dsi_dcs_write_seq(dsi, 0xbb, 0x01);
	usleep_range(5000, 6000);
	mipi_dsi_dcs_write_seq(dsi, 0xe9, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0xd1, 0x67, 0x0c, 0xff, 0x05);
	mipi_dsi_dcs_write_seq(dsi, 0xb1,
			       0x10, 0xfa, 0xaf, 0xaf, 0x2b, 0x2b, 0xb2, 0x57,
			       0x4d, 0x36, 0x36, 0x36, 0x36, 0x22, 0x21, 0x15,
			       0x00);
	mipi_dsi_dcs_write_seq(dsi, 0xb2,
			       0x00, 0xb0, 0x47, 0xd0, 0x00, 0x2c, 0x50, 0x2c,
			       0x00, 0x00, 0x00, 0x00, 0x15, 0x20, 0xd7, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0xb4,
			       0x70, 0x60, 0x01, 0x01, 0x80, 0x67, 0x00, 0x00,
			       0x01, 0x9b, 0x01, 0x58, 0x00, 0xff, 0x00, 0xff);
	mipi_dsi_dcs_write_seq(dsi, 0xbf, 0xfc, 0x85, 0x80);
	mipi_dsi_dcs_write_seq(dsi, 0xd2, 0x2b, 0x2b);
	mipi_dsi_dcs_write_seq(dsi, 0xd3,
			       0x00, 0x00, 0x00, 0x00, 0x78, 0x04, 0x00, 0x04,
			       0x00, 0x27, 0x00, 0x64, 0x4f, 0x2d, 0x2d, 0x00,
			       0x00, 0x32, 0x10, 0x27, 0x00, 0x27, 0x32, 0x10,
			       0x23, 0x00, 0x23, 0x32, 0x18, 0x03, 0x08, 0x03,
			       0x00, 0x00, 0x20, 0x30, 0x01, 0x55, 0x21, 0x2e,
			       0x01, 0x55, 0x0f);
	usleep_range(5000, 6000);
	mipi_dsi_dcs_write_seq(dsi, 0xe0,
			       0x00, 0x06, 0x11, 0x19, 0x21, 0x3b, 0x53, 0x5a,
			       0x60, 0x5b, 0x74, 0x78, 0x7d, 0x8a, 0x87, 0x8f,
			       0x98, 0xaa, 0xaa, 0x54, 0x5b, 0x66, 0x70, 0x00,
			       0x06, 0x11, 0x19, 0x21, 0x3b, 0x53, 0x5a, 0x60,
			       0x5b, 0x74, 0x78, 0x7d, 0x8a, 0x87, 0x8f, 0x98,
			       0xaa, 0xaa, 0x54, 0x5b, 0x66, 0x70);
	usleep_range(5000, 6000);
	mipi_dsi_dcs_write_seq(dsi, 0xbd, 0x01);
	mipi_dsi_dcs_write_seq(dsi, 0xb1, 0x01, 0x9b, 0x01, 0x31);
	mipi_dsi_dcs_write_seq(dsi, 0xcb,
			       0xf4, 0x36, 0x12, 0x16, 0xc0, 0x28, 0x6c, 0x85,
			       0x3f, 0x04);
	mipi_dsi_dcs_write_seq(dsi, 0xd3,
			       0x01, 0x00, 0xbc, 0x00, 0x00, 0x11, 0x10, 0x00,
			       0x0e, 0x00, 0x01);
	usleep_range(5000, 6000);
	mipi_dsi_dcs_write_seq(dsi, 0xbd, 0x02);
	mipi_dsi_dcs_write_seq(dsi, 0xb4, 0x4e, 0x00, 0x33, 0x11, 0x33, 0x88);
	mipi_dsi_dcs_write_seq(dsi, 0xbf, 0xf2, 0x00, 0x02);
	mipi_dsi_dcs_write_seq(dsi, 0xbd, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0xc0,
			       0x23, 0x23, 0x22, 0x11, 0xa2, 0x17, 0x00, 0x80,
			       0x00, 0x00, 0x08, 0x00, 0x63, 0x63);
	mipi_dsi_dcs_write_seq(dsi, 0xc6, 0xf9);
	mipi_dsi_dcs_write_seq(dsi, 0xc7, 0x30);
	mipi_dsi_dcs_write_seq(dsi, 0xc8,
			       0x00, 0x04, 0x04, 0x00, 0x00, 0x85, 0x43, 0xff);
	mipi_dsi_dcs_write_seq(dsi, 0xd0, 0x07, 0x04, 0x05);
	mipi_dsi_dcs_write_seq(dsi, 0xd5,
			       0x18, 0x18, 0x18, 0x18, 0x1a, 0x1a, 0x1a, 0x1a,
			       0x1b, 0x1b, 0x1b, 0x1b, 0x24, 0x24, 0x24, 0x24,
			       0x07, 0x06, 0x07, 0x06, 0x05, 0x04, 0x05, 0x04,
			       0x03, 0x02, 0x03, 0x02, 0x01, 0x00, 0x01, 0x00,
			       0x21, 0x20, 0x21, 0x20, 0x18, 0x18, 0x18, 0x18,
			       0x18, 0x18, 0x18, 0x18);
	usleep_range(5000, 6000);
	mipi_dsi_dcs_write_seq(dsi, 0xe7,
			       0x12, 0x13, 0x02, 0x02, 0x49, 0x49, 0x0e, 0x0e,
			       0x0f, 0x1a, 0x1d, 0x74, 0x28, 0x74, 0x01, 0x07,
			       0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 0x68);
	mipi_dsi_dcs_write_seq(dsi, 0xbd, 0x01);
	mipi_dsi_dcs_write_seq(dsi, 0xe7,
			       0x02, 0x38, 0x01, 0x93, 0x0d, 0xda, 0x0e);
	mipi_dsi_dcs_write_seq(dsi, 0xbd, 0x02);
	mipi_dsi_dcs_write_seq(dsi, 0xe7,
			       0xff, 0x01, 0xff, 0x01, 0x00, 0x00, 0x22, 0x00,
			       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			       0x81, 0x00, 0x02, 0x40);
	mipi_dsi_dcs_write_seq(dsi, 0xbd, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0xba,
			       0x70, 0x03, 0xa8, 0x83, 0xf2, 0x80, 0xc0, 0x0d);
	mipi_dsi_dcs_write_seq(dsi, 0xbd, 0x02);
	mipi_dsi_dcs_write_seq(dsi, 0xd8,
			       0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0xff, 0xff,
			       0xff, 0xff, 0xf0, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0xbd, 0x03);
	mipi_dsi_dcs_write_seq(dsi, 0xd8,
			       0xaa, 0xaa, 0xaa, 0xaa, 0xa0, 0x00, 0xaa, 0xaa,
			       0xaa, 0xaa, 0xa0, 0x00, 0x55, 0x55, 0x55, 0x55,
			       0x50, 0x00, 0x55, 0x55, 0x55, 0x55, 0x50, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0xbd, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0xe1, 0x01, 0x06);
	mipi_dsi_dcs_write_seq(dsi, 0xcc, 0x02);
	mipi_dsi_dcs_write_seq(dsi, 0xbd, 0x03);
	mipi_dsi_dcs_write_seq(dsi, 0xb2, 0x80);
	mipi_dsi_dcs_write_seq(dsi, 0xbd, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x51, 0x00, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x53, 0x2c);
	mipi_dsi_dcs_write_seq(dsi, 0xc9, 0x00, 0x0d, 0xf0, 0x00);

	ret = mipi_dsi_dcs_exit_sleep_mode(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to exit sleep mode: %d\n", ret);
		return ret;
	}
	msleep(60);

	mipi_dsi_dcs_write_seq(dsi, 0xb2,
			       0x00, 0xb0, 0x47, 0xd0, 0x00, 0x2c, 0x50, 0x2c,
			       0x00, 0x00, 0x00, 0x00, 0x15, 0x20, 0xd7, 0x00);

	ret = mipi_dsi_dcs_set_display_on(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to set display on: %d\n", ret);
		return ret;
	}
	msleep(20);

	return 0;
}

static int himax_boe_off(struct himax_boe *ctx)
{
	struct mipi_dsi_device *dsi = ctx->dsi;
	struct device *dev = &dsi->dev;
	int ret;

	ret = mipi_dsi_dcs_set_display_off(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to set display off: %d\n", ret);
		return ret;
	}
	msleep(30);

	ret = mipi_dsi_dcs_enter_sleep_mode(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to enter sleep mode: %d\n", ret);
		return ret;
	}
	msleep(60);

	return 0;
}

static int himax_boe_prepare(struct drm_panel *panel)
{
	struct himax_boe *ctx = to_himax_boe(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	if (ctx->prepared)
		return 0;

	himax_boe_reset(ctx);

	ret = himax_boe_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		return ret;
	}

	ctx->prepared = true;
	return 0;
}

static int himax_boe_unprepare(struct drm_panel *panel)
{
	struct himax_boe *ctx = to_himax_boe(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	if (!ctx->prepared)
		return 0;

	ret = himax_boe_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);

	ctx->prepared = false;
	return 0;
}

static const struct drm_display_mode himax_boe_mode = {
	.clock = (1200 + 20 + 8 + 16) * (2000 + 80 + 8 + 38) * 60 / 1000,
	.hdisplay = 1200,
	.hsync_start = 1200 + 20,
	.hsync_end = 1200 + 20 + 8,
	.htotal = 1200 + 20 + 8 + 16,
	.vdisplay = 2000,
	.vsync_start = 2000 + 80,
	.vsync_end = 2000 + 80 + 8,
	.vtotal = 2000 + 80 + 8 + 38,
	.width_mm = 0,
	.height_mm = 0,
};

static int himax_boe_get_modes(struct drm_panel *panel,
			       struct drm_connector *connector)
{
	struct drm_display_mode *mode;

	mode = drm_mode_duplicate(connector->dev, &himax_boe_mode);
	if (!mode)
		return -ENOMEM;

	drm_mode_set_name(mode);

	mode->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
	connector->display_info.width_mm = mode->width_mm;
	connector->display_info.height_mm = mode->height_mm;
	drm_mode_probed_add(connector, mode);

	return 1;
}

static const struct drm_panel_funcs himax_boe_panel_funcs = {
	.prepare = himax_boe_prepare,
	.unprepare = himax_boe_unprepare,
	.get_modes = himax_boe_get_modes,
};

static int himax_boe_bl_update_status(struct backlight_device *bl)
{
	struct mipi_dsi_device *dsi = bl_get_data(bl);
	u16 brightness = backlight_get_brightness(bl);
	int ret;

	dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_dcs_set_display_brightness_large(dsi, brightness);
	if (ret < 0)
		return ret;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	return 0;
}

// TODO: Check if /sys/class/backlight/.../actual_brightness actually returns
// correct values. If not, remove this function.
static int himax_boe_bl_get_brightness(struct backlight_device *bl)
{
	struct mipi_dsi_device *dsi = bl_get_data(bl);
	u16 brightness;
	int ret;

	dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_dcs_get_display_brightness_large(dsi, &brightness);
	if (ret < 0)
		return ret;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	return brightness;
}

static const struct backlight_ops himax_boe_bl_ops = {
	.update_status = himax_boe_bl_update_status,
	.get_brightness = himax_boe_bl_get_brightness,
};

static struct backlight_device *
himax_boe_create_backlight(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	const struct backlight_properties props = {
		.type = BACKLIGHT_RAW,
		.brightness = 4095,
		.max_brightness = 4095,
	};

	return devm_backlight_device_register(dev, dev_name(dev), dev, dsi,
					      &himax_boe_bl_ops, &props);
}

static int himax_boe_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct himax_boe *ctx;
	int ret;

	ctx = devm_kzalloc(dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(ctx->reset_gpio),
				     "Failed to get reset-gpios\n");

	ctx->dsi = dsi;
	mipi_dsi_set_drvdata(dsi, ctx);

	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_CLOCK_NON_CONTINUOUS |
			  MIPI_DSI_MODE_LPM;

	drm_panel_init(&ctx->panel, dev, &himax_boe_panel_funcs,
		       DRM_MODE_CONNECTOR_DSI);

	ctx->panel.backlight = himax_boe_create_backlight(dsi);
	if (IS_ERR(ctx->panel.backlight))
		return dev_err_probe(dev, PTR_ERR(ctx->panel.backlight),
				     "Failed to create backlight\n");

	drm_panel_add(&ctx->panel);

	ret = mipi_dsi_attach(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to attach to DSI host: %d\n", ret);
		drm_panel_remove(&ctx->panel);
		return ret;
	}

	return 0;
}

static void himax_boe_remove(struct mipi_dsi_device *dsi)
{
	struct himax_boe *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id himax_boe_of_match[] = {
	{ .compatible = "himax,boe" }, // FIXME
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, himax_boe_of_match);

static struct mipi_dsi_driver himax_boe_driver = {
	.probe = himax_boe_probe,
	.remove = himax_boe_remove,
	.driver = {
		.name = "panel-himax-boe",
		.of_match_table = himax_boe_of_match,
	},
};
module_mipi_dsi_driver(himax_boe_driver);

MODULE_AUTHOR("TQMatvey <maybeetube@gmail.com>");
MODULE_DESCRIPTION("DRM driver for himax boe video mode dsi panel");
MODULE_LICENSE("GPL");

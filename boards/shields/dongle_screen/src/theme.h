/* SPDX-License-Identifier: MIT */
#pragma once
#include <lvgl.h>
#define DASH_BG lv_color_hex(0x050505)
#define DASH_TEXT lv_color_hex(0xe6e6e6)
#define DASH_MUTED lv_color_hex(0x777777)
#define DASH_RED lv_color_hex(0xd71921)
#define DASH_INACTIVE lv_color_hex(0x333333)
static inline void dash_container(lv_obj_t *obj, int width, int height) {
    lv_obj_set_size(obj, width, height);
    lv_obj_set_style_bg_color(obj, DASH_BG, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_radius(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}

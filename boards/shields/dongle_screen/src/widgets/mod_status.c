/* SPDX-License-Identifier: MIT
 * YADS modifier widget, using its existing HID report source.
 */
#include <zephyr/kernel.h>
#include <zmk/hid.h>
#include <lvgl.h>
#include "mod_status.h"
#include "../theme.h"

static const uint8_t modifier_masks[] = {
    MOD_LCTL | MOD_RCTL, MOD_LALT | MOD_RALT,
    MOD_LGUI | MOD_RGUI, MOD_LSFT | MOD_RSFT,
};

static void update_mod_status(struct zmk_widget_mod_status *widget) {
    uint8_t mods = zmk_hid_get_keyboard_report()->body.modifiers;
    if (widget->initialized && mods == widget->previous_mods) { return; }
    widget->previous_mods = mods;
    widget->initialized = true;
    for (int i = 0; i < 4; i++) {
        lv_obj_set_style_text_color(widget->labels[i],
            (mods & modifier_masks[i]) ? DASH_RED : DASH_MUTED, 0);
    }
}

/* Run on the LVGL thread, not the upstream Zephyr timer interrupt. */
static void mod_status_timer_cb(lv_timer_t *timer) {
#if LVGL_VERSION_MAJOR >= 9
    update_mod_status(lv_timer_get_user_data(timer));
#else
    update_mod_status(timer->user_data);
#endif
}

int zmk_widget_mod_status_init(struct zmk_widget_mod_status *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    dash_container(widget->obj, 256, 18);
    static const char *const names[] = {"CTRL", "ALT", "GUI", "SHFT"};
    widget->initialized = false;
    for (int i = 0; i < 4; i++) {
        widget->labels[i] = lv_label_create(widget->obj);
        lv_obj_set_width(widget->labels[i], 64);
        lv_obj_set_style_text_font(widget->labels[i], &lv_font_unscii_8, 0);
        lv_obj_set_style_text_align(widget->labels[i], LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_text(widget->labels[i], names[i]);
        lv_obj_set_pos(widget->labels[i], i * 64, 4);
    }
    update_mod_status(widget);
    lv_timer_create(mod_status_timer_cb, 100, widget);
    return 0;
}

lv_obj_t *zmk_widget_mod_status_obj(struct zmk_widget_mod_status *widget) {
    return widget->obj;
}

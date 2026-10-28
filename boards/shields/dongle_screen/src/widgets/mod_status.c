/* SPDX-License-Identifier: MIT
 * YADS modifier widget, using its existing HID report source.
 */
#include <zephyr/kernel.h>
#include <stdio.h>
#include <zmk/hid.h>
#include <lvgl.h>
#include "mod_status.h"
#include "../theme.h"

static uint8_t previous_mods = 0xff;
static void update_mod_status(struct zmk_widget_mod_status *widget) {
    uint8_t mods = zmk_hid_get_keyboard_report()->body.modifiers;
    if (mods == previous_mods) { return; }
    previous_mods = mods;
    char text[100];
    snprintf(text, sizeof(text), "#%s CTRL# #%s ALT# #%s GUI# #%s SHFT#",
             (mods & (MOD_LCTL | MOD_RCTL)) ? "d71921" : "333333",
             (mods & (MOD_LALT | MOD_RALT)) ? "d71921" : "333333",
             (mods & (MOD_LGUI | MOD_RGUI)) ? "d71921" : "333333",
             (mods & (MOD_LSFT | MOD_RSFT)) ? "d71921" : "333333");
    lv_label_set_text(widget->label, text);
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
    widget->label = lv_label_create(widget->obj);
    lv_obj_align(widget->label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_recolor(widget->label, true);
    update_mod_status(widget);
    lv_timer_create(mod_status_timer_cb, 100, widget);
    return 0;
}

lv_obj_t *zmk_widget_mod_status_obj(struct zmk_widget_mod_status *widget) {
    return widget->obj;
}

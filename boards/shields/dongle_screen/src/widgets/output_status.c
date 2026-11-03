/*
 * Copyright (c) 2024 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/endpoint_changed.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/usb.h>
#include <zmk/ble.h>
#include <zmk/endpoints.h>
#include "output_status.h"
#include "../theme.h"

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct output_status_state {
    enum zmk_transport selected_transport;
    enum zmk_transport preferred_transport;
    int active_profile_index;
    bool ble_ready;
    bool usb_ready;
};

static struct output_status_state get_state(const zmk_event_t *_eh) {
    return (struct output_status_state){
        .selected_transport = zmk_endpoint_get_selected().transport,
        .preferred_transport = zmk_endpoint_get_preferred_transport(),
        .active_profile_index = zmk_ble_active_profile_index(),
        .ble_ready = zmk_ble_active_profile_is_connected(),
        .usb_ready = zmk_usb_is_hid_ready(),
    };
}

static bool state_equal(struct output_status_state a, struct output_status_state b) {
    return a.selected_transport == b.selected_transport &&
           a.preferred_transport == b.preferred_transport &&
           a.active_profile_index == b.active_profile_index &&
           a.ble_ready == b.ble_ready && a.usb_ready == b.usb_ready;
}

static void set_status_symbol(struct zmk_widget_output_status *widget,
                              struct output_status_state state) {
    const bool usb_active = state.selected_transport == ZMK_TRANSPORT_USB && state.usb_ready;
    const bool ble_active = state.selected_transport == ZMK_TRANSPORT_BLE && state.ble_ready;
    lv_obj_set_style_text_color(widget->transport_label, usb_active ? DASH_RED : DASH_MUTED, 0);
    lv_obj_set_style_text_color(widget->ble_label, ble_active ? DASH_RED : DASH_MUTED, 0);
    lv_label_set_text_fmt(widget->ble_label, "BLE %d", state.active_profile_index + 1);
    // Underline the requested output; red always means the actual active output.
    lv_obj_set_style_text_decor(widget->transport_label,
        state.preferred_transport == ZMK_TRANSPORT_USB ? LV_TEXT_DECOR_UNDERLINE : LV_TEXT_DECOR_NONE, 0);
    lv_obj_set_style_text_decor(widget->ble_label,
        state.preferred_transport == ZMK_TRANSPORT_BLE ? LV_TEXT_DECOR_UNDERLINE : LV_TEXT_DECOR_NONE, 0);
    const char *hint = "";
    if (state.preferred_transport == ZMK_TRANSPORT_BLE && !state.ble_ready) {
        hint = "BLE not connected";
    } else if (state.preferred_transport == ZMK_TRANSPORT_USB && !state.usb_ready) {
        hint = "USB not ready";
    }
    lv_label_set_text(widget->hint_label, hint);
}

static struct output_status_state previous_state;
static bool state_initialized;

static void output_status_update_cb(struct output_status_state state) {
    if (state_initialized && state_equal(state, previous_state)) {
        return;
    }
    previous_state = state;
    state_initialized = true;
    struct zmk_widget_output_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        set_status_symbol(widget, state);
    }
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_output_status, struct output_status_state,
                          output_status_update_cb, get_state)
ZMK_SUBSCRIPTION(widget_output_status, zmk_endpoint_changed);
ZMK_SUBSCRIPTION(widget_output_status, zmk_ble_active_profile_changed);
ZMK_SUBSCRIPTION(widget_output_status, zmk_usb_conn_state_changed);

/* Preference-only changes do not raise endpoint_changed when ZMK falls back
 * to the same available output. Poll on LVGL's thread, redraw only on changes. */
static void output_status_timer_cb(lv_timer_t *timer) {
    (void)timer;
    output_status_update_cb(get_state(NULL));
}

int zmk_widget_output_status_init(struct zmk_widget_output_status *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    dash_container(widget->obj, 208, 36);
    widget->transport_label = lv_label_create(widget->obj);
    widget->separator_label = lv_label_create(widget->obj);
    widget->ble_label = lv_label_create(widget->obj);
    widget->hint_label = lv_label_create(widget->obj);
    lv_obj_t *labels[] = {widget->transport_label, widget->separator_label, widget->ble_label};
    for (int i = 0; i < 3; i++) {
        lv_obj_set_style_text_font(labels[i], DASH_STATUS_FONT, 0);
        lv_obj_set_style_text_letter_space(labels[i], 0, 0);
    }
    lv_label_set_text(widget->transport_label, "USB");
    lv_label_set_text(widget->separator_label, "-");
    lv_obj_set_style_text_color(widget->separator_label, DASH_MUTED, 0);
    lv_obj_set_pos(widget->transport_label, 8, 0);
    lv_obj_set_pos(widget->separator_label, 80, 0);
    lv_obj_set_pos(widget->ble_label, 112, 0);
    lv_obj_set_style_text_font(widget->hint_label, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(widget->hint_label, DASH_MUTED, 0);
    lv_obj_set_width(widget->hint_label, 208);
    lv_obj_set_style_text_align(widget->hint_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(widget->hint_label, 0, 24);
    sys_slist_append(&widgets, &widget->node);
    state_initialized = false;
    widget_output_status_init();
    lv_timer_create(output_status_timer_cb, 200, NULL);
    return 0;
}

lv_obj_t *zmk_widget_output_status_obj(struct zmk_widget_output_status *widget) {
    return widget->obj;
}

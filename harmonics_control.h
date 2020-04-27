//
// Created by tmerchant on 04/04/2020.
//

#ifndef WAVETABLE_EDITOR_HARMONICS_CONTROL_H
#define WAVETABLE_EDITOR_HARMONICS_CONTROL_H

#include <gtkmm.h>
#include "util.h"

typedef struct harmonics_context
{
    int harmonics_width;
    int harmonics_height;
    int harmonics_page;
    int num_pages;
    int left_button_enabled;
    int right_button_enabled;
    int left_button_highlighted;
    int right_button_highlighted;
    int sine_highlighted;
    int square_highlighted;
    int sawtooth_highlighted;
    int triangle_highlighted;
    int odd_lock_highlighted;
    int even_lock_highlighted;
    int first_bin_in_page;
    int last_bin_in_page;
    int bins_per_page;
    int bin_width;
    int editing_cos_bins;
    int editing_sin_bins;
    int odd_lock;
    int even_lock;
    int zoom;
    int _1x_enabled;
    int _2x_enabled;
    int _4x_enabled;
    int _8x_enabled;
    int _16x_enabled;
    int _32x_enabled;
    int _64x_enabled;
    int _1x_highlighted;
    int _2x_highlighted;
    int _4x_highlighted;
    int _8x_highlighted;
    int _16x_highlighted;
    int _32x_highlighted;
    int _64x_highlighted;
    int expr_highlighted;
} harmonics_context;

#define NUM_HARMONICS_BTNS 16

extern button_hitbox *harmonic_btns;
extern harmonics_context harmonics_ctx;

bool draw_harmonics_control(const Cairo::RefPtr<Cairo::Context>& ctx);
bool harmonics_mouse_move(GdkEventMotion* motionevent);
bool harmonics_mouse_release(GdkEventButton* buttonevent);
bool harmonics_mouse_press(GdkEventButton* buttonevent);
bool harmonics_mouse_leave(GdkEventCrossing*);

#endif //WAVETABLE_EDITOR_HARMONICS_CONTROL_H

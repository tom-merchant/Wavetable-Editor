//
// Created by tmerchant on 04/04/2020.
//

#ifndef WAVETABLE_EDITOR_OSCILLOSCOPE_CONTROL_H
#define WAVETABLE_EDITOR_OSCILLOSCOPE_CONTROL_H

#include "fftw3.h"
#include <gtkmm.h>
#include <portaudio.h>

#include "util.h"

#define NUM_OSCILLOSCOPE_BTNS 6

typedef struct oscilloscope_context
{
    int oscilloscope_width;
    int oscilloscope_height;
    int play_highlighted;
    int settings_highlighted;
    int frequency_select_highlighted;
    int amplitude_select_highlighted;
    int play_morph_highlighted;
    int edit_lfo_highlighted;
    int playing_wavetable;
    int playing_morph_wavetable;
    double frequency;
    double amplitude;
    int device_count;
    double playback_time;
    int ADSR_released;
    int release_ADSR;
    const PaDeviceInfo **devices;
    bool has_initialised;
} oscilloscope_context;

extern oscilloscope_context oscilloscope_ctx;
extern fftw_plan oscilloscope_plan;

extern button_hitbox* oscilloscope_btns;

bool draw_oscilloscope_control(const Cairo::RefPtr<Cairo::Context>& ctx);

bool oscilloscope_mouse_move(GdkEventMotion* motionevent);
bool oscilloscope_mouse_press(GdkEventButton* buttonevent);
bool oscilloscope_mouse_release(GdkEventButton* buttonevent);
bool oscilloscope_mouse_leave(GdkEventCrossing*);
void oscilloscope_free_resources();


#endif //WAVETABLE_EDITOR_OSCILLOSCOPE_CONTROL_H

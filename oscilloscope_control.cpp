//
// Created by tmerchant on 04/04/2020.
//

#include <iostream>
#include "oscilloscope_control.h"
#include "control_structures.h"
#include "util.h"
#include "morph_preview.h"

namespace oscil{
    extern "C"
    {
        #include "oscillator.h"
    }
}

extern "C"
{
#include "AudioUtils.h"
}

oscilloscope_context oscilloscope_ctx = {};
fftw_plan oscilloscope_plan;
fftw_complex oscilloscope_freq_buf[507];
fftw_complex oscilloscope_buffer[506];
double oscilloscope_normal_buffer[506];
int order = 506;

button_hitbox* oscilloscope_btns;


oscil::oscillator *lfo;
oscil::oscillator *wtosc;
ADSR_envelope* amp_env;
PaStream *stream;

int audioCallback( const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo,
                   PaStreamCallbackFlags statusFlags, void *userData )
{
    unsigned long i;

    if(oscilloscope_ctx.playing_wavetable || oscilloscope_ctx.playing_morph_wavetable)
    {
        if ( oscilloscope_ctx.release_ADSR && !oscilloscope_ctx.ADSR_released )
        {
            ADSR_release ( amp_env, oscilloscope_ctx.playback_time );
            oscilloscope_ctx.ADSR_released = 1;
            oscilloscope_ctx.release_ADSR = 0;
        }

        for ( i = 0 ; i < frameCount * 2 ; i++ )
        {
            if(oscilloscope_ctx.playing_morph_wavetable)
            {
                wtosc->morph_pos = (oscil::next_sample (lfo) + 1) / 2;
                morph_ctx.playback_position = (float)wtosc->morph_pos;
            }

            auto nextsamp = (float)(oscilloscope_ctx.amplitude * oscil::next_sample ( wtosc ) *
                                    value_at ((envelope *) amp_env, oscilloscope_ctx.playback_time ));

            oscilloscope_ctx.playback_time += ( 1 / 44100.0f );

            ((float *) output )[ i ] = nextsamp;
            ((float *) output )[ i + 1 ] = nextsamp;
        }

        if(oscilloscope_ctx.ADSR_released)
        {
            if(oscilloscope_ctx.playback_time > amp_env->_t + amp_env->release->next->time)
            {
                oscilloscope_ctx.playing_wavetable = 0;
                oscilloscope_ctx.playing_morph_wavetable = 0;
                oscilloscope_ctx.ADSR_released = 0;
                oscilloscope_ctx.release_ADSR = 0;
                oscilloscope_ctx.playback_time = 0;
                ADSR_reset (amp_env);
                reset_oscillator (wtosc);
            }
        }
    }
    else
    {
        memset(output, 0, 2 * frameCount * sizeof(float));
    }

    return 0;
}


bool oscilloscope_mouse_move(GdkEventMotion* motionevent)
{
    if(!oscilloscope_ctx.has_initialised)return false;
    int x = motionevent->x;
    int y = motionevent->y;
    int i;
    
    bool anyhighlighted = false;

    for(i = 0; i < NUM_OSCILLOSCOPE_BTNS; i++)
    {
        if(is_in_rect(x, y, oscilloscope_btns[i].rect[0], oscilloscope_btns[i].rect[1], oscilloscope_btns[i].rect[2], oscilloscope_btns[i].rect[3]))
        {
            anyhighlighted = true;

            if(!*oscilloscope_btns[i].highlight)
            {
                *oscilloscope_btns[i].highlight = 1;
                ctrls.oscilloscope_control->queue_draw ();
            }
            break;
        }
    }

    if(!anyhighlighted)
    {
        anyhighlighted = false;

        for(i = 0; i < NUM_OSCILLOSCOPE_BTNS; i++)
        {
            if(*oscilloscope_btns[i].highlight)
            {
                *oscilloscope_btns[i].highlight = 0;
                anyhighlighted = true;
            }
        }

        if(anyhighlighted) ctrls.oscilloscope_control->queue_draw ();
    }

    return true;
}


void on_audio_device_change(Gtk::ComboBoxText *cb)
{
    int device = cb->get_active_row_number ();
    Pa_AbortStream (stream);
    Pa_CloseStream (stream);

    PaStreamParameters outParams;

    outParams.device = device;
    outParams.channelCount = 2;
    outParams.sampleFormat = paFloat32;
    outParams.hostApiSpecificStreamInfo = nullptr;
    //outParams.suggestedLatency = 32 / 44100.0f;
    outParams.suggestedLatency = 512 / 44100.0f;

    Pa_OpenStream (&stream, nullptr, &outParams, 44100, paFramesPerBufferUnspecified, paNoFlag, audioCallback, nullptr);

    Pa_StartStream (stream);
}

bool oscilloscope_mouse_release(GdkEventButton* buttonevent)
{
    oscilloscope_ctx.release_ADSR = 1;

    return false;
}

void on_frequency_change(Gtk::Entry* entry)
{
    oscilloscope_ctx.frequency = CLAMP(strtod(entry->get_text ().c_str (), nullptr), 0, 44100 / 2.0);

    if(wtosc)
    {
        wtosc->frequency = oscilloscope_ctx.frequency;
    }
}

void on_amplitude_change(Gtk::Entry* entry)
{
    oscilloscope_ctx.amplitude = CLAMP(strtod(entry->get_text ().c_str (), nullptr), -1, 1);
}

void on_lfo_frequency_change(Gtk::Entry* entry)
{
    lfo->frequency = CLAMP(strtod(entry->get_text ().c_str (), nullptr), 0, 44100 / 2.0);
}

void on_lfo_shape_change(Gtk::ComboBoxText *cb)
{
    switch(cb->get_active_row_number ())
    {
        case 0:
            lfo->source = oscil::sine;
            break;
        case 1:
            lfo->source = oscil::unlimited_square;
            break;
        case 2:
            lfo->source = oscil::unlimited_triangle;
            break;
        case 3:
            lfo->source = oscil::unlimited_ramp;
            break;
        case 4:
            lfo->source = oscil::unlimited_inverse_ramp;
            break;
    }
}

bool oscilloscope_mouse_press(GdkEventButton* buttonevent)
{
    int i;

    if(oscilloscope_ctx.play_highlighted && !oscilloscope_ctx.playing_wavetable && !oscilloscope_ctx.playing_morph_wavetable)
    {
        if(wtosc)free(wtosc);
        if ( active_table->mipmap )free_wavetable_mipmap (active_table->mipmap);

        int oversample = (int)strtol(ctrls.wavetable_oversample_entry->get_text ().c_str (), nullptr, 10);
        int tpo = (int)strtol(ctrls.wavetable_tables_per_octave_entry->get_text ().c_str (), nullptr, 10);

        active_table->mipmap = generate_wavetables (active_table->harmonics, oversample, tpo);
        generate_wavetable_freqmap (active_table->mipmap, 44100);

        wtosc = oscil::new_wavetable_oscillator ( oscilloscope_ctx.frequency, 0, 44100, active_table->mipmap );

        oscilloscope_ctx.release_ADSR = 0;
        oscilloscope_ctx.playing_wavetable = 1;
    }
    else if(oscilloscope_ctx.settings_highlighted && !oscilloscope_ctx.playing_wavetable && !oscilloscope_ctx.playing_morph_wavetable)
    {
        auto dialog = new Gtk::Dialog();
        auto audio_devices = new Gtk::ComboBoxText();
        dialog->set_modal (true);

        for(i = 0; i < oscilloscope_ctx.device_count; i++)
        {
            audio_devices->append(oscilloscope_ctx.devices[i]->name);
        }

        reinterpret_cast<Gtk::Box*>(dialog->get_child ())->add (*audio_devices);

        audio_devices->signal_changed ().connect (sigc::bind<Gtk::ComboBoxText*>(sigc::ptr_fun (&on_audio_device_change), audio_devices));

        audio_devices->set_visible (true);
        dialog->set_visible (true);
        dialog->show();
    }
    else if(oscilloscope_ctx.frequency_select_highlighted)
    {
        auto dialog = new Gtk::Dialog();
        auto freqEntry = new Gtk::Entry();

        reinterpret_cast<Gtk::Box*>(dialog->get_child ())->add (*freqEntry);

        std::stringstream freqText;
        freqText << oscilloscope_ctx.frequency;

        freqEntry->set_placeholder_text (freqText.str ());

        freqEntry->signal_activate ().connect (sigc::bind<Gtk::Entry*>(sigc::ptr_fun (&on_frequency_change), freqEntry));

        dialog->set_visible(true);
        freqEntry->set_visible(true);

        dialog->show();
    }
    else if(oscilloscope_ctx.amplitude_select_highlighted)
    {
        auto dialog = new Gtk::Dialog();
        auto ampEntry = new Gtk::Entry(); // TODO: This should be a logarithmic slider, not a textbox, Gtk::Scale

        reinterpret_cast<Gtk::Box*>(dialog->get_child ())->add (*ampEntry);

        std::stringstream ampText;
        ampText << oscilloscope_ctx.amplitude;

        ampEntry->set_placeholder_text (ampText.str ());

        ampEntry->signal_activate ().connect (sigc::bind<Gtk::Entry*>(sigc::ptr_fun (&on_amplitude_change), ampEntry));

        dialog->set_visible(true);
        ampEntry->set_visible(true);

        dialog->show();
    }
    else if(oscilloscope_ctx.play_morph_highlighted && ctrls.mwm && !oscilloscope_ctx.playing_wavetable && !oscilloscope_ctx.playing_morph_wavetable)
    {
        if(wtosc)free(wtosc);
        if ( active_table->mipmap )free_wavetable_mipmap ( active_table->mipmap );

        wtosc = oscil::new_morph_wavetable_oscillator ( oscilloscope_ctx.frequency, 0, 44100, ctrls.mwm );
        oscil::reset_oscillator (lfo);
        oscilloscope_ctx.release_ADSR = 0;
        oscilloscope_ctx.playing_morph_wavetable = 1;
        ctrls.oscilloscope_control->queue_draw ();
    }
    else if(oscilloscope_ctx.edit_lfo_highlighted && !oscilloscope_ctx.playing_morph_wavetable)
    {
        auto dialog = new Gtk::Dialog();
        auto freqEntry = new Gtk::Entry();
        auto lfoShape = new Gtk::ComboBoxText();

        dialog->set_title ("Change LFO frequency/shape");

        reinterpret_cast<Gtk::Box*>(dialog->get_child ())->add (*freqEntry);
        reinterpret_cast<Gtk::Box*>(dialog->get_child ())->add (*lfoShape);

        std::stringstream freqText;
        freqText << lfo->frequency << " Hz";

        freqEntry->set_placeholder_text (freqText.str ());

        lfoShape->append ("Sine");
        lfoShape->append ("Square");
        lfoShape->append ("Triangle");
        lfoShape->append ("Ramp");
        lfoShape->append ("Inverse ramp");

        freqEntry->signal_activate ().connect (sigc::bind<Gtk::Entry*>(sigc::ptr_fun (&on_lfo_frequency_change), freqEntry));
        lfoShape->signal_changed ().connect (sigc::bind<Gtk::ComboBoxText*> (sigc::ptr_fun (&on_lfo_shape_change), lfoShape));

        lfoShape->set_visible (true);
        freqEntry->set_visible(true);
        dialog->set_visible(true);

        dialog->show();
    }

    return false;
}

bool oscilloscope_mouse_leave(GdkEventCrossing*)
{
    oscilloscope_ctx.release_ADSR = 1;
    oscilloscope_ctx.play_highlighted = 0;
    oscilloscope_ctx.settings_highlighted = 0;
    return false;
}

bool draw_oscilloscope_control(const Cairo::RefPtr<Cairo::Context>& ctx)
{
    int i;
    if(!oscilloscope_ctx.has_initialised)
    {
        oscilloscope_ctx.oscilloscope_width = ctrls.oscilloscope_control->get_window ()->get_width ();
        oscilloscope_ctx.oscilloscope_height = ctrls.oscilloscope_control->get_window ()->get_height ();
        oscilloscope_plan = fftw_plan_dft (1, &order, oscilloscope_freq_buf, oscilloscope_buffer, FFTW_BACKWARD, 0);

        oscilloscope_btns = new button_hitbox[NUM_OSCILLOSCOPE_BTNS]{
                {&oscilloscope_ctx.play_highlighted, {42, oscilloscope_ctx.oscilloscope_height - 24, 18, 18}},
                {&oscilloscope_ctx.settings_highlighted, {66, oscilloscope_ctx.oscilloscope_height - 24, 18, 18}},
                {&oscilloscope_ctx.frequency_select_highlighted, {90, oscilloscope_ctx.oscilloscope_height - 24, 18, 18}},
                {&oscilloscope_ctx.amplitude_select_highlighted, {114, oscilloscope_ctx.oscilloscope_height - 24, 18, 18}},
                {&oscilloscope_ctx.play_morph_highlighted, {162, oscilloscope_ctx.oscilloscope_height - 24, 18, 18}},
                {&oscilloscope_ctx.edit_lfo_highlighted, {186, oscilloscope_ctx.oscilloscope_height - 24, 18, 18}}
        };

        Pa_Initialize ();
        Pa_OpenDefaultStream (&stream, 0, 2, paFloat32, 44100, paFramesPerBufferUnspecified, audioCallback, nullptr);

        Pa_StartStream (stream);

        oscilloscope_ctx.device_count = Pa_GetDeviceCount ();
        oscilloscope_ctx.devices = (const PaDeviceInfo**)malloc (oscilloscope_ctx.device_count * sizeof(void*));

        for(i = 0; i < oscilloscope_ctx.device_count; i++)
        {
            oscilloscope_ctx.devices[i] = Pa_GetDeviceInfo (i);
        }

        oscilloscope_ctx.frequency = 50;
        oscilloscope_ctx.amplitude = 0.8;

        amp_env = create_ADSR_envelope (0.1, 0.2, 0.9, 0.2);

        lfo = oscil::new_oscillator (0.5, 0, 0.5, 44100, oscil::unlimited_triangle);

        oscilloscope_ctx.has_initialised = true;
    }

    ctx->set_source_rgb (0, 0, 0);
    ctx->rectangle ( 0, 0, oscilloscope_ctx.oscilloscope_width, oscilloscope_ctx.oscilloscope_height );
    ctx->fill ();

    ctx->set_source_rgb (0, 1, 0);
    ctx->set_line_width (3);
    ctx->line_to (0, oscilloscope_ctx.oscilloscope_height - 30);
    ctx->line_to (oscilloscope_ctx.oscilloscope_width, oscilloscope_ctx.oscilloscope_height - 30);
    ctx->rectangle (0, 0, oscilloscope_ctx.oscilloscope_width, oscilloscope_ctx.oscilloscope_height);
    ctx->stroke ();

    for(i = 0; i < order + 1; i++)
    {
        oscilloscope_freq_buf[i][0] = 0;
        oscilloscope_freq_buf[i][1] = 0;
    }

    if(!oscilloscope_ctx.playing_morph_wavetable)
    {
        for(i = 1; i < MIN(order + 1, active_table->harmonics->num_harmonics + 1); i++)
        {
            oscilloscope_freq_buf[i][0] = active_table->harmonics->cos_table[i - 1];
            oscilloscope_freq_buf[i][1] = active_table->harmonics->sin_table[i - 1];
        }

        fftw_execute ( oscilloscope_plan );

        for(i = 0; i < order; i++)
        {
            oscilloscope_normal_buffer[i] = oscilloscope_buffer[i][0];
        }

        remove_dc_offset(oscilloscope_normal_buffer, order);
        normalise(oscilloscope_normal_buffer, order);

        for(i = 0; i < order; i++)
        {
            //scale it down into view
            oscilloscope_normal_buffer[i] *= 0.8;
        }
    }
    else
    {
        wavetable_mipmap* table = &ctrls.mwm->tables[(int)(wtosc->morph_pos * (ctrls.mwm->total_tables - 1))];

        for(i = 0; i < order; i++)
        {
            oscilloscope_normal_buffer[i] = table->tables[0].table[(int)(i * ((double)table->tables[0].num_samples / order))];
        }

        for(i = 0; i < order; i++)
        {
            //scale it down into view
            oscilloscope_normal_buffer[i] *= 0.8;
        }

        ctrls.oscilloscope_control->queue_draw ();
    }

    for(i = 0; i < order; i++)
    {
        ctx->line_to (i + 1, 0.5 * (1 + oscilloscope_normal_buffer[i]) * (oscilloscope_ctx.oscilloscope_height - 30));
    }

    ctx->rectangle (42, oscilloscope_ctx.oscilloscope_height - 24, 18, 18);
    ctx->rectangle (66, oscilloscope_ctx.oscilloscope_height - 24, 18, 18);
    ctx->rectangle (90, oscilloscope_ctx.oscilloscope_height - 24, 18, 18);
    ctx->rectangle (114, oscilloscope_ctx.oscilloscope_height - 24, 18, 18);

    ctx->rectangle (162, oscilloscope_ctx.oscilloscope_height - 24, 18, 18);
    ctx->rectangle (186, oscilloscope_ctx.oscilloscope_height - 24, 18, 18);

    ctx->stroke();

    if(oscilloscope_ctx.play_highlighted || oscilloscope_ctx.playing_wavetable)ctx->set_source_rgb ( 1, 0, 0);

    ctx->line_to (42 + 4, oscilloscope_ctx.oscilloscope_height - 24 + 4);
    ctx->line_to (42 + 15, oscilloscope_ctx.oscilloscope_height - 24 + 9);
    ctx->line_to (42 + 4, oscilloscope_ctx.oscilloscope_height - 24 + 14);

    ctx->fill();

    ctx->set_source_rgb (0, 1, 0);
    if(oscilloscope_ctx.settings_highlighted)ctx->set_source_rgb (1, 0, 0);

    ctx->line_to (66 + 1, oscilloscope_ctx.oscilloscope_height - 24 + 15);
    ctx->set_font_size (17);
    ctx->show_text ("⚙");

    ctx->set_source_rgb (0, 1, 0);
    if(oscilloscope_ctx.frequency_select_highlighted)ctx->set_source_rgb (1, 0, 0);

    ctx->line_to (90 + 6, oscilloscope_ctx.oscilloscope_height - 24 + 15);
    ctx->set_font_size (16);
    ctx->show_text ("f");

    ctx->set_source_rgb (0, 1, 0);
    if(oscilloscope_ctx.amplitude_select_highlighted)ctx->set_source_rgb (1, 0, 0);

    ctx->line_to (114 + 4, oscilloscope_ctx.oscilloscope_height - 24 + 15);
    ctx->set_font_size (16);
    ctx->show_text ("A");

    ctx->set_source_rgb (0, 1, 0);
    if(oscilloscope_ctx.play_morph_highlighted)ctx->set_source_rgb (1, 0, 0);

    ctx->begin_new_sub_path ();
    ctx->line_to (162 + 4, oscilloscope_ctx.oscilloscope_height - 24 + 4);
    ctx->line_to (162 + 15, oscilloscope_ctx.oscilloscope_height - 24 + 9);
    ctx->line_to (162 + 4, oscilloscope_ctx.oscilloscope_height - 24 + 14);

    ctx->fill();

    ctx->set_source_rgb (0, 1, 0);
    if(oscilloscope_ctx.edit_lfo_highlighted)ctx->set_source_rgb (1, 0, 0);

    ctx->line_to (187, oscilloscope_ctx.oscilloscope_height - 24 + 15);
    ctx->set_font_size (19);
    ctx->show_text ("∿");

    return true;
}

void oscilloscope_free_resources()
{
    Pa_CloseStream (stream);
}
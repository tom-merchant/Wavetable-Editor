//
// Created by tmerchant on 04/04/2020.
//

#include <iostream>
#include "harmonics_control.h"
#include "control_structures.h"


button_hitbox *harmonic_btns;

bool harmonics_init = false;

harmonics_context harmonics_ctx = {0};

int* zoom_enabled = &harmonics_ctx._1x_enabled;

#define MIN_BIN_WIDTH 5

bool is_in_rect(int x, int y, int rectx, int recty, int rectw, int recth)
{
    return (x > rectx && x <= rectx + rectw) && (y > recty && y <= recty + recth );
}

bool harmonics_mouse_leave(GdkEventCrossing*)
{
    harmonics_ctx.right_button_highlighted = 0;
    harmonics_ctx.left_button_highlighted = 0;
    harmonics_ctx.sine_highlighted = 0;
    harmonics_ctx.square_highlighted = 0;
    harmonics_ctx.sawtooth_highlighted = 0;
    harmonics_ctx.triangle_highlighted = 0;
    harmonics_ctx.odd_lock_highlighted = 0;
    harmonics_ctx.even_lock_highlighted = 0;
    harmonics_ctx._1x_highlighted = 0;
    harmonics_ctx._2x_highlighted = 0;
    harmonics_ctx._4x_highlighted = 0;
    harmonics_ctx._8x_highlighted = 0;
    harmonics_ctx._16x_highlighted = 0;
    harmonics_ctx._32x_highlighted = 0;
    harmonics_ctx._64x_highlighted = 0;
    harmonics_ctx.editing_cos_bins = 0;
    harmonics_ctx.editing_sin_bins = 0;
    ctrls.harmonics_control->queue_draw ();

    return false;
}

void on_expr_go(Gtk::Dialog *dialogue)
{
    int i;

    Gtk::TextView *expr_entry = reinterpret_cast<Gtk::TextView*>(reinterpret_cast<Gtk::Box*>(dialogue->get_child ())->get_children ().at (1));

    active_table->last_expr = expr_entry->get_buffer ()->get_text (true);

    std::stringstream func, exec_func, sinlength, coslength;

    js_dostring (ctrls.js, "var _sin = [];");
    js_dostring (ctrls.js, "var _cos = [];");

    func << std::endl << "function gen_harmonics ( cos_array, sin_array, num_bins ) {" << std::endl;
    func << active_table->last_expr << std::endl;
    func << "}";

    js_dostring (ctrls.js, func.str ().c_str ());

    exec_func << "gen_harmonics(_cos, _sin, " << active_table->harmonics->num_harmonics << ");";

    js_dostring (ctrls.js, exec_func.str ().c_str ());

    int sin_length, cos_length;

    js_getglobal (ctrls.js, "_sin");
    sin_length = js_getlength (ctrls.js, -1);


    for(i = 0; i < MIN(sin_length, active_table->harmonics->num_harmonics); i++)
    {
        js_getindex (ctrls.js, -1, i);
        active_table->harmonics->sin_table[i] = js_tonumber (ctrls.js, -1);
        js_pop (ctrls.js, 1);

        if(std::isnan (active_table->harmonics->sin_table[i]))
        {
            active_table->harmonics->sin_table[i] = 0;
        }
    }

    js_getglobal (ctrls.js, "_cos");
    cos_length = js_getlength (ctrls.js, -1);

    for(i = 0; i < MIN(cos_length, active_table->harmonics->num_harmonics); i++)
    {
        js_getindex (ctrls.js, -1, i);
        active_table->harmonics->sin_table[i] = js_tonumber (ctrls.js, -1);
        js_pop (ctrls.js, 1);

        if(std::isnan (active_table->harmonics->cos_table[i]))
        {
            active_table->harmonics->cos_table[i] = 0;
        }
    }

    js_gc (ctrls.js, 0);

    dialogue->close ();

    update_wavetable_controls ();
}

bool draw_harmonics_control(const Cairo::RefPtr<Cairo::Context>& ctx)
{
    int i;

    if(!harmonics_init)
    {
        harmonics_ctx.harmonics_width = ctrls.harmonics_control->get_window ()->get_width ();
        harmonics_ctx.harmonics_height = ctrls.harmonics_control->get_window ()->get_height ();
        harmonics_init = true;
        harmonics_ctx.zoom = 1;
        harmonics_ctx._1x_enabled = 1;

        harmonic_btns = new button_hitbox[NUM_HARMONICS_BTNS]{
                {&harmonics_ctx.right_button_highlighted, {harmonics_ctx.harmonics_width - 24, harmonics_ctx.harmonics_height - 24, 18, 18}},
                {&harmonics_ctx.left_button_highlighted, {6, harmonics_ctx.harmonics_height - 24, 18, 18}},
                {&harmonics_ctx.sine_highlighted, {42, harmonics_ctx.harmonics_height - 24, 18, 18}},
                {&harmonics_ctx.square_highlighted, {66, harmonics_ctx.harmonics_height - 24, 18, 18}},
                {&harmonics_ctx.sawtooth_highlighted, {90, harmonics_ctx.harmonics_height - 24, 18, 18}},
                {&harmonics_ctx.triangle_highlighted, {114, harmonics_ctx.harmonics_height - 24, 18, 18}},
                {&harmonics_ctx.odd_lock_highlighted, {162, harmonics_ctx.harmonics_height - 24, 18, 18}},
                {&harmonics_ctx.even_lock_highlighted, {186, harmonics_ctx.harmonics_height - 24, 18, 18}},
                {&harmonics_ctx._1x_highlighted, {234, harmonics_ctx.harmonics_height - 24, 18, 18}},
                {&harmonics_ctx._2x_highlighted, {258, harmonics_ctx.harmonics_height - 24, 18, 18}},
                {&harmonics_ctx._4x_highlighted, {282, harmonics_ctx.harmonics_height - 24, 18, 18}},
                {&harmonics_ctx._8x_highlighted, {306, harmonics_ctx.harmonics_height - 24, 18, 18}},
                {&harmonics_ctx._16x_highlighted, {330, harmonics_ctx.harmonics_height - 24, 18, 18}},
                {&harmonics_ctx._32x_highlighted, {354, harmonics_ctx.harmonics_height - 24, 18, 18}},
                {&harmonics_ctx._64x_highlighted, {378, harmonics_ctx.harmonics_height - 24, 18, 18}},
                {&harmonics_ctx.expr_highlighted, {426, harmonics_ctx.harmonics_height - 24, 18, 18}}
        };
    }

    ctx->set_source_rgb (0, 0, 0);
    ctx->rectangle ( 0, 0, harmonics_ctx.harmonics_width, harmonics_ctx.harmonics_height );
    ctx->fill ();

    ctx->set_source_rgb (1, 1, 1);
    ctx->set_line_width (3);
    ctx->line_to (0, (double)(harmonics_ctx.harmonics_height - 30) / 2);
    ctx->line_to (harmonics_ctx.harmonics_width, (double)(harmonics_ctx.harmonics_height - 30) / 2);
    ctx->begin_new_sub_path ();
    ctx->line_to (0, harmonics_ctx.harmonics_height - 30);
    ctx->line_to (harmonics_ctx.harmonics_width, harmonics_ctx.harmonics_height - 30);
    ctx->rectangle (0, 0, harmonics_ctx.harmonics_width, harmonics_ctx.harmonics_height);
    ctx->stroke ();

    harmonics_ctx.bin_width = (harmonics_ctx.harmonics_width - 6) / active_table->harmonics->num_harmonics;

    if ( harmonics_ctx.bin_width < MIN_BIN_WIDTH )
    {
        harmonics_ctx.num_pages = ceil((double)(active_table->harmonics->num_harmonics * MIN_BIN_WIDTH) / (float)(harmonics_ctx.harmonics_width - 6));
        harmonics_ctx.bin_width = MIN_BIN_WIDTH;

        if ( harmonics_ctx.harmonics_page == 0 )
        {
            harmonics_ctx.left_button_enabled = 0;
            harmonics_ctx.right_button_enabled = 1;
        }
        else if (harmonics_ctx.harmonics_page < harmonics_ctx.num_pages - 1)
        {
            harmonics_ctx.left_button_enabled = 1;
            harmonics_ctx.right_button_enabled = 1;
        }
        else
        {
            harmonics_ctx.left_button_enabled = 1;
            harmonics_ctx.right_button_enabled = 0;
        }
    }
    else
    {
        harmonics_ctx.num_pages = 1;
        harmonics_ctx.left_button_enabled = 0;
        harmonics_ctx.right_button_enabled = 0;
    }


    if ( harmonics_ctx.left_button_enabled )
    {
        if(harmonics_ctx.left_button_highlighted)
        {
            ctx->set_source_rgb (0.1, 0.2, 1);
        }
        ctx->rectangle (6, harmonics_ctx.harmonics_height - 24, 18, 18);
        ctx->begin_new_sub_path ();
        ctx->line_to (24, harmonics_ctx.harmonics_height - 24);
        ctx->line_to (8, harmonics_ctx.harmonics_height - ((double)(24 + 6) / 2));
        ctx->line_to (24, harmonics_ctx.harmonics_height - 6);
        ctx->stroke ();
        ctx->set_source_rgb (1, 1, 1);
    }

    if( harmonics_ctx.right_button_enabled )
    {
        if(harmonics_ctx.right_button_highlighted)
        {
            ctx->set_source_rgb (0.1, 0.2, 1);
        }
        ctx->rectangle (harmonics_ctx.harmonics_width - 24, harmonics_ctx.harmonics_height - 24, 18, 18);
        ctx->line_to (harmonics_ctx.harmonics_width - 8, harmonics_ctx.harmonics_height - ((double)(24 + 6) / 2));
        ctx->line_to (harmonics_ctx.harmonics_width - 24, harmonics_ctx.harmonics_height - 6);
        ctx->stroke ();
        ctx->set_source_rgb (1, 1, 1);
    }

    ctx->rectangle (42, harmonics_ctx.harmonics_height - 24, 18, 18);
    ctx->rectangle (66, harmonics_ctx.harmonics_height - 24, 18, 18);
    ctx->rectangle (90, harmonics_ctx.harmonics_height - 24, 18, 18);
    ctx->rectangle (114, harmonics_ctx.harmonics_height - 24, 18, 18);
    ctx->rectangle (162, harmonics_ctx.harmonics_height - 24, 18, 18);
    ctx->rectangle (186, harmonics_ctx.harmonics_height - 24, 18, 18);
    ctx->rectangle (234, harmonics_ctx.harmonics_height - 24, 18, 18);
    ctx->rectangle (258, harmonics_ctx.harmonics_height - 24, 18, 18);
    ctx->rectangle (282, harmonics_ctx.harmonics_height - 24, 18, 18);
    ctx->rectangle (306, harmonics_ctx.harmonics_height - 24, 18, 18);
    ctx->rectangle (330, harmonics_ctx.harmonics_height - 24, 18, 18);
    ctx->rectangle (354, harmonics_ctx.harmonics_height - 24, 18, 18);
    ctx->rectangle (378, harmonics_ctx.harmonics_height - 24, 18, 18);
    ctx->rectangle (426, harmonics_ctx.harmonics_height - 24, 18, 18);
    ctx->stroke ();

    //sine wave icon
    if(harmonics_ctx.sine_highlighted) ctx->set_source_rgb (0.1, 0.2, 1);
    ctx->line_to (43, harmonics_ctx.harmonics_height - 24 + 15);
    ctx->set_font_size (19);
    ctx->show_text ("∿");
    ctx->stroke ();

    ctx->set_source_rgb (1, 1, 1);
    //square wave icon
    if(harmonics_ctx.square_highlighted) ctx->set_source_rgb (0.1, 0.2, 1);
    ctx->line_to (66 + 3, harmonics_ctx.harmonics_height - 24 + 12);
    ctx->line_to (66 + 9, harmonics_ctx.harmonics_height - 24 + 12);
    ctx->line_to (66 + 9, harmonics_ctx.harmonics_height - 24 + 6);
    ctx->line_to (66 + 15, harmonics_ctx.harmonics_height - 24 + 6);
    ctx->stroke ();

    ctx->set_source_rgb (1, 1, 1);
    //sawtooth icon
    if(harmonics_ctx.sawtooth_highlighted) ctx->set_source_rgb (0.1, 0.2, 1);
    ctx->line_to (90 + 3, harmonics_ctx.harmonics_height - 24 + 12 );
    ctx->line_to (90 + 13, harmonics_ctx.harmonics_height - 24 + 6 );
    ctx->line_to (90 + 13, harmonics_ctx.harmonics_height - 24 + 13 );
    ctx->stroke ();

    ctx->set_source_rgb (1, 1, 1);
    //triangle wave icon
    if(harmonics_ctx.triangle_highlighted) ctx->set_source_rgb (0.1, 0.2, 1);
    ctx->line_to (114 + 4, harmonics_ctx.harmonics_height - 24 + 12);
    ctx->line_to (114 + 9, harmonics_ctx.harmonics_height - 24 + 6);
    ctx->line_to (114 + 14, harmonics_ctx.harmonics_height - 24 + 12);
    ctx->stroke ();

    ctx->set_source_rgb (1, 1, 1);
    //odd lock icon
    if(harmonics_ctx.odd_lock) ctx->set_source_rgb (1, 0.3, 0.7);
    if(harmonics_ctx.odd_lock_highlighted) ctx->set_source_rgb (0.1, 0.2, 1);
    ctx->line_to (162 + 3, harmonics_ctx.harmonics_height - 24 + 15);
    ctx->set_font_size (16);
    ctx->show_text ("O");
    ctx->stroke ();

    ctx->set_source_rgb (1, 1, 1);
    //even lock icon
    if(harmonics_ctx.even_lock) ctx->set_source_rgb (1, 0.3, 0.7);
    if(harmonics_ctx.even_lock_highlighted) ctx->set_source_rgb (0.1, 0.2, 1);
    ctx->line_to (186 + 3, harmonics_ctx.harmonics_height - 24 + 15);
    ctx->set_font_size (16);
    ctx->show_text ("E");
    ctx->stroke ();

    ctx->set_source_rgb (1, 1, 1);
    //1x zoom icon
    if(harmonics_ctx._1x_highlighted) ctx->set_source_rgb (0.1, 0.2, 1);
    if(harmonics_ctx._1x_enabled) ctx->set_source_rgb (1, 0.3, 0.7);
    ctx->line_to (234 + 1, harmonics_ctx.harmonics_height - 24 + 14);
    ctx->set_font_size (12);
    ctx->show_text ("1x");
    ctx->stroke ();

    ctx->set_source_rgb (1, 1, 1);
    //2x zoom icon
    if(harmonics_ctx._2x_highlighted) ctx->set_source_rgb (0.1, 0.2, 1);
    if(harmonics_ctx._2x_enabled) ctx->set_source_rgb (1, 0.3, 0.7);
    ctx->line_to (258 + 1, harmonics_ctx.harmonics_height - 24 + 14);
    ctx->show_text ("2x");
    ctx->stroke ();

    ctx->set_source_rgb (1, 1, 1);
    //2x zoom icon
    if(harmonics_ctx._4x_highlighted) ctx->set_source_rgb (0.1, 0.2, 1);
    if(harmonics_ctx._4x_enabled) ctx->set_source_rgb (1, 0.3, 0.7);
    ctx->line_to (282 + 1, harmonics_ctx.harmonics_height - 24 + 14);
    ctx->show_text ("4x");
    ctx->stroke ();

    ctx->set_source_rgb (1, 1, 1);
    //2x zoom icon
    if(harmonics_ctx._8x_highlighted) ctx->set_source_rgb (0.1, 0.2, 1);
    if(harmonics_ctx._8x_enabled) ctx->set_source_rgb (1, 0.3, 0.7);
    ctx->line_to (306 + 1, harmonics_ctx.harmonics_height - 24 + 14);
    ctx->show_text ("8x");
    ctx->stroke ();

    ctx->set_source_rgb (1, 1, 1);
    //2x zoom icon
    if(harmonics_ctx._16x_highlighted) ctx->set_source_rgb (0.1, 0.2, 1);
    if(harmonics_ctx._16x_enabled) ctx->set_source_rgb (1, 0.3, 0.7);
    ctx->line_to (330 + 1, harmonics_ctx.harmonics_height - 24 + 14);
    ctx->show_text ("16");
    ctx->stroke ();

    ctx->set_source_rgb (1, 1, 1);
    //2x zoom icon
    if(harmonics_ctx._32x_highlighted) ctx->set_source_rgb (0.1, 0.2, 1);
    if(harmonics_ctx._32x_enabled) ctx->set_source_rgb (1, 0.3, 0.7);
    ctx->line_to (354 + 1, harmonics_ctx.harmonics_height - 24 + 14);
    ctx->show_text ("32");
    ctx->stroke ();

    ctx->set_source_rgb (1, 1, 1);
    //2x zoom icon
    if(harmonics_ctx._64x_highlighted) ctx->set_source_rgb (0.1, 0.2, 1);
    if(harmonics_ctx._64x_enabled) ctx->set_source_rgb (1, 0.3, 0.7);
    ctx->line_to (378 + 1, harmonics_ctx.harmonics_height - 24 + 14);
    ctx->show_text ("64");
    ctx->stroke ();

    ctx->set_source_rgb (1, 1, 1);

    if(harmonics_ctx.expr_highlighted) ctx->set_source_rgb (0.1, 0.2, 1);
    ctx->line_to(426 + 1.4, harmonics_ctx.harmonics_height - 24 + 13);
    ctx->show_text("{}");
    ctx->stroke();

    ctx->set_source_rgb (1, 1, 1);

    harmonics_ctx.bins_per_page = (harmonics_ctx.harmonics_width - 6) / harmonics_ctx.bin_width;

    harmonics_ctx.first_bin_in_page = harmonics_ctx.bins_per_page * harmonics_ctx.harmonics_page;
    harmonics_ctx.last_bin_in_page = harmonics_ctx.first_bin_in_page + harmonics_ctx.bins_per_page;

#define ABSMIN(x, y) ((fabs(x) < fabs(y) ? x : y))

    for ( i = harmonics_ctx.first_bin_in_page; i <= harmonics_ctx.last_bin_in_page && i < active_table->harmonics->num_harmonics; i++ )
    {
        ctx->rectangle (3 + (i % harmonics_ctx.bins_per_page) * harmonics_ctx.bin_width, ((double)(harmonics_ctx.harmonics_height - 30) / 2 - 6) - (double)(harmonics_ctx.harmonics_height - 30) / 4, harmonics_ctx.bin_width,
                        ABSMIN(harmonics_ctx.zoom * active_table->harmonics->cos_table[i] * ((-((double)(harmonics_ctx.harmonics_height - 30) / 2 - 9)) / 2), ((-((double)(harmonics_ctx.harmonics_height - 30) / 2 - 9)) / 2)) );

        ctx->rectangle (3 + (i % harmonics_ctx.bins_per_page) * harmonics_ctx.bin_width, (harmonics_ctx.harmonics_height - 36) - (double)(harmonics_ctx.harmonics_height - 30) / 4, harmonics_ctx.bin_width,
                        ABSMIN(harmonics_ctx.zoom * active_table->harmonics->sin_table[i] * ((-((double)(harmonics_ctx.harmonics_height - 30) / 2 - 9)) / 2), ((-((double)(harmonics_ctx.harmonics_height - 30) / 2 - 9)) / 2)) );
    }

    ctx->fill ();

    return true;
}

bool harmonics_mouse_press(GdkEventButton* buttonevent)
{
    int x = buttonevent->x;
    int y = buttonevent->y;
    int i, j = 0;

    if(buttonevent->button == 1)
    {
        if ( is_in_rect ( x, y, 3, 3, harmonics_ctx.harmonics_width - 6, (( harmonics_ctx.harmonics_height - 30 ) / 2 ) - 6 ))
        {
            harmonics_ctx.editing_cos_bins = 1;
        }
        else if ( is_in_rect ( x, y, 3, (( harmonics_ctx.harmonics_height - 30 ) / 2 ) + 3, harmonics_ctx.harmonics_width - 6, (( harmonics_ctx.harmonics_height - 30 ) / 2 ) - 6 ))
        {
            harmonics_ctx.editing_sin_bins = 1;
        }
        else if ( harmonics_ctx.left_button_enabled && harmonics_ctx.left_button_highlighted )
        {
            harmonics_ctx.harmonics_page--;
            ctrls.harmonics_control->queue_draw ();

            if(harmonics_ctx.harmonics_page == 0) harmonics_ctx.left_button_enabled = 0;
        }
        else if ( harmonics_ctx.right_button_enabled && harmonics_ctx.right_button_highlighted )
        {
            harmonics_ctx.harmonics_page++;
            ctrls.harmonics_control->queue_draw ();

            if(harmonics_ctx.harmonics_page == harmonics_ctx.num_pages - 1) harmonics_ctx.right_button_enabled = 0;
        }
        else if ( harmonics_ctx.sine_highlighted )
        {
            memset(active_table->harmonics->cos_table, 0, sizeof( double ) * active_table->harmonics->num_harmonics);
            memset(active_table->harmonics->sin_table, 0, sizeof( double ) * active_table->harmonics->num_harmonics);

            active_table->harmonics->sin_table[0] = 1;

            ctrls.harmonics_control->queue_draw ();
            ctrls.oscilloscope_control->queue_draw ();
        }
        else if ( harmonics_ctx.square_highlighted )
        {
            memset(active_table->harmonics->cos_table, 0, sizeof( double ) * active_table->harmonics->num_harmonics);
            memset(active_table->harmonics->sin_table, 0, sizeof( double ) * active_table->harmonics->num_harmonics);

            for(i = 0; i < active_table->harmonics->num_harmonics; i++)
            {
                if((i + 1) % 2)
                {
                    active_table->harmonics->sin_table[ i ] = 1.0f / (double)( i + 1 );
                }
            }

            ctrls.harmonics_control->queue_draw ();
            ctrls.oscilloscope_control->queue_draw ();
        }
        else if ( harmonics_ctx.sawtooth_highlighted )
        {
            memset(active_table->harmonics->cos_table, 0, sizeof( double ) * active_table->harmonics->num_harmonics);
            memset(active_table->harmonics->sin_table, 0, sizeof( double ) * active_table->harmonics->num_harmonics);

            for(i = 0; i < active_table->harmonics->num_harmonics; i++)
            {
                active_table->harmonics->sin_table[i] = 1.0f / (double)(i + 1);
            }

            ctrls.harmonics_control->queue_draw ();
            ctrls.oscilloscope_control->queue_draw ();
        }
        else if ( harmonics_ctx.triangle_highlighted )
        {
            memset(active_table->harmonics->cos_table, 0, sizeof( double ) * active_table->harmonics->num_harmonics);
            memset(active_table->harmonics->sin_table, 0, sizeof( double ) * active_table->harmonics->num_harmonics);

            for(i = 0; i < active_table->harmonics->num_harmonics; i++)
            {
                if((i + 1) % 2)
                {
                    if(!j)
                    {
                        active_table->harmonics->sin_table[ i ] = 1.0f / (double)((i + 1) * (i + 1));
                    }
                    else
                    {
                        active_table->harmonics->sin_table[ i ] = -1.0f / (double)((i + 1) * (i + 1));
                    }

                    j = (j ? 0 : 1);
                }
            }

            ctrls.harmonics_control->queue_draw ();
            ctrls.oscilloscope_control->queue_draw ();
        }
        else if(harmonics_ctx.odd_lock_highlighted && !harmonics_ctx.even_lock)
        {
            harmonics_ctx.odd_lock = !harmonics_ctx.odd_lock;
        }
        else if(harmonics_ctx.even_lock_highlighted && !harmonics_ctx.odd_lock)
        {
            harmonics_ctx.even_lock = !harmonics_ctx.even_lock;
        }
        else if(harmonics_ctx._1x_highlighted)
        {
            *zoom_enabled = 0;
            harmonics_ctx._1x_enabled = 1;
            zoom_enabled = &harmonics_ctx._1x_enabled;
            harmonics_ctx.zoom = 1;
            ctrls.harmonics_control->queue_draw ();
        }
        else if(harmonics_ctx._2x_highlighted)
        {
            *zoom_enabled = 0;
            harmonics_ctx._2x_enabled = 1;
            zoom_enabled = &harmonics_ctx._2x_enabled;
            harmonics_ctx.zoom = 2;
            ctrls.harmonics_control->queue_draw ();
        }
        else if(harmonics_ctx._4x_highlighted)
        {
            *zoom_enabled = 0;
            harmonics_ctx._4x_enabled = 1;
            zoom_enabled = &harmonics_ctx._4x_enabled;
            harmonics_ctx.zoom = 4;
            ctrls.harmonics_control->queue_draw ();
        }
        else if(harmonics_ctx._8x_highlighted)
        {
            *zoom_enabled = 0;
            harmonics_ctx._8x_enabled = 1;
            zoom_enabled = &harmonics_ctx._8x_enabled;
            harmonics_ctx.zoom = 8;
            ctrls.harmonics_control->queue_draw ();
        }
        else if(harmonics_ctx._16x_highlighted)
        {
            *zoom_enabled = 0;
            harmonics_ctx._16x_enabled = 1;
            zoom_enabled = &harmonics_ctx._16x_enabled;
            harmonics_ctx.zoom = 16;
            ctrls.harmonics_control->queue_draw ();
        }
        else if(harmonics_ctx._32x_highlighted)
        {
            *zoom_enabled = 0;
            harmonics_ctx._32x_enabled = 1;
            zoom_enabled = &harmonics_ctx._32x_enabled;
            harmonics_ctx.zoom = 32;
            ctrls.harmonics_control->queue_draw ();
        }
        else if(harmonics_ctx._64x_highlighted)
        {
            *zoom_enabled = 0;
            harmonics_ctx._64x_enabled = 1;
            zoom_enabled = &harmonics_ctx._64x_enabled;
            harmonics_ctx.zoom = 64;
            ctrls.harmonics_control->queue_draw ();
        }
        else if(harmonics_ctx.expr_highlighted)
        {
            auto dialogue = new Gtk::Dialog("Enter code", true);
            auto func_label = new Gtk::Label("function gen_harmonics ( cos_array, sin_array, num_bins ) {");
            auto expr_entry = new Gtk::TextView();
            auto close_brace_label = new Gtk::Label("}");
            auto expr_go = new Gtk::Button("Go!");

            expr_entry->get_buffer ()->set_text (active_table->last_expr);

            //TODO: Add another one of these for time domain

            func_label->set_xalign (GTK_ALIGN_FILL);
            close_brace_label->set_xalign (GTK_ALIGN_FILL);

            reinterpret_cast<Gtk::Box*>(dialogue->get_child ())->add (*func_label);
            reinterpret_cast<Gtk::Box*>(dialogue->get_child ())->add (*expr_entry);
            reinterpret_cast<Gtk::Box*>(dialogue->get_child ())->add (*close_brace_label);
            reinterpret_cast<Gtk::Box*>(dialogue->get_child ())->add (*expr_go);

            expr_go->signal_clicked ().connect ( sigc::bind<Gtk::Dialog*>(sigc::ptr_fun (&on_expr_go), dialogue) );

            expr_entry->set_visible (true);
            func_label->set_visible (true);
            close_brace_label->set_visible (true);
            expr_go->set_visible (true);
            dialogue->set_visible (true);
            dialogue->show();
        }
    }

    return false;
}

bool harmonics_mouse_release(GdkEventButton* buttonevent)
{
    if(buttonevent->button == 1)
    {
        harmonics_ctx.editing_cos_bins = 0;
        harmonics_ctx.editing_sin_bins = 0;
    }

    return false;
}


bool harmonics_mouse_move(GdkEventMotion* motionevent)
{
    int x = motionevent->x;
    int y = motionevent->y;
    int i;

    if(harmonics_ctx.editing_cos_bins && is_in_rect ( x, y, 3, 3, harmonics_ctx.harmonics_width - 6, (( harmonics_ctx.harmonics_height - 30 ) / 2 ) - 6 ) )
    {
        int effective_x = x - 3;
        int bin = (effective_x / harmonics_ctx.bin_width) + harmonics_ctx.harmonics_page * harmonics_ctx.bins_per_page;
        double val = (0.5 - ((double)(y - 3) / (((double)( harmonics_ctx.harmonics_height - 30 ) / 2.0f ) - 6))) * 2;

        val /= harmonics_ctx.zoom;

        if( bin < active_table->harmonics->num_harmonics && bin >= 0 && !(harmonics_ctx.even_lock && bin % 2 == 0) && !(harmonics_ctx.odd_lock && bin % 2) )
        {
            active_table->harmonics->cos_table[ bin ] = val;
            ctrls.harmonics_control->queue_draw ();
            ctrls.oscilloscope_control->queue_draw ();
        }
    }
    else if(harmonics_ctx.editing_sin_bins && is_in_rect ( x, y, 3, (( harmonics_ctx.harmonics_height - 30 ) / 2 ) + 3, harmonics_ctx.harmonics_width - 6, (( harmonics_ctx.harmonics_height - 30 ) / 2 ) - 6 ))
    {
        int effective_x = x - 3;
        int bin = (effective_x / harmonics_ctx.bin_width) + harmonics_ctx.harmonics_page * harmonics_ctx.bins_per_page;
        double val = (0.5 - ((double)(y - (((double)( harmonics_ctx.harmonics_height - 30 ) / 2.0f ) + 3)) / (((double)( harmonics_ctx.harmonics_height - 30 ) / 2.0f ) - 6))) * 2;

        val /= harmonics_ctx.zoom;

        if( bin < active_table->harmonics->num_harmonics && bin >= 0 && !(harmonics_ctx.even_lock && bin % 2 == 0) && !(harmonics_ctx.odd_lock && bin % 2) )
        {
            active_table->harmonics->sin_table[ bin ] = val;
            ctrls.harmonics_control->queue_draw ();
            ctrls.oscilloscope_control->queue_draw ();
        }
    }
    else
    {
        bool anyhighlighted = false;

        for(i = 0; i < NUM_HARMONICS_BTNS; i++)
        {
            if(is_in_rect(x, y, harmonic_btns[i].rect[0], harmonic_btns[i].rect[1], harmonic_btns[i].rect[2], harmonic_btns[i].rect[3]))
            {
                anyhighlighted = true;

                if(!*harmonic_btns[i].highlight)
                {
                    *harmonic_btns[i].highlight = 1;
                    ctrls.harmonics_control->queue_draw ();
                }
                break;
            }
        }

        if(!anyhighlighted)
        {
            anyhighlighted = false;

            for(i = 0; i < NUM_HARMONICS_BTNS; i++)
            {
                if(*harmonic_btns[i].highlight)
                {
                    *harmonic_btns[i].highlight = 0;
                    anyhighlighted = true;
                }
            }

            if(anyhighlighted) ctrls.harmonics_control->queue_draw ();
        }

    }

    return true;
}


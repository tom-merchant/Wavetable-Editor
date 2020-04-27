//
// Created by tmerchant on 06/04/2020.
//

#include <iostream>
#include "EnvelopeEditor.h"
#include "control_structures.h"
#include "util.h"

#ifndef CLAMP
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#endif

EnvelopeEditor::EnvelopeEditor ( envelope *env )
: env(env)
{

}

bool EnvelopeEditor::envelope_mouse_press ( GdkEventButton *event, EnvelopeEditor *e )
{
    breakpoint* newbp, *current;
    int j = 0;

    if(event->button == GDK_BUTTON_PRIMARY)
    {
        if ( event->state & GDK_CONTROL_MASK && e->phantom_point_visible )
        {
            e->editing_point_type = -1;
            newbp = (breakpoint *) calloc ( 1, sizeof ( breakpoint ));
            newbp->time = e->env->minTime + ( e->env->maxTime - e->env->minTime ) * ( event->x / e->get_width () );
            newbp->value = e->plotData[ (int) event->x ] / (float) e->get_height ();
            newbp->interpType = LINEAR;
            newbp->interpCallback = interp_functions[ LINEAR ];
            insert_breakpoint ( e->env, newbp );
            e->queue_draw ();
        }
        else if(e->draggingPoint == -1 && e->highlightedPoint != -1)
        {
            e->draggingPoint = e->highlightedPoint;
        }
        else if(e->editing_point_type != -1)
        {
            if(e->interp_button_highlighted != -1 && e->editing_point_type != -1)
            {
                for(current = e->env->first; current != nullptr; current = current->next, j++)
                {
                    if(j == e->editing_point_type)
                    {
                        current->interpType = (interp_t)e->interp_button_highlighted;
                        current->interpCallback = interp_functions [ e->interp_button_highlighted ];

                        // If the user selected Quadratic bezier we must supply a control point in the interp params
                        if ( current->interpType == QUADRATIC_BEZIER && current->nInterp_params < 2)
                        {
                            current->nInterp_params      = 2;
                            current->interp_params       = (double*) malloc ( sizeof ( double ) * 2 );
                            current->interp_params [ 0 ] = (current->time + (current->next ? current->next->time : current->time)) / 2;
                            current->interp_params [ 1 ] = (current->value + (current->next ? current->next->value : current->value)) / 2;
                        }

                        e->updatePlot = true;
                        e->editing_point_type = -1;
                        e->interp_button_highlighted = -1;
                        e->queue_draw ();
                        return false;
                    }

                    if(current->interpType == QUADRATIC_BEZIER) j++;
                }
            }

            e->editing_point_type = -1;
            e->interp_button_highlighted = -1;
            e->queue_draw ();
        }
    }
    else if (event->button == GDK_BUTTON_SECONDARY)
    {
        if(e->highlightedPoint != - 1)
        {
            if(e->draggingPoint != -1) e->draggingPoint = -1;

            if(e->highlightedPoint == e->editing_point_type)
            {
                e->editing_point_type = -1;
            }
            else
            {
                e->editing_point_type = e->highlightedPoint;
            }

            e->queue_draw ();
        }
    }

    return false;
}

bool EnvelopeEditor::envelope_mouse_release (GdkEventButton *event, EnvelopeEditor* e)
{
    if(event->button == GDK_BUTTON_PRIMARY)
    {
        e->draggingPoint = -1;
    }

    return false;
}

bool EnvelopeEditor::envelope_mouse_leave ( GdkEventCrossing *, EnvelopeEditor *e )
{
    e->draggingPoint = -1;
    e->phantom_point_visible = false;
    e->phantom_point_x = 0;
    e->highlightedPoint = -1;
    e->interp_button_highlighted = -1;
    e->queue_draw ();
}

bool EnvelopeEditor::envelope_mouse_move (GdkEventMotion *event, EnvelopeEditor* e)
{
    int i, j = 0;
    double x, y, clampX1 = e->env->minTime, clampX2 = e->env->maxTime, time, value;
    breakpoint* current;
    breakpoint* last = nullptr;
    bool anyPointHighlighted = false;
    bool anyButtonHighlighted = false;

    e->mousex = event->x;
    e->mousey = event->y;

    if(e->draggingPoint != -1)
    {
        for(current = e->env->first; current != nullptr; last = current, current = current->next, j++)
        {
            if(e->draggingPoint == j)
            {
                if(last)
                {
                    clampX1 = last->time;
                }

                if(current->next)
                {
                    clampX2 = current->next->time;
                }

                time = e->env->minTime + ( e->env->maxTime - e->env->minTime ) * ( event->x / e->get_width () );
                value = e->env->maxVal - (e->env->minVal + ( e->env->maxVal - e->env->minVal ) * ( event->y / e->get_height () ));
                time = CLAMP (time, clampX1, clampX2);

                current->time = time;
                current->value = value;
                e->updatePlot = true;
                e->queue_draw ();
                break;
            }

            if(current->interpType == QUADRATIC_BEZIER)
            for(i = 0, j++; i < current->nInterp_params / 2; i+=2)
            {
                if(e->draggingPoint == j)
                {
                    current->interp_params[ i ] = e->env->minTime + ( e->env->maxTime - e->env->minTime ) * ( event->x / e->get_width ());
                    current->interp_params[ i + 1 ] = e->env->maxVal - (e->env->minVal + ( e->env->maxVal - e->env->minVal ) * ( event->y / e->get_height ()));
                    e->updatePlot = true;
                    e->queue_draw ();
                }
            }
        }
    }
    else if(event->state & GDK_CONTROL_MASK)
    {
        e->phantom_point_visible = true;
        e->phantom_point_x = event->x;
        e->queue_draw ();
    }
    else
    {
        e->phantom_point_visible = false;
        e->queue_draw ();

        for(current = e->env->first; current != nullptr; current = current->next, j++)
        {
            x = (float)( (current->time - e->env->minTime) / (e->env->maxTime - e->env->minTime) ) * (float)e->get_width ();
            y = (float)e->get_height () - (( (current->value - e->env->minVal) / (e->env->maxVal - e->env->minVal)   ) * (float)e->get_height ());

            if( sqrt ( pow ( event->x - x, 2 ) + pow ( event->y - y, 2 ) ) <= 2 * e->lineThickness  * e->dpi)
            {
                e->highlightedPoint = j;
                anyPointHighlighted = true;
                e->queue_draw ();
            }

            if(j == e->editing_point_type)
            {
                x = ((current->time - e->env->minTime) / (e->env->maxTime - e->env->minTime)) * e->get_width ();
                y =  e->get_height () - (((current->value - e->env->minVal) / (e->env->maxVal - e->env->minVal)) * e->get_height ());

                if(is_in_rect(event->x, event->y, x, y, 1.5 * e->dpi, (3.0/16) * e->dpi))
                {
                    e->interp_button_highlighted = 0;
                    anyButtonHighlighted = true;
                }
                else if(is_in_rect(event->x, event->y, x, y + (3.0/16) * e->dpi, 1.5 * e->dpi, (3.0/16) * e->dpi))
                {
                    e->interp_button_highlighted = 1;
                    anyButtonHighlighted = true;
                }
                else if(is_in_rect(event->x, event->y, x, y + (6.0/16) * e->dpi, 1.5 * e->dpi, (3.0/16) * e->dpi))
                {
                    e->interp_button_highlighted = 2;
                    anyButtonHighlighted = true;
                }
                else if(is_in_rect(event->x, event->y, x, y + (9.0/16) * e->dpi, 1.5 * e->dpi, (3.0/16) * e->dpi))
                {
                    e->interp_button_highlighted = 3;
                    anyButtonHighlighted = true;
                }
            }

            if(current->interpType == QUADRATIC_BEZIER)
            for(i = 0, j++; i < current->nInterp_params / 2; i+=2)
            {
                x = (float)( (current->interp_params[ i ]   - e->env->minTime) / (e->env->maxTime - e->env->minTime) ) * (float)e->get_width ();
                y = (float)e->get_height () - (( (current->interp_params[i + 1] - e->env->minVal ) / (e->env->maxVal - e->env->minVal)   ) * (float)e->get_height ());

                if( sqrt ( pow ( event->x - x, 2 ) + pow ( event->y - y, 2 ) ) <= 2 * e->lineThickness * e->dpi)
                {
                    e->highlightedPoint = j;
                    anyPointHighlighted = true;
                    e->queue_draw ();
                }
            }
        }

        if(!anyPointHighlighted)
        {
            e->highlightedPoint = -1;
        }

        if(!anyButtonHighlighted)
        {
            e->interp_button_highlighted = -1;
        }
    }

    return false;
}

bool EnvelopeEditor::on_draw ( const Cairo::RefPtr<Cairo::Context> &ctx )
{
    int i, j = 0;
    double x, y;
    breakpoint* current;

    if ( ! env )
    {
        return false;
    }

    if(init)
    {
        get_window ()->set_events ( get_window ()->get_events () | Gdk::BUTTON_RELEASE_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::POINTER_MOTION_MASK | Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK | Gdk::LEAVE_NOTIFY_MASK );
        get_parent_window ()->set_title ("Edit envelope");
        signal_button_press_event ().connect (sigc::bind<EnvelopeEditor*>(sigc::ptr_fun (&envelope_mouse_press), this));
        signal_button_release_event ().connect (sigc::bind<EnvelopeEditor*>(sigc::ptr_fun (&envelope_mouse_release), this));
        signal_motion_notify_event ().connect (sigc::bind<EnvelopeEditor*>(sigc::ptr_fun (&envelope_mouse_move), this));
        signal_leave_notify_event ().connect (sigc::bind<EnvelopeEditor*>(sigc::ptr_fun(&envelope_mouse_leave), this));
        init = false;
    }

    if ( updatePlot )
    {
        if ( ! plotData )
        {
            plotData = (float*) calloc ( this->get_width (), sizeof ( float ) );
        }
        else
        {
            free ( plotData );
            plotData = (float*) calloc ( this->get_width (), sizeof ( float ) );
        }

        plot_envelope ( env, (int)this->get_width (), (int)this->get_height (), plotData );

        for(i = 0; i < get_width(); i++)
        {
            plotData[i] = get_height() - plotData[i];
        }

        updatePlot = false;
    }

    ctx->set_source_rgba (bgColour[0] / 255.0, bgColour[1] / 255.0, bgColour[2] / 255.0, 0);
    ctx->rectangle (0, 0, get_width (), get_height ());
    ctx->fill ();

    ctx->set_line_width (lineThickness * dpi);
    ctx->set_source_rgb (fgColour[0] / 255.0, fgColour[1] / 255.0, fgColour[2] / 255.0);

    for ( i = 0; i < get_width (); i++ )
    {
        ctx->line_to ( i, plotData [ i ] );
    }

    ctx->stroke ();

    for(current = env->first; current != nullptr; current = current->next, j++)
    {
        x = ((current->time - env->minTime) / (env->maxTime - env->minTime)) * get_width ();
        y =  get_height () - (((current->value - env->minVal) / (env->maxVal - env->minVal)) * get_height ());

        if(draggingPoint == j || highlightedPoint == j)
        {
            ctx->fill();
            ctx->set_source_rgb (fgColour2[0] / 255.0, fgColour2[1] / 255.0, fgColour2[2] / 255.0);
        }

        ctx->arc (x, y, lineThickness * 2 * dpi, 0, M_PI * 2);
        ctx->begin_new_sub_path ();

        if(draggingPoint == j || highlightedPoint == j)
        {
            ctx->fill ();
            ctx->set_source_rgb (fgColour[0] / 255.0, fgColour[1] / 255.0, fgColour[2] / 255.0);
        }

        if(current->interpType == QUADRATIC_BEZIER)
        for(i = 0, j++; i < current->nInterp_params / 2; i+=2)
        {
            if(draggingPoint == j || highlightedPoint == j)
            {
                ctx->fill();
                ctx->set_source_rgb (fgColour2[0] / 255.0, fgColour2[1] / 255.0, fgColour2[2] / 255.0);
            }

            x = ((current->interp_params[i] - env->minTime) / (env->maxTime - env->minTime)) * get_width ();
            y = get_height () - (((current->interp_params[i + 1] - env->minVal) / (env->maxVal - env->minVal)) * get_height ());

            ctx->arc (x, y, lineThickness * 2 * dpi, 0, M_PI * 2);

            if(draggingPoint == j || highlightedPoint == j)
            {
                ctx->fill ();
                ctx->set_source_rgb (fgColour[0] / 255.0, fgColour[1] / 255.0, fgColour[2] / 255.0);
            }
        }
    }

    ctx->fill();

    if(phantom_point_visible)
    {
        ctx->set_source_rgb (fgColour2[0] / 255.0, fgColour2[1] / 255.0, fgColour2[2] / 255.0);
        ctx->set_line_width (lineThickness * dpi / 2);

        ctx->arc (phantom_point_x, plotData[phantom_point_x], lineThickness * 2 * dpi, 0, M_PI * 2);
        ctx->rectangle ( mousex + 10, mousey, 1 * dpi, 0.5 * dpi );
        ctx->begin_new_sub_path ();

        std::stringstream timetext;
        std::stringstream valuetext;

        timetext  << "time:  " << env->minTime + (env->maxTime - env->minTime) * ((double)phantom_point_x / get_width ());
        valuetext << "value: " << 1 - (plotData[phantom_point_x] / (float)get_height ());

        //TODO: Maybe move the info box to the left if it goes off the screen

        ctx->set_font_size (12);

        ctx->line_to (mousex + 10 + 0.125 * dpi, mousey + 0.25 * dpi);
        ctx->show_text (timetext.str ().substr (0, 12));
        ctx->begin_new_sub_path ();

        ctx->line_to (mousex + 10 + 0.125 * dpi, mousey + 0.375 * dpi);
        ctx->show_text (valuetext.str ().substr (0, 12));
        ctx->begin_new_sub_path ();

        ctx->stroke ();
        ctx->set_source_rgb (fgColour[0] / 255.0, fgColour[1] / 255.0, fgColour[2] / 255.0);
        ctx->set_line_width (lineThickness * dpi);
    }

    if(editing_point_type != -1)
    {
        j = 0;

        for(current = env->first; current != nullptr; current = current->next, j++)
        {
            if(j == editing_point_type)
            {
                ctx->set_line_width (1.5);
                x = ((current->time - env->minTime) / (env->maxTime - env->minTime)) * get_width ();
                y =  get_height () - (((current->value - env->minVal) / (env->maxVal - env->minVal)) * get_height ());

                ctx->set_source_rgb (fgColour3[0] / 255.0, fgColour3[1] / 255.0, fgColour3[2] / 255.0);
                ctx->rectangle (x, y, 1.5 * dpi, (9.0/16) * dpi);
                ctx->fill ();
                ctx->set_source_rgb (fgColour2[0] / 255.0, fgColour2[1] / 255.0, fgColour2[2] / 255.0);
                ctx->rectangle (x, y, 1.5 * dpi, (12.0/16) * dpi);
                ctx->begin_new_sub_path ();
                ctx->line_to (x, y + (3.0/16) * dpi);
                ctx->line_to (x + 1.5 * dpi, y + (3.0/16) * dpi);
                ctx->begin_new_sub_path ();
                ctx->line_to (x, y + (6.0/16) * dpi);
                ctx->line_to (x + 1.5 * dpi, y + (6.0/16) * dpi);
                ctx->stroke();
                ctx->begin_new_sub_path ();
                ctx->line_to (x, y + (9.0/16) * dpi);
                ctx->line_to (x + 1.5 * dpi, y + (9.0/16) * dpi);
                ctx->stroke();

                if(interp_button_highlighted == 0)ctx->set_source_rgb (fgColour[0] / 255.0, fgColour[1] / 255.0, fgColour[2] / 255.0);
                ctx->set_font_size (12);
                ctx->line_to (x + (1.0/16) * dpi, y + (2.2 / 16) * dpi);
                ctx->show_text (interpTypeStrings[0]);
                if(interp_button_highlighted == 0)ctx->set_source_rgb (fgColour2[0] / 255.0, fgColour2[1] / 255.0, fgColour2[2] / 255.0);
                if(interp_button_highlighted == 1)ctx->set_source_rgb (fgColour[0] / 255.0, fgColour[1] / 255.0, fgColour[2] / 255.0);
                ctx->line_to (x + (1.0/16) * dpi, y + (5.2 / 16) * dpi);
                ctx->show_text (interpTypeStrings[1]);
                if(interp_button_highlighted == 1)ctx->set_source_rgb (fgColour2[0] / 255.0, fgColour2[1] / 255.0, fgColour2[2] / 255.0);
                if(interp_button_highlighted == 2)ctx->set_source_rgb (fgColour[0] / 255.0, fgColour[1] / 255.0, fgColour[2] / 255.0);
                ctx->line_to (x + (1.0/16) * dpi, y + (8.2 / 16) * dpi);
                ctx->show_text (interpTypeStrings[2]);
                if(interp_button_highlighted == 2)ctx->set_source_rgb (fgColour2[0] / 255.0, fgColour2[1] / 255.0, fgColour2[2] / 255.0);
                if(interp_button_highlighted == 3)ctx->set_source_rgb (fgColour[0] / 255.0, fgColour[1] / 255.0, fgColour[2] / 255.0);
                ctx->line_to (x + (1.0/16) * dpi, y + (11.2 / 16) * dpi);
                ctx->show_text (interpTypeStrings[3]);
            }

            if(current->interpType == QUADRATIC_BEZIER) j++;
        }
    }
    return false;
}

EnvelopeEditor::~EnvelopeEditor ( )
{
    if ( plotData )
    {
        free ( plotData );
    }
}

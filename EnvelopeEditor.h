//
// Created by tmerchant on 06/04/2020.
//

#ifndef WAVETABLE_EDITOR_ENVELOPEEDITOR_H
#define WAVETABLE_EDITOR_ENVELOPEEDITOR_H

#include <envelope.h>
#include <string>
#include <gtkmm/drawingarea.h>


class EnvelopeEditor : public Gtk::DrawingArea
{
public:
    explicit EnvelopeEditor ( envelope* env );
    ~EnvelopeEditor ( ) override;
    float dpi = 96;
protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& ctx) override;
    static bool envelope_mouse_press (GdkEventButton *event, EnvelopeEditor* e);
    static bool envelope_mouse_release (GdkEventButton *event, EnvelopeEditor* e);
    static bool envelope_mouse_move (GdkEventMotion *event, EnvelopeEditor* e);
    static bool envelope_mouse_leave (GdkEventCrossing *event, EnvelopeEditor* e);
    envelope* env = nullptr;
    float lineThickness = 1/32.0f;
    //TODO: add support for negative min env values, float axisHeight = 0;
    //TODO: Maybe also draw gradations and axis values
    const std::string interpTypeStrings [ 4 ] = { "Linear", "Nearest Neighbour", "Quadratic Bezier", "Exponential" };
    unsigned char bgColour[3] =  { 0, 0, 0 };
    unsigned char fgColour[3]  =  { 187, 12, 249 };
    unsigned char fgColour2[3] =  { 255, 255, 255 };
    unsigned char fgColour3[3] =  { 33, 33, 33 };
    int mousex = 0;
    int mousey = 0;
private:
    float* plotData = nullptr;
    bool updatePlot = true;
    bool init = true;
    int draggingPoint = -1;
    int highlightedPoint = -1;
    int editing_point_type = -1;
    int interp_button_highlighted = -1;
    bool phantom_point_visible = false;
    int phantom_point_x = 0;
};

#endif //WAVETABLE_EDITOR_ENVELOPEEDITOR_H

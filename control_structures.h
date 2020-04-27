//
// Created by tmerchant on 04/04/2020.
//

#ifndef WAVETABLE_EDITOR_CONTROL_STRUCTURES_H
#define WAVETABLE_EDITOR_CONTROL_STRUCTURES_H

#include "mujs/mujs.h"

#include <gtkmm.h>

extern "C"
{
#include "wavetable.h"
}

typedef struct wavetable_morph_control_row
{
    wavetable_morph_control_row *next;
    struct wavetable_control_row *wavetable;
    Gtk::ListBoxRow *thisrow;
    Gtk::CheckButton *include_btn;
    Gtk::Entry *morph_order;
    int index;
} wavetable_morph_control_row;

typedef struct wavetable_control_row
{
    wavetable_control_row *next;
    wavetable_morph_control_row *morph_row;
    Gtk::Dialog *dialogue_ptr;
    Gtk::ListBoxRow *thisrow;
    Gtk::Button *select_btn;
    Gtk::Button *rename_btn;
    Gtk::Button *save_btn;
    Gtk::Button *delete_btn;
    int index;
    int label_index;
    int oversample;
    int tables_per_octave;

    wavetable_harmonics* harmonics;
    wavetable_mipmap* mipmap;

    std::string last_expr;

} wavetable_control_row;

typedef struct global_controls
{
    Glib::RefPtr<Gtk::Application> app;
    Gtk::Window *window;
    Gtk::ComboBox *morph_combobox;
    Gtk::DrawingArea *harmonics_control;
    Gtk::DrawingArea *oscilloscope_control;
    Gtk::GLArea *wavetable_morph_preview;
    Gtk::ListBox *morph_wavetable_selection_listbox;
    Gtk::ListBox *wavetable_menu;
    Gtk::ListBoxRow *wavetable_menu_ctrl_row;
    Gtk::ImageMenuItem *mnu_btn_quit;
    Gtk::Button *generate_morph_table;
    Gtk::Button *reset_morph_controls;
    Gtk::Button *add_wavetable;
    Gtk::Button *load_wavetable;
    Gtk::Button *save_morph_table;
    Gtk::Button *save_morph_table_as;
    Gtk::Entry *wavetable_harmonics_entry;
    Gtk::Entry *wavetable_oversample_entry;
    Gtk::Entry *wavetable_tables_per_octave_entry;
    Gtk::Entry *morph_num_harmonics_entry;
    Gtk::Entry *morph_oversample_entry;
    Gtk::Entry *morph_tables_per_octave_entry;
    Gtk::Entry *morph_tables_per_morph_entry;
    Gtk::ImageMenuItem *file_save;
    Gtk::ImageMenuItem *file_save_as;
    Gtk::ImageMenuItem *file_new;
    Gtk::ImageMenuItem *file_open;
    envelope* morph_envelope;
    morph_wavetable_harmonics morph_harmonics = {};
    morph_wavetable_mipmap *mwm;
    std::string morph_save_path;
    std::string project_save_path;
    js_State *js;
} global_controls;

extern global_controls ctrls;
extern wavetable_morph_control_row first_morph_row;
extern wavetable_control_row *active_table;

void update_wavetable_controls();

#endif //WAVETABLE_EDITOR_CONTROL_STRUCTURES_H

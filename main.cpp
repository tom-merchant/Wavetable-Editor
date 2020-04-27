#include <iostream>

#include <gtkmm.h>

extern "C"
{
#include "wavetable.h"
#include "envelope.h"
#include "AudioUtils.h"
}

#include "fftw3.h"
#include "mujs/mujs.h"

#include "control_structures.h"
#include "morph_preview.h"
#include "oscilloscope_control.h"
#include "harmonics_control.h"
#include "EnvelopeEditor.h"

// TODO: Load WAV files

global_controls ctrls;

wavetable_control_row first_row;
wavetable_control_row *last_row = &first_row;
wavetable_control_row *active_table = &first_row;

wavetable_morph_control_row first_morph_row;
wavetable_morph_control_row *last_morph_row = &first_morph_row;

void on_quit();
void on_add_wavetable();
void on_load_wavetable();
void on_select_wavetable(wavetable_control_row* row);
void on_delete_wavetable(wavetable_control_row* row);
void on_save_wavetable(wavetable_control_row* row);
void on_save_morph_wavetable();
void on_save_morph_wavetable_as();
void on_rename_wavetable(wavetable_control_row* row);
void on_set_oversample();
void on_set_tables_per_octave();
void on_wavetable_morph_order_changed(wavetable_morph_control_row *row);
void on_harmonics_change();
void on_morph_type_selected();
void on_project_save(); //TODO: Implement this save everything e.g code for wavetables
void on_project_save_as(); //TODO: Implement this
void on_project_open();
void init_gl();


int main( int argc, char **argv)
{
    ctrls.app = Gtk::Application::create(argc, argv, "com.tmerchant.wavetable_editor");

    auto ui = Gtk::Builder::create_from_file("wavetable_editor_ui.glade");
    auto builder = ui.get ();

    builder->get_widget ( "mainwindow", reinterpret_cast<Gtk::Window *&>(ctrls.window) );
    builder->get_widget ( "morph_combobox", reinterpret_cast<Gtk::ComboBox *&>(ctrls.morph_combobox) );
    builder->get_widget ( "harmonics_control", reinterpret_cast<Gtk::DrawingArea *&>(ctrls.harmonics_control) );
    builder->get_widget ( "oscilloscope_control", reinterpret_cast<Gtk::DrawingArea *&>(ctrls.oscilloscope_control) );
    builder->get_widget ( "wavetable_morph_preview", reinterpret_cast<Gtk::GLArea *&>(ctrls.wavetable_morph_preview) );
    builder->get_widget ( "morph_wavetable_selection_listbox", reinterpret_cast<Gtk::ListBox *&>(ctrls.morph_wavetable_selection_listbox) );
    builder->get_widget ( "wt0_morph_control_row", reinterpret_cast<Gtk::ListBoxRow *&>(first_morph_row.thisrow) );
    builder->get_widget ( "wavetable_menu", reinterpret_cast<Gtk::ListBox *&>(ctrls.wavetable_menu) );
    builder->get_widget ( "wavetable_menu_ctrl_row", reinterpret_cast<Gtk::ListBoxRow *&>(ctrls.wavetable_menu_ctrl_row) );
    builder->get_widget ( "wavetable_control_row_0", reinterpret_cast<Gtk::ListBoxRow *&>(first_row.thisrow) );
    builder->get_widget ( "mnu_btn_quit", reinterpret_cast<Gtk::ImageMenuItem *&>(ctrls.mnu_btn_quit) );

    builder->get_widget ( "generate_morph_table", reinterpret_cast<Gtk::Button *&>(ctrls.generate_morph_table) );
    builder->get_widget ( "reset_morph_controls", reinterpret_cast<Gtk::Button *&>(ctrls.reset_morph_controls) );
    builder->get_widget ( "add_wavetable", reinterpret_cast<Gtk::Button *&>(ctrls.add_wavetable) );
    builder->get_widget ( "load_wavetable", reinterpret_cast<Gtk::Button *&>(ctrls.load_wavetable) );
    builder->get_widget ( "save_morph_table", reinterpret_cast<Gtk::Button *&>(ctrls.save_morph_table) );
    builder->get_widget ( "save_morph_table_as", reinterpret_cast<Gtk::Button *&>(ctrls.save_morph_table_as) );

    builder->get_widget ( "wt0btn", reinterpret_cast<Gtk::Button *&>(first_row.select_btn) );
    builder->get_widget ( "wt0rename", reinterpret_cast<Gtk::Button *&>(first_row.rename_btn) );
    builder->get_widget ( "wt0save", reinterpret_cast<Gtk::Button *&>(first_row.save_btn) );
    builder->get_widget ( "wt0delete", reinterpret_cast<Gtk::Button *&>(first_row.delete_btn) );

    builder->get_widget ( "wt0_morph_include", reinterpret_cast<Gtk::CheckButton *&>(first_morph_row.include_btn) );
    builder->get_widget ( "wt0_morph_order", reinterpret_cast<Gtk::Entry *&>(first_morph_row.morph_order) );

    builder->get_widget ( "wavetable_harmonics_entry", reinterpret_cast<Gtk::Entry *&>(ctrls.wavetable_harmonics_entry) );
    builder->get_widget ( "wavetable_oversample_entry", reinterpret_cast<Gtk::Entry *&>(ctrls.wavetable_oversample_entry) );
    builder->get_widget ( "wavetable_tables_per_octave_entry", reinterpret_cast<Gtk::Entry *&>(ctrls.wavetable_tables_per_octave_entry) );
    builder->get_widget ( "morph_num_harmonics_entry", reinterpret_cast<Gtk::Entry *&>(ctrls.morph_num_harmonics_entry) );
    builder->get_widget ( "morph_oversample_entry", reinterpret_cast<Gtk::Entry *&>(ctrls.morph_oversample_entry) );
    builder->get_widget ( "morph_tables_per_octave_entry", reinterpret_cast<Gtk::Entry *&>(ctrls.morph_tables_per_octave_entry) );
    builder->get_widget ( "morph_tables_per_morph_entry", reinterpret_cast<Gtk::Entry *&>(ctrls.morph_tables_per_morph_entry) );

    builder->get_widget ( "file_save", reinterpret_cast<Gtk::ImageMenuItem *&>(ctrls.file_save) );
    builder->get_widget ( "file_save_as", reinterpret_cast<Gtk::ImageMenuItem *&>(ctrls.file_save_as) );
    builder->get_widget ( "file_new", reinterpret_cast<Gtk::ImageMenuItem *&>(ctrls.file_new) );
    builder->get_widget ( "file_open", reinterpret_cast<Gtk::ImageMenuItem *&>(ctrls.file_open) );

    ctrls.add_wavetable->signal_clicked ().connect ( sigc::ptr_fun(&on_add_wavetable) );
    ctrls.load_wavetable->signal_clicked ().connect ( sigc::ptr_fun(&on_load_wavetable) );

    ctrls.mnu_btn_quit->signal_activate ().connect ( sigc::ptr_fun(&on_quit) );

    first_row.select_btn->signal_clicked ().connect ( sigc::bind<wavetable_control_row*>(sigc::ptr_fun (&on_select_wavetable), &first_row) );
    first_row.rename_btn->signal_clicked ().connect ( sigc::bind<wavetable_control_row*>(sigc::ptr_fun (&on_rename_wavetable), &first_row) );
    first_row.save_btn->signal_clicked ().connect ( sigc::bind<wavetable_control_row*>(sigc::ptr_fun (&on_save_wavetable), &first_row) );

    first_row.index = 0;
    first_row.label_index = 0;
    first_row.harmonics = (wavetable_harmonics*)malloc(sizeof(wavetable_harmonics));
    first_row.harmonics->num_harmonics = (int)strtol(ctrls.wavetable_harmonics_entry->get_text ().c_str (), nullptr, 10);
    first_row.oversample = (int)strtol(ctrls.wavetable_oversample_entry->get_text ().c_str (), nullptr, 10);
    first_row.tables_per_octave = (int)strtol(ctrls.wavetable_tables_per_octave_entry->get_text ().c_str (), nullptr, 10);
    first_row.harmonics->cos_table = (double*)calloc(first_row.harmonics->num_harmonics, sizeof(double));
    first_row.harmonics->sin_table = (double*)calloc(first_row.harmonics->num_harmonics, sizeof(double));
    first_row.harmonics->sin_table[0] = 1;

    first_row.last_expr = "";

    first_row.morph_row = &first_morph_row;
    first_morph_row.wavetable = &first_row;

    first_morph_row.index = 0;

    first_morph_row.morph_order->signal_changed ().connect ( sigc::bind<wavetable_morph_control_row*>(sigc::ptr_fun ( &on_wavetable_morph_order_changed ), &first_morph_row) );

    ctrls.harmonics_control->signal_draw ().connect ( sigc::ptr_fun(&draw_harmonics_control) );
    ctrls.harmonics_control->get_window ()->set_events (ctrls.harmonics_control->get_window ()->get_events () | Gdk::EventMask::POINTER_MOTION_MASK | Gdk::EventMask::LEAVE_NOTIFY_MASK | Gdk::EventMask::BUTTON_PRESS_MASK | Gdk::EventMask::BUTTON_RELEASE_MASK);
    ctrls.harmonics_control->signal_motion_notify_event ().connect ( sigc::ptr_fun(&harmonics_mouse_move) );
    ctrls.harmonics_control->signal_button_press_event ().connect ( sigc::ptr_fun(&harmonics_mouse_press) );
    ctrls.harmonics_control->signal_button_release_event ().connect ( sigc::ptr_fun(&harmonics_mouse_release) );
    ctrls.harmonics_control->signal_leave_notify_event ().connect ( sigc::ptr_fun(&harmonics_mouse_leave) );

    ctrls.oscilloscope_control->signal_draw ().connect ( sigc::ptr_fun(&draw_oscilloscope_control) );
    ctrls.oscilloscope_control->get_window ()->set_events (ctrls.oscilloscope_control->get_window ()->get_events () | Gdk::EventMask::POINTER_MOTION_MASK | Gdk::EventMask::LEAVE_NOTIFY_MASK | Gdk::EventMask::BUTTON_PRESS_MASK | Gdk::EventMask::BUTTON_RELEASE_MASK);
    ctrls.oscilloscope_control->signal_motion_notify_event ().connect ( sigc::ptr_fun(&oscilloscope_mouse_move) );
    ctrls.oscilloscope_control->signal_button_press_event ().connect ( sigc::ptr_fun(&oscilloscope_mouse_press) );
    ctrls.oscilloscope_control->signal_button_release_event ().connect ( sigc::ptr_fun(&oscilloscope_mouse_release) );
    ctrls.oscilloscope_control->signal_leave_notify_event ().connect ( sigc::ptr_fun(&oscilloscope_mouse_leave) );


    ctrls.wavetable_morph_preview->signal_render ().connect ( sigc::ptr_fun (&draw_morph_preview_control) );
    ctrls.wavetable_morph_preview->signal_realize ().connect( sigc::ptr_fun (&init_gl) );

    ctrls.wavetable_harmonics_entry->signal_activate ().connect ( sigc::ptr_fun(&on_harmonics_change) );

    ctrls.wavetable_oversample_entry->signal_changed ().connect ( sigc::ptr_fun(&on_set_oversample) );
    ctrls.wavetable_tables_per_octave_entry->signal_changed ().connect ( sigc::ptr_fun(&on_set_tables_per_octave) );

    ctrls.file_save->signal_activate ().connect ( sigc::ptr_fun (&on_project_save) );
    ctrls.file_save->signal_activate ().connect ( sigc::ptr_fun (&on_project_save_as) );
    ctrls.file_open->signal_activate ().connect ( sigc::ptr_fun (&on_project_open) );
    //ctrls.file_new->signal_activate ().connect ( sigc::ptr_fun (&on_new_project) );

    ctrls.morph_combobox->signal_changed ().connect ( sigc::ptr_fun ( &on_morph_type_selected ) );
    ctrls.generate_morph_table->signal_clicked ().connect ( sigc::ptr_fun ( &on_generate_morph_table ) );

    ctrls.save_morph_table->signal_clicked ().connect (sigc::ptr_fun (&on_save_morph_wavetable));
    ctrls.save_morph_table_as->signal_clicked ().connect (sigc::ptr_fun (&on_save_morph_wavetable_as));

    ctrls.js = js_newstate(nullptr, nullptr, JS_STRICT);

    return ctrls.app->run(*ctrls.window);
}

void on_add_wavetable()
{
    wavetable_control_row *new_row;
    Gtk::Box *hbox;

    new_row = (wavetable_control_row*)calloc ( 1, sizeof(wavetable_control_row) );
    new(&new_row->last_expr) std::string("");

    new_row->index = last_row->index + 1;
    new_row->label_index = last_row->label_index + 1;

    std::stringstream button_label;

    button_label << "wavetable" << (new_row->label_index + 1);

    new_row->select_btn = new Gtk::Button( button_label.str() );
    new_row->save_btn = new Gtk::Button( "save" );
    new_row->delete_btn = new Gtk::Button( "delete" );
    new_row->rename_btn = new Gtk::Button( "rename" );

    new_row->select_btn->signal_clicked ().connect ( sigc::bind<wavetable_control_row*>(sigc::ptr_fun (&on_select_wavetable), new_row) );
    new_row->delete_btn->signal_clicked ().connect ( sigc::bind<wavetable_control_row*>(sigc::ptr_fun (&on_delete_wavetable), new_row) );
    new_row->save_btn->signal_clicked ().connect ( sigc::bind<wavetable_control_row*>(sigc::ptr_fun (&on_save_wavetable), new_row) );
    new_row->rename_btn->signal_clicked ().connect ( sigc::bind<wavetable_control_row*>(sigc::ptr_fun (&on_rename_wavetable), new_row) );

    hbox = new Gtk::Box(Gtk::Orientation::ORIENTATION_HORIZONTAL, 0);

    hbox->add (*new_row->select_btn);
    hbox->add (*new_row->rename_btn);
    hbox->add (*new_row->save_btn);
    hbox->add (*new_row->delete_btn);

    new_row->rename_btn->set_hexpand (true);
    new_row->rename_btn->set_vexpand (true);
    new_row->save_btn->set_hexpand (true);
    new_row->save_btn->set_vexpand (true);
    new_row->delete_btn->set_hexpand (true);
    new_row->delete_btn->set_vexpand (true);
    new_row->select_btn->set_halign (Gtk::ALIGN_FILL);
    new_row->select_btn->set_valign (Gtk::ALIGN_FILL);
    new_row->rename_btn->set_halign (Gtk::ALIGN_FILL);
    new_row->rename_btn->set_valign (Gtk::ALIGN_FILL);
    new_row->save_btn->set_halign (Gtk::ALIGN_FILL);
    new_row->save_btn->set_valign (Gtk::ALIGN_FILL);
    new_row->delete_btn->set_halign (Gtk::ALIGN_FILL);
    new_row->delete_btn->set_valign (Gtk::ALIGN_FILL);

    new_row->thisrow = new Gtk::ListBoxRow();
    new_row->thisrow->add (*hbox);

    ctrls.wavetable_menu->insert (*new_row->thisrow, last_row->index + 1);

    new_row->thisrow->set_visible (true);
    new_row->select_btn->set_visible (true);
    new_row->rename_btn->set_visible (true);
    new_row->save_btn->set_visible (true);
    new_row->delete_btn->set_visible (true);
    hbox->set_visible (true);

    last_row->next = new_row;
    last_row = new_row;

    new_row->oversample = (int)strtol(ctrls.wavetable_oversample_entry->get_text ().c_str (), nullptr, 10);
    new_row->tables_per_octave = (int)strtol(ctrls.wavetable_tables_per_octave_entry->get_text ().c_str (), nullptr, 10);

    new_row->harmonics = (wavetable_harmonics*)malloc(sizeof(wavetable_harmonics));
    new_row->harmonics->num_harmonics = (int)strtol(ctrls.wavetable_harmonics_entry->get_text ().c_str (), nullptr, 10);
    new_row->harmonics->cos_table = (double*)calloc(new_row->harmonics->num_harmonics, sizeof(double));
    new_row->harmonics->sin_table = (double*)calloc(new_row->harmonics->num_harmonics, sizeof(double));
    new_row->harmonics->sin_table[0] = 1;

    wavetable_morph_control_row *new_morph_row;

    new_morph_row = (wavetable_morph_control_row*)calloc(1, sizeof(wavetable_morph_control_row));
    new_row->morph_row = new_morph_row;
    new_morph_row->wavetable = new_row;

    new_morph_row->index = last_morph_row->index + 1;

    std::stringstream morph_button_label, morph_entry_text;

    morph_button_label << "Include " << button_label.str ();
    morph_entry_text << new_morph_row->index + 1;

    Gtk::Label *order_label;
    new_morph_row->include_btn = new Gtk::CheckButton(morph_button_label.str ());
    order_label = new Gtk::Label("order: ");
    new_morph_row->morph_order = new Gtk::Entry();

    new_morph_row->morph_order->set_width_chars (3);
    new_morph_row->morph_order->set_text (morph_entry_text.str ());
    new_morph_row->morph_order->signal_changed().connect ( sigc::bind<wavetable_morph_control_row*>(sigc::ptr_fun ( &on_wavetable_morph_order_changed ), new_morph_row) );
    on_wavetable_morph_order_changed(new_morph_row);

    hbox = new Gtk::Box(Gtk::Orientation::ORIENTATION_HORIZONTAL);

    hbox->add (*new_morph_row->include_btn);
    hbox->add (*order_label);
    hbox->add (*new_morph_row->morph_order);

    new_morph_row->thisrow = new Gtk::ListBoxRow();
    new_morph_row->thisrow->add (*hbox);

    ctrls.morph_wavetable_selection_listbox->insert (*new_morph_row->thisrow, last_morph_row->index + 1);

    new_morph_row->include_btn->set_hexpand (true);
    new_morph_row->include_btn->set_halign (Gtk::ALIGN_FILL);

    new_morph_row->thisrow->set_visible (true);
    new_morph_row->morph_order->set_visible (true);
    new_morph_row->include_btn->set_visible (true);
    order_label->set_visible(true);
    hbox->set_visible (true);

    last_morph_row->next = new_morph_row;
    last_morph_row = new_morph_row;

    active_table = new_row;
    update_wavetable_controls ();
}

void free_wavetable_row(wavetable_control_row* row)
{
    delete row->select_btn;
    delete row->save_btn;
    delete row->rename_btn;
    delete row->delete_btn;
    delete row->thisrow->get_child ();
    delete row->thisrow;

    free(row->harmonics->cos_table);
    free(row->harmonics->sin_table);
    free(row->harmonics);
    if(row->mipmap)free_wavetable_mipmap (row->mipmap);

    typedef std::string str;
    ( &row->last_expr )->~str ();

    delete (reinterpret_cast<Gtk::Box*>(row->morph_row->thisrow->get_child ()))->get_children ().at (1);
    delete row->morph_row->include_btn;
    delete row->morph_row->morph_order;
    delete row->morph_row->thisrow->get_child ();
    delete row->morph_row->thisrow;

    free(row->morph_row);

    free(row);
}

void on_delete_wavetable(wavetable_control_row* row)
{
    if(active_table == row)
    {
        active_table = &first_row;
        update_wavetable_controls ();
    }

    wavetable_control_row *tmp_row = row;
    wavetable_morph_control_row *morph_row = row->morph_row;

    while(tmp_row != last_row)
    {
        tmp_row = tmp_row->next;
        tmp_row->index--;
    }

    tmp_row = &first_row;

    while(tmp_row->next != row )
    {
        tmp_row = tmp_row->next;
    }

    if(tmp_row->next == last_row)
    {
        last_row = tmp_row;
        tmp_row->next = nullptr;
    }
    else
    {
        tmp_row->next = row->next;
    }

    ctrls.wavetable_menu->remove (*row->thisrow);

    delete row->select_btn;
    delete row->save_btn;
    delete row->rename_btn;
    delete row->delete_btn;
    delete row->thisrow->get_child ();
    delete row->thisrow;

    free(row->harmonics->cos_table);
    free(row->harmonics->sin_table);
    free(row->harmonics);
    if(row->mipmap)free_wavetable_mipmap (row->mipmap);

    typedef std::string str;
    ( &row->last_expr )->~str ();

    free(row);


    wavetable_morph_control_row *tmp_morph_row;

    tmp_morph_row = morph_row;

    while(tmp_morph_row != last_morph_row)
    {
        tmp_morph_row = tmp_morph_row->next;
        tmp_morph_row->index--;
    }

    tmp_morph_row = &first_morph_row;

    while(tmp_morph_row->next != morph_row )
    {
        tmp_morph_row = tmp_morph_row->next;
    }

    if(tmp_morph_row->next == last_morph_row)
    {
        last_morph_row = tmp_morph_row;
        tmp_morph_row->next = nullptr;
    }
    else
    {
        tmp_morph_row->next = morph_row->next;
    }

    ctrls.morph_wavetable_selection_listbox->remove (*morph_row->thisrow);

    if(morph_row->include_btn->get_active ())
    {
        free_morph_wavetable_mipmap (ctrls.mwm);
        free(ctrls.morph_harmonics.spectrums);
        ctrls.mwm = nullptr;
        generate_morph_surface_vertices (nullptr);
    }

    delete (reinterpret_cast<Gtk::Box*>(morph_row->thisrow->get_child ()))->get_children ().at (1);
    delete morph_row->include_btn;
    delete morph_row->morph_order;
    delete morph_row->thisrow->get_child ();
    delete morph_row->thisrow;

    free(morph_row);
}

void on_renamed_wavetable (wavetable_control_row* row)
{
    std::string new_name = reinterpret_cast<Gtk::Entry*>(reinterpret_cast<Gtk::Box*>(row->dialogue_ptr->get_child ())->get_children ().at (0))->get_text ();
    std::stringstream new_morph_label;

    new_morph_label << "Include " << new_name;

    row->select_btn->set_label (new_name);
    row->morph_row->include_btn->set_label (new_morph_label.str ());

    row->dialogue_ptr->close ();

    delete reinterpret_cast<Gtk::Entry*>(reinterpret_cast<Gtk::Box*>(row->dialogue_ptr->get_child ())->get_children ().at (0));
    delete row->dialogue_ptr;
}

void on_rename_wavetable (wavetable_control_row* row)
{
    auto dialogue = new Gtk::Dialog("Enter name", true);
    auto name_entry = new Gtk::Entry();

    name_entry->set_text (row->select_btn->get_label () );

    reinterpret_cast<Gtk::Box*>(dialogue->get_child ())->add (*name_entry);

    name_entry->signal_activate ().connect ( sigc::bind<wavetable_control_row*>(sigc::ptr_fun (&on_renamed_wavetable), row) );

    row->dialogue_ptr = dialogue;

    name_entry->set_visible (true);
    dialogue->set_visible (true);
    dialogue->show();
}


void on_harmonics_change()
{
    int newharm;
    double* newcos;
    double* newsin;

    newharm = (int)strtol(ctrls.wavetable_harmonics_entry->get_text ().c_str (), nullptr, 10);

    if(newharm <= 0) return;

    if(newharm > active_table->harmonics->num_harmonics)
    {
        newcos = (double*)calloc(newharm, sizeof(double));
        newsin = (double*)calloc(newharm, sizeof(double));
        memcpy (newcos, active_table->harmonics->cos_table, active_table->harmonics->num_harmonics * sizeof(double));
        memcpy (newsin, active_table->harmonics->sin_table, active_table->harmonics->num_harmonics * sizeof(double));
        free(active_table->harmonics->cos_table);
        free(active_table->harmonics->sin_table);
        active_table->harmonics->cos_table = newcos;
        active_table->harmonics->sin_table = newsin;
    }

    active_table->harmonics->num_harmonics = (int)strtol(ctrls.wavetable_harmonics_entry->get_text ().c_str (), nullptr, 10);

    ctrls.harmonics_control->queue_draw ();
    ctrls.oscilloscope_control->queue_draw ();
}

void update_wavetable_controls()
{
    ctrls.harmonics_control->queue_draw ();
    ctrls.oscilloscope_control->queue_draw ();
}

void on_load_wavetable_selected(const int &thing, Gtk::FileChooserDialog *dialog)
{
    int i;

    if(thing == Gtk::ResponseType::RESPONSE_ACCEPT)
    {
        dialog->hide ();

        //TODO: Check magic number to make sure it's a valid wavetable file

        on_add_wavetable();

        wavetable_mipmap* wtm = load_wavetable ((char*)(dialog->get_file ()->get_path ().c_str ()), 44100);

        fftw_plan plan;

        double* samples = wtm->tables[0].table;
        int num_samples = wtm->tables[0].num_samples;
        fftw_complex* spectrum;

        spectrum = fftw_alloc_complex (num_samples + 1);

        plan = fftw_plan_dft_r2c (1, &num_samples, samples, spectrum, FFTW_ESTIMATE);
        fftw_execute (plan);

        free(last_row->harmonics->cos_table);
        free(last_row->harmonics->sin_table);

        last_row->harmonics->num_harmonics = num_samples;

        last_row->harmonics->cos_table = (double*)malloc(num_samples * sizeof(double));
        last_row->harmonics->sin_table = (double*)malloc(num_samples * sizeof(double));

        double spectrum_max = 0;

        for(i = 0; i < num_samples; i++)
        {
            last_row->harmonics->cos_table[i] = spectrum[i + 1][0];
            last_row->harmonics->sin_table[i] = spectrum[i + 1][1];

            spectrum_max = MAX(spectrum_max, spectrum[i + 1][0]);
            spectrum_max = MAX(spectrum_max, spectrum[i + 1][1]);
        }

        for(i = 0; i < num_samples; i++)
        {
            last_row->harmonics->cos_table[i] /= spectrum_max;
            last_row->harmonics->sin_table[i] /= spectrum_max;
        }

        fftw_free (spectrum);
        fftw_destroy_plan (plan);
        free_wavetable_mipmap (wtm);

        active_table = last_row;

        std::string filename = dialog->get_file ()->get_path ().substr (dialog->get_file ()->get_parent ()->get_path ().length () + 1, -1);
        int extpos = filename.find_last_of ('.');
        filename = filename.substr (0, extpos);

        active_table->select_btn->set_label (filename);

        std::stringstream morph_label;
        morph_label << "Include " << filename;

        active_table->morph_row->include_btn->set_label (morph_label.str ());

        delete dialog;

        update_wavetable_controls ();
    }
}

void on_load_wavetable()
{
    auto fcd = new Gtk::FileChooserDialog("Choose file");

    fcd->signal_response ().connect ( sigc::bind<Gtk::FileChooserDialog*> (sigc::ptr_fun (&on_load_wavetable_selected), fcd) );

    fcd->set_modal (true);
    Glib::RefPtr<Gtk::FileFilter> ff = Gtk::FileFilter::create();
    ff->set_name ("Wavetable mipmap");
    ff->add_pattern ("*.mwt"); // TODO maybe add support for loading wavs here
    fcd->set_filter (ff);
    fcd->add_button ("Open", Gtk::ResponseType::RESPONSE_ACCEPT);
    fcd->show ();
}

void on_select_wavetable(wavetable_control_row* row)
{
    active_table = row;
    std::stringstream num, oversample, tpo;
    num << row->harmonics->num_harmonics;
    oversample << row->oversample;
    tpo << row->tables_per_octave;
    ctrls.wavetable_harmonics_entry->set_text (num.str ());
    ctrls.wavetable_oversample_entry->set_text (oversample.str ());
    ctrls.wavetable_tables_per_octave_entry->set_text (tpo.str ());
    update_wavetable_controls();
}

void on_save_path_selected(const int &response_type, Gtk::FileChooserDialog *dialog)
{
    if(response_type == Gtk::ResponseType::RESPONSE_ACCEPT)
    {
        dialog->hide ();

        std::string wtpath = dialog->get_file ()->get_path ();

        active_table->mipmap = generate_wavetables (active_table->harmonics, active_table->oversample, active_table->tables_per_octave);
        save_wavetable (active_table->mipmap, (char*)wtpath.c_str ());

        delete dialog;
    }
}

void on_save_wavetable(wavetable_control_row* row)
{
    auto *fcd = new Gtk::FileChooserDialog("Choose file",Gtk::FileChooserAction::FILE_CHOOSER_ACTION_SAVE);

    fcd->signal_response ().connect ( sigc::bind<Gtk::FileChooserDialog*> (sigc::ptr_fun (&on_save_path_selected), fcd) );

    fcd->set_modal (true);
    Glib::RefPtr<Gtk::FileFilter> ff = Gtk::FileFilter::create();
    ff->set_name ("Wavetable mipmap");
    ff->add_pattern ("*.mwt");
    fcd->set_filter (ff);
    fcd->add_button ("Save", Gtk::ResponseType::RESPONSE_ACCEPT);
    fcd->set_current_name (row->select_btn->get_label ().append (".mwt"));
    fcd->show ();
}

void on_morph_save_path_selected(const int &response_type, Gtk::FileChooserDialog *fcd)
{
    if(response_type == Gtk::ResponseType::RESPONSE_ACCEPT)
    {
        fcd->hide ();

        std::string wtpath = fcd->get_file ()->get_path ();

        save_morph_wavetable_harmonics (&ctrls.morph_harmonics, wtpath.c_str ());

        ctrls.morph_save_path = wtpath;

        delete fcd;
    }
}

void on_save_morph_wavetable()
{
    if(ctrls.morph_save_path.length () > 0)
    {
        save_morph_wavetable_harmonics (&ctrls.morph_harmonics, ctrls.morph_save_path.c_str ());
    }
    else
    {
        on_save_morph_wavetable_as ();
    }
}

void on_save_morph_wavetable_as()
{
    if(ctrls.mwm)
    {
        auto *fcd = new Gtk::FileChooserDialog("Choose file",Gtk::FileChooserAction::FILE_CHOOSER_ACTION_SAVE);

        fcd->signal_response ().connect ( sigc::bind<Gtk::FileChooserDialog*> (sigc::ptr_fun (&on_morph_save_path_selected), fcd) );

        fcd->set_modal (true);
        Glib::RefPtr<Gtk::FileFilter> ff = Gtk::FileFilter::create();
        ff->set_name ("Morph wavetable harmonics");
        ff->add_pattern ("*.mwh");
        fcd->set_filter (ff);
        fcd->add_button ("Save", Gtk::ResponseType::RESPONSE_ACCEPT);

        fcd->set_current_name ("wavetable.mwh");

        fcd->show ();
    }
}

void save_project (std::string path)
{
    wavetable_control_row *table = &first_row;

    FILE* out = fopen(path.c_str (), "w");

    while(table)
    {
        //TODO: this
        table = table->next;
    }

    fclose(out);
}

void load_project (std::string path)
{
    FILE* in = fopen(path.c_str (), "r");
    //TODO: And this
    fclose(in);
}

void on_project_save_path_selected (const int &response_type, Gtk::FileChooserDialog *fcd)
{
    if(response_type == Gtk::ResponseType::RESPONSE_ACCEPT)
    {
        std::string projectpath = fcd->get_file ()->get_path ();

        ctrls.project_save_path = projectpath;

        save_project (ctrls.project_save_path);
    }
}

void on_project_save()
{
    if(ctrls.project_save_path.size () > 0)
    {
        save_project (ctrls.project_save_path);
    }
    else
    {
        on_project_save_as ();
    }
}

void on_project_save_as()
{
    auto *fcd = new Gtk::FileChooserDialog("Choose file", Gtk::FileChooserAction::FILE_CHOOSER_ACTION_SAVE);

    fcd->signal_response ().connect ( sigc::bind<Gtk::FileChooserDialog*> (sigc::ptr_fun (&on_project_save_path_selected), fcd) );

    fcd->set_modal (true);
    Glib::RefPtr<Gtk::FileFilter> ff = Gtk::FileFilter::create();
    ff->set_name ("Wavetable project");
    ff->add_pattern ("*.wtp");
    fcd->set_filter (ff);
    fcd->add_button ("Save", Gtk::ResponseType::RESPONSE_ACCEPT);

    fcd->show ();
}

void on_project_load(const int &response_type, Gtk::FileChooserDialog *fcd)
{
    morph_wavetable_harmonics *mwh;
    int i;

    if(response_type == Gtk::ResponseType::RESPONSE_ACCEPT)
    {
        wavetable_control_row *current;

        while(first_row.next)
        {
            on_delete_wavetable (first_row.next);
        }

        std::string projectpath = fcd->get_file ()->get_path ();

        if(projectpath.substr (projectpath.size() - 4, projectpath.size()).compare (".wtp") == 0)
        {
            ctrls.project_save_path = projectpath;
            load_project(ctrls.project_save_path);
        }
        else if(projectpath.substr (projectpath.size() - 4, projectpath.size()).compare (".mwh") == 0)
        {
            mwh = load_morph_wavetable_harmonics (projectpath.c_str ());

            ctrls.morph_envelope = mwh->morph_env;

            memcpy(&ctrls.morph_harmonics, mwh, sizeof(morph_wavetable_harmonics));

            free(first_row.harmonics->cos_table);
            free(first_row.harmonics->sin_table);
            free(first_row.harmonics);

            first_row.harmonics = &mwh->spectrums[0];
            first_row.morph_row->include_btn->set_active (true);

            for(i = 1; i < mwh->num_wavetables; i++)
            {
                on_add_wavetable ();
                free(active_table->harmonics->cos_table);
                free(active_table->harmonics->sin_table);
                free(active_table->harmonics);
                active_table->harmonics = &mwh->spectrums[i];
                active_table->morph_row->include_btn->set_active (true);
            }

            ctrls.morph_combobox->set_active (mwh->morph_style);

            on_generate_morph_table ();
        }

        update_wavetable_controls ();

        fcd->hide();
        delete fcd;
    }
}

void on_project_open()
{
    auto *fcd = new Gtk::FileChooserDialog("Choose file",Gtk::FileChooserAction::FILE_CHOOSER_ACTION_OPEN);

    fcd->signal_response ().connect ( sigc::bind<Gtk::FileChooserDialog*> (sigc::ptr_fun (&on_project_load), fcd) );

    fcd->set_modal (true);
    Glib::RefPtr<Gtk::FileFilter> ff = Gtk::FileFilter::create();
    ff->set_name ("Wavetable project");
    ff->add_pattern ("*.wtp");
    ff->add_pattern ("*.mwh");
    fcd->set_filter (ff);
    fcd->add_button ("Open", Gtk::ResponseType::RESPONSE_ACCEPT);

    fcd->show ();
}


void on_set_oversample()
{
    int newoversample = (int)strtol(ctrls.wavetable_oversample_entry->get_text ().c_str (), nullptr, 10);

    if(newoversample <= 0)
    {
        ctrls.wavetable_oversample_entry->set_text ("");
        newoversample = 1;
    }

    active_table->oversample = newoversample;
}

void on_set_tables_per_octave()
{
    int newtpo = (int)strtol(ctrls.wavetable_tables_per_octave_entry->get_text ().c_str (), nullptr, 10);

    if(newtpo <= 0)
    {
        ctrls.wavetable_tables_per_octave_entry->set_text ("");
        newtpo = 1;
    }

    active_table->tables_per_octave = newtpo;
}

void on_morph_type_selected()
{
    if(ctrls.morph_combobox->get_active_row_number () == 5)
    {
        if(!ctrls.morph_envelope)
        {
            ctrls.morph_envelope = (envelope*)calloc(1, sizeof(envelope));
            ctrls.morph_envelope->maxTime = 1;
            ctrls.morph_envelope->minTime = 0;
            ctrls.morph_envelope->minVal = 0;
            ctrls.morph_envelope->maxVal = 1;
            ctrls.morph_envelope->type = SIMPLE;
            ctrls.morph_envelope->first = (breakpoint*)calloc ( 1, sizeof ( breakpoint ) );
            ctrls.morph_envelope->first->next = (breakpoint*)calloc ( 1, sizeof ( breakpoint ) );
            ctrls.morph_envelope->first->value = 0;
            ctrls.morph_envelope->first->time = 0;
            ctrls.morph_envelope->first->interpType = LINEAR;
            ctrls.morph_envelope->first->interpCallback = linear_interp;
            ctrls.morph_envelope->first->next->value = 1;
            ctrls.morph_envelope->first->next->time = 1;
            ctrls.morph_envelope->first->next->interpType = LINEAR;
            ctrls.morph_envelope->first->next->interpCallback = linear_interp;
            ctrls.morph_envelope->current = ctrls.morph_envelope->first;
        }

        auto dialog = new Gtk::Dialog();
        auto envelope_editor = new EnvelopeEditor(ctrls.morph_envelope);
        reinterpret_cast<Gtk::Box*>(dialog->get_child ())->add ( *envelope_editor );

        envelope_editor->set_size_request (400, 200);

        dialog->set_visible (true);
        envelope_editor->set_visible (true);

        dialog->show();
    }
}

void on_wavetable_morph_order_changed(wavetable_morph_control_row *row)
{
    wavetable_morph_control_row *current_row;

    for(current_row = &first_morph_row; current_row != nullptr; current_row = current_row->next)
    {
        std::string current_text = row->morph_order->get_text ();
        int current_text_num = (int)strtol ( current_text.c_str (), nullptr, 10 );
        std::stringstream next;
        next << current_text_num + 1;

        if((current_row != row && strcmp(current_row->morph_order->get_text ().c_str (), current_text.c_str ()) == 0) || current_text_num < 0)
        {
            row->morph_order->set_text (next.str ());
            current_row = &first_morph_row;
        }
    }
}

void init_gl()
{
    ctrls.wavetable_morph_preview->set_required_version (3, 3);
}

void on_quit()
{
    wavetable_control_row *rowptr = &first_row;
    wavetable_control_row *nextrow = rowptr->next;

    while( (rowptr = nextrow) )
    {
        nextrow = rowptr->next;
        free_wavetable_row (rowptr);
    }

    fftw_destroy_plan (oscilloscope_plan);
    delete[] harmonic_btns;
    //TODO: move control specific resource freeing into their own source files

    oscilloscope_free_resources ();

    js_freestate (ctrls.js);

    glDeleteVertexArrays (1, &morph_ctx.morph_surface_VAO_id);
    glDeleteBuffers (1, &morph_ctx.morph_surface_VBO_id);
    //TODO: get handle for shaders and delete them too
    glDeleteProgram (morph_ctx.shader_program);

    ctrls.app->quit ();
}
/*  $Id$
 *
 *  Copyright (C) 2010 Erik Edelmann <erik.edelmann@iki.fi>,
 *                     Adrian Dimitrov <enzo_01@abv.bg>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 *  USA.
 */

#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#include <string.h>
#include <locale.h>
#include <gtk/gtk.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4panel/libxfce4panel.h>
#include "parsetree.h"
#include "parser.h"
#include "eval.h"


// Default settings
#define DEFAULT_DEGREES FALSE
#define DEFAULT_SIZE 20
#define DEFAULT_HIST_SIZE 25
#define DEFAULT_OUTPUT_BASE 10


typedef struct {
    XfcePanelPlugin *plugin;

    GtkWidget *ebox;
    GtkWidget *box;
    GtkWidget *combo;
    GtkWidget *degrees_button;
    GtkWidget *radians_button;
    GtkWidget *hexadecimal_button;

    GList *expr_hist;   // Expression history
    
    // Settings
    gboolean degrees; // Degrees or radians for trigonometric functions?
    gint size;		  // Size of comboboxentry 
    gint hist_size;
    gint output_base; // 10 = decimal, 16 = hexadecimal. Other values are not supported
} CalcPlugin;



void calc_save_config(XfcePanelPlugin *plugin, CalcPlugin *calc)
{
    XfceRc *rc;
    gchar *file;

    file = xfce_panel_plugin_save_location(plugin, TRUE);
    if (file == NULL) return;

    rc = xfce_rc_simple_open(file, FALSE);
    g_free(file);

    if (rc != NULL) {
        xfce_rc_write_bool_entry(rc, "degrees", calc->degrees);
        xfce_rc_write_int_entry(rc, "size", calc->size);
        xfce_rc_write_int_entry(rc, "hist_size", calc->hist_size);
        xfce_rc_write_int_entry(rc, "output_base", calc->output_base);
        xfce_rc_close(rc);
    }
}


static void calc_read_config(CalcPlugin *calc)
{
    XfceRc *rc;
    gchar *file;

    file = xfce_panel_plugin_lookup_rc_file(calc->plugin);

    if (file) {
        rc = xfce_rc_simple_open(file, TRUE);
        g_free(file);
    } else
        rc = NULL;

    if (rc) {
        calc->degrees = xfce_rc_read_bool_entry(rc, "degrees", DEFAULT_DEGREES);
        calc->size = xfce_rc_read_int_entry(rc, "size", DEFAULT_SIZE);
        calc->hist_size = xfce_rc_read_int_entry(rc, "hist_size", DEFAULT_HIST_SIZE);
        calc->output_base = xfce_rc_read_int_entry(rc, "output_base", DEFAULT_OUTPUT_BASE);
        xfce_rc_close(rc);
    } else {
        /* Something went wrong, apply default values. */
        calc->degrees = DEFAULT_DEGREES;
        calc->size = DEFAULT_SIZE;
        calc->hist_size = DEFAULT_HIST_SIZE;
        calc->output_base = DEFAULT_OUTPUT_BASE;
    }
}


static GList *add_to_expr_hist(GList *ehist, gint hist_size, const gchar *str)
{
    GList *elem;

    // Remove duplicates
    if ((elem = g_list_find_custom(ehist, str, (GCompareFunc)g_strcmp0))) {
        g_free(elem->data);
        ehist = g_list_delete_link(ehist, elem);
    }

    // Add the new expression
    ehist = g_list_append(ehist, g_strdup(str));

    // Remove oldest, if list is growing too long.
    if (g_list_length(ehist) > hist_size) {
        elem = g_list_first(ehist);
        g_free(elem->data);
        ehist = g_list_delete_link(ehist, elem);
    }

    return ehist;
}


/* Called when user presses enter in the entry. */

static void entry_enter_cb(GtkEntry *entry, CalcPlugin *calc)
{
    node_t *parsetree;
    const gchar *input;
    GError *err = NULL;
    GList *item;

    input = gtk_entry_get_text(entry);
    parsetree = build_parse_tree(input, &err);
    if (err) {
        xfce_dialog_show_error (NULL, NULL, _("Calculator error: %s"), err->message);
        g_error_free(err);
        free_parsetree(parsetree);
        return;
    }

    calc->expr_hist = add_to_expr_hist(calc->expr_hist, calc->hist_size, input);
    gtk_combo_box_text_remove_all (GTK_COMBO_BOX_TEXT(calc->combo));
    for (item = calc->expr_hist; item != NULL; item = item->next)
        gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(calc->combo), item->data);

    if (parsetree) {
        gdouble r;
        gchar *output;

        r = eval_parse_tree(parsetree, calc->degrees);

        if(calc->output_base == 16) {
#ifdef G_GINT64_MODIFIER
            gint64 i = r;
            output = g_strdup_printf("0x%" G_GINT64_MODIFIER "x", i);
            // note that gdouble precision is less than gint64
            // numbers higher then 2^53 are not exact! (same applies to negative numbers)
#else
            // Some platforms do not support printing 64-bit integers, even
            // though the types are supported. On such platforms
            // G_GINT64_MODIFIER is not defined.
            gint32 i = r;
            output = g_strdup_printf("0x%" G_GINT32_MODIFIER "x", i);
#endif
        }
        else {
            output = g_strdup_printf("%.16g", r);
        }
        gtk_entry_set_text(entry, output);
        gtk_editable_set_position(GTK_EDITABLE(entry), -1);
        g_free(output);
        free_parsetree(parsetree);
    }
    
}


static gboolean entry_buttonpress_cb(GtkWidget *entry, GdkEventButton *event,
                                     CalcPlugin *calc)
{
    GtkWidget *toplevel;

    toplevel = gtk_widget_get_toplevel(entry);

    if (event->button != 3 && toplevel && gtk_widget_get_window(toplevel))
        xfce_panel_plugin_focus_widget(calc->plugin, entry);

    return FALSE;
}


static CalcPlugin *calc_new(XfcePanelPlugin *plugin)
{
    CalcPlugin *calc;
    GtkOrientation orientation;
    GtkWidget *icon, *combo, *entry;

    calc = g_slice_new0(CalcPlugin);
    calc->plugin = plugin;
    calc_read_config(calc);

    orientation = xfce_panel_plugin_get_orientation(plugin);

    calc->ebox = gtk_event_box_new();
    gtk_widget_show(calc->ebox);

    calc->box = gtk_box_new(orientation, 2);
    gtk_widget_show(calc->box);
    gtk_container_add(GTK_CONTAINER(calc->ebox), calc->box);

    icon = gtk_label_new(_(" Calc:"));
    gtk_widget_show(icon);
    gtk_box_pack_start(GTK_BOX(calc->box), icon, FALSE, FALSE, 0);

    combo = gtk_combo_box_text_new_with_entry();
    entry = gtk_bin_get_child(GTK_BIN(combo));
    g_signal_connect(G_OBJECT(entry), "activate",
                     G_CALLBACK(entry_enter_cb), (gpointer) calc);
    g_signal_connect(G_OBJECT(entry), "button-press-event",
                     G_CALLBACK(entry_buttonpress_cb), (gpointer) calc);
    gtk_widget_set_valign(combo, GTK_ALIGN_CENTER);
    gtk_widget_show(combo);
    gtk_box_pack_start(GTK_BOX(calc->box), combo, FALSE, FALSE, 0);
    calc->combo = combo;

    calc->expr_hist = NULL;

    gtk_entry_set_max_length(GTK_ENTRY(entry), 50);
    gtk_entry_set_width_chars(GTK_ENTRY(entry), calc->size);
#if GTK_CHECK_VERSION (3, 22, 20)
    gtk_entry_set_input_hints (GTK_ENTRY(entry), GTK_INPUT_HINT_NO_EMOJI);
#endif

    return calc;
}


/* Used with g_list_foreach() to free data items in a list. */

static void free_stuff(gpointer data, gpointer unused)
{
    g_free(data);
}


static void calc_free(XfcePanelPlugin *plugin, CalcPlugin *calc)
{
    GtkWidget *dialog;

    dialog = g_object_get_data(G_OBJECT(plugin), "dialog");
    if (dialog != NULL)
        gtk_widget_destroy(dialog);

    gtk_widget_destroy(calc->ebox);
    gtk_widget_destroy(calc->box);
    gtk_widget_destroy(calc->combo);

    g_list_foreach(calc->expr_hist, (GFunc)free_stuff, NULL);
    g_list_free(calc->expr_hist);

    /* 
     * FIXME: Do we need to free the strings in the combo list, or is the
     * freeing of expr_hist enough?
     */

    g_slice_free(CalcPlugin, calc);
}


static void calc_orientation_changed(XfcePanelPlugin *plugin,
                                     GtkOrientation orientation,
                                     CalcPlugin *calc)
{
    gtk_orientable_set_orientation(GTK_ORIENTABLE(calc->box), orientation);
}


static gboolean calc_size_changed(XfcePanelPlugin *plugin, gint size,
                                  CalcPlugin *calc)
{
    GtkOrientation orientation;

    orientation = xfce_panel_plugin_get_orientation(plugin);

    if (orientation == GTK_ORIENTATION_HORIZONTAL)
        gtk_widget_set_size_request(GTK_WIDGET(plugin), -1, size);
    else
        gtk_widget_set_size_request(GTK_WIDGET(plugin), size, -1);

    return TRUE;
}


static gboolean calc_plugin_update_size(XfcePanelPlugin *plugin, gint size,
                                        CalcPlugin *calc)
{
	g_assert(calc);
	g_assert(calc->combo);

	calc->size = size;

	GtkWidget *entry = gtk_bin_get_child (GTK_BIN (calc->combo));
	gtk_entry_set_width_chars(GTK_ENTRY(entry), size);

	return TRUE;
}


static void calc_plugin_size_changed(GtkSpinButton *spin, CalcPlugin *calc)
{
	g_assert(calc);
	calc_plugin_update_size(NULL, gtk_spin_button_get_value_as_int(spin), calc);
}


static void calc_hist_size_changed(GtkSpinButton *spin, CalcPlugin *calc)
{
    g_assert(calc);
    calc->hist_size = gtk_spin_button_get_value_as_int(spin);
}


/* Called when the "trigonometrics use degree/radians" menu items change state.

   Note that since they are radio buttons, grouped together, they will allways
   change state both at the same time - one to "active" and the other to "not
   active". Since we actually need only one call per time, we'll ignore the call
   for the de-activated button. */

static void angle_unit_chosen(GtkCheckMenuItem *button, CalcPlugin *calc)
{
    if (!gtk_check_menu_item_get_active(button))
        return;
    
    if (button == (GtkCheckMenuItem *)calc->degrees_button)
        calc->degrees = TRUE;
    else {
        g_assert(button == (GtkCheckMenuItem *)calc->radians_button);
        calc->degrees = FALSE;
    }
}

static void hexadecimal_output_chosen(GtkCheckMenuItem *button, CalcPlugin *calc)
{
    g_assert(button == (GtkCheckMenuItem *)calc->hexadecimal_button);

    if (gtk_check_menu_item_get_active(button))
        calc->output_base = 16;
    else {
        calc->output_base = 10;
    }
    // convert current value to new base
    GtkWidget *entry = gtk_bin_get_child (GTK_BIN (calc->combo));
    entry_enter_cb(GTK_ENTRY(entry), calc);
}


static void calc_dialog_response(GtkWidget *dialog, gint response,
                                 CalcPlugin *calc)
{
    if (response == GTK_RESPONSE_OK || response == GTK_RESPONSE_DELETE_EVENT) {
        g_object_set_data(G_OBJECT(calc->plugin), "dialog", NULL);
        xfce_panel_plugin_unblock_menu(calc->plugin);
        calc_save_config(calc->plugin, calc);
        gtk_widget_destroy(dialog);
    }
}


static void calc_configure(XfcePanelPlugin *plugin, CalcPlugin *calc)
{
    GtkWidget *dialog;
    GtkWidget *toplevel;

    GtkWidget *area;
    GtkWidget *frame;
    GtkWidget *bin;
    GtkWidget *hbox;
    GtkWidget *size_label;
    GtkWidget *size_spin;
    GtkWidget *button;
    GtkWidget *image;
    GtkAdjustment *adjustment;

    xfce_panel_plugin_block_menu(plugin);

    toplevel = gtk_widget_get_toplevel(GTK_WIDGET(plugin)); 
    dialog = xfce_titled_dialog_new_with_buttons(_("Calculator Plugin"),
                       GTK_WINDOW(toplevel),
                       GTK_DIALOG_DESTROY_WITH_PARENT,
                       _("_Close"), GTK_RESPONSE_OK, NULL);

#if LIBXFCE4UI_CHECK_VERSION (4, 15, 1)
  xfce_titled_dialog_create_action_area (XFCE_TITLED_DIALOG (dialog));
#endif

    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
    gtk_window_set_icon_name(GTK_WINDOW(dialog), "xfce4-calculator-plugin");

    /* Link the dialog to the plugin, so we can destroy it when the plugin
     * is closed, but the dialog is still open */
    g_object_set_data(G_OBJECT(plugin), "dialog", dialog);

    g_signal_connect(G_OBJECT(dialog), "response",
                     G_CALLBACK(calc_dialog_response), calc);

	area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

    /* Appearance */
	frame = xfce_gtk_frame_box_new (_("Appearance"), &bin);

	gtk_container_set_border_width(GTK_CONTAINER(frame), 6);
	gtk_box_pack_start(GTK_BOX(area), frame, TRUE, TRUE, 0);
	gtk_widget_show(frame);

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
	gtk_container_add(GTK_CONTAINER (bin), hbox);
	gtk_widget_show(hbox);

	size_label = gtk_label_new(_("Width (in chars):"));
	gtk_box_pack_start(GTK_BOX(hbox), size_label, FALSE, TRUE, 0);
	gtk_widget_show(size_label);
	adjustment = gtk_adjustment_new(calc->size, 5, 100, 1, 5, 10);
	size_spin = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment), 1, 0);
	gtk_widget_add_mnemonic_label(size_spin, size_label);
	gtk_box_pack_end(GTK_BOX(hbox), size_spin, FALSE, TRUE, 0);
	gtk_widget_show(size_spin);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(size_spin), calc->size);
	g_signal_connect(size_spin, "value-changed",
                     G_CALLBACK(calc_plugin_size_changed), calc);

    /* History */
    frame = xfce_gtk_frame_box_new (_("History"), &bin);

    gtk_container_set_border_width(GTK_CONTAINER (frame), 6);
    gtk_box_pack_start(GTK_BOX(area), frame, TRUE, TRUE, 0);
    gtk_widget_show (frame);

    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_container_add(GTK_CONTAINER(bin), hbox);
    gtk_widget_show(hbox);

    size_label = gtk_label_new (_("Size:"));
    gtk_box_pack_start(GTK_BOX(hbox), size_label, FALSE, TRUE, 0);
    gtk_widget_show(size_label);
    adjustment = gtk_adjustment_new(calc->hist_size, 0, 100, 1, 10, 20);
    size_spin = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment), 1, 0);
    gtk_box_pack_end(GTK_BOX(hbox), size_spin, FALSE, TRUE, 0);
    gtk_widget_show (size_spin);
    g_signal_connect(size_spin, "value-changed",
                     G_CALLBACK(calc_hist_size_changed), calc);

    /* add the "Close" button */
    button = gtk_button_new_with_mnemonic (_("_Close"));
    image = gtk_image_new_from_icon_name ("window-close", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image (GTK_BUTTON (button), image);
#if LIBXFCE4UI_CHECK_VERSION (4, 15, 1)
    xfce_titled_dialog_add_action_widget (XFCE_TITLED_DIALOG (dialog), button, GTK_RESPONSE_OK);
#else
    gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_OK);
#endif
    gtk_widget_show (button);

    gtk_widget_show(dialog);
}

void calc_about (XfcePanelPlugin *plugin)
{
       GdkPixbuf *icon;
   const gchar *authors[] = {
      "Erik Edelmann <erik.edelmann@iki.fi>",
      "Adrian Dimitrov <enzo_01@abv.bg>",
      "Roland Kuebert <roland@upic.de>", NULL };
   icon = xfce_panel_pixbuf_from_source("xfce4-calculator-plugin", NULL, 32);
   gtk_show_about_dialog(NULL,
      "logo", icon,
      "license", xfce_get_license_text (XFCE_LICENSE_TEXT_GPL),
      "version", PACKAGE_VERSION,
      "program-name", PACKAGE_NAME,
      "comments", _("Calculator for Xfce panel"),
      "website", "https://docs.xfce.org/panel-plugins/xfce4-calculator-plugin",
      "copyright", _("Copyright (c) 2003-2019\n"),
      "authors", authors, NULL);

   if(icon)
      g_object_unref(G_OBJECT(icon));

}



static void calc_construct(XfcePanelPlugin *plugin)
{
    CalcPlugin *calc;
    GtkWidget *degrees, *radians, *hexadecimal;

    /* Make sure the comma sign (",") isn't treated as a decimal separator. */
    setlocale(LC_NUMERIC, "C");

    xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

    calc = calc_new(plugin);
    gtk_container_add(GTK_CONTAINER(plugin), calc->ebox);

    /* Show the panel's right-click menu on this ebox */
    xfce_panel_plugin_add_action_widget(plugin, calc->ebox);
    
    g_signal_connect(G_OBJECT(plugin), "free-data",
                     G_CALLBACK(calc_free), calc);
    g_signal_connect(G_OBJECT(plugin), "save",
                     G_CALLBACK(calc_save_config), calc);
    g_signal_connect(G_OBJECT(plugin), "size-changed",
                     G_CALLBACK(calc_size_changed), calc);
    g_signal_connect(G_OBJECT(plugin), "orientation-changed",
                     G_CALLBACK(calc_orientation_changed), calc);

    /* Show the configure menu item and connect signal */
    xfce_panel_plugin_menu_show_configure(plugin);
    g_signal_connect(G_OBJECT(plugin), "configure-plugin",
                     G_CALLBACK(calc_configure), calc);

	/* Show the about menu item and connect signal */
	xfce_panel_plugin_menu_show_about (plugin);
	g_signal_connect (G_OBJECT (plugin), "about",
		G_CALLBACK (calc_about), calc);



    // Add controls for choosing angle unit to the menu.
    degrees = gtk_radio_menu_item_new_with_label(
                    NULL,
                    _("Trigonometrics use degrees"));
    radians = gtk_radio_menu_item_new_with_label(
                    gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(degrees)),
                    _("Trigonometrics use radians"));

    if (calc->degrees)
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(degrees), TRUE);
    else
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(radians), TRUE);

    g_signal_connect(G_OBJECT(degrees), "toggled",
                     G_CALLBACK(angle_unit_chosen), calc);

    g_signal_connect(G_OBJECT(radians), "toggled",
                     G_CALLBACK(angle_unit_chosen), calc);

    gtk_widget_show(degrees);
    gtk_widget_show(radians);

    xfce_panel_plugin_menu_insert_item(plugin, GTK_MENU_ITEM(degrees));
    xfce_panel_plugin_menu_insert_item(plugin, GTK_MENU_ITEM(radians));

    // Add checkbox to enable hexadecimal output
    hexadecimal = gtk_check_menu_item_new_with_label(_("Hexadecimal output"));

    if (calc->output_base == 16)
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(hexadecimal), TRUE);

    g_signal_connect(G_OBJECT(hexadecimal), "toggled",
                     G_CALLBACK(hexadecimal_output_chosen), calc);

    gtk_widget_show(hexadecimal);
    xfce_panel_plugin_menu_insert_item(plugin, GTK_MENU_ITEM(hexadecimal));


    calc->degrees_button = degrees;
    calc->radians_button = radians;
    calc->hexadecimal_button = hexadecimal;
}

XFCE_PANEL_PLUGIN_REGISTER(calc_construct);

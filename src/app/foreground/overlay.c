/**
 * Copyright (C) 2021 Ryan Britton
 *
 * This file is part of Goodnight Mouse.
 *
 * Goodnight Mouse is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Goodnight Mouse is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Goodnight Mouse.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "overlay.h"

#include "simplecss.h"

//#define WINDOW_CSS_COLOR "background-color: rgba(%d, %d, %d, %d)"
//#define WINDOW_CSS_FONT "font-family: %s"
//#define WINDOW_CSS_FONT_SIZE "font-size: %dpx"
//#define WINDOW_CSS_FONT_STYLE "font-style: %s"
//#define WINDOW_CSS_FONT_WEIGHT "font-weight: %s"

static void *remove_input(GtkWidget *overlay, gpointer data);

Overlay *overlay_new(OverlayConfig *config)
{
    Overlay *overlay = g_new(Overlay, 1);

    overlay->window = NULL;
    //overlay->tags = NULL;

    //// create overlay
    //overlay->overlay = gtk_window_new(GTK_WINDOW_POPUP);
    //gtk_window_set_title(overlay->overlay, OVERLAY_WINDOW_TITLE);
    //
    //// set css styling
    //GtkStyleContext *style_context = gtk_widget_get_style_context(overlay->overlay);
    //gtk_style_context_add_class(style_context, OVERLAY_CSS_CLASS);
    ////gtk_style_context_add_provider(style_context, config->css, GTK_STYLE_PROVIDER_PRIORITY_SETTINGS);
    //gtk_widget_set_visual(overlay->overlay, gdk_screen_get_rgba_visual(gtk_widget_get_screen(overlay->overlay)));
    //
    //// remove overlay interactivity
    //gtk_window_set_accept_focus(overlay->overlay, FALSE);
    //gtk_widget_set_sensitive(overlay->overlay, FALSE);
    //g_signal_connect(overlay->overlay, "draw", G_CALLBACK(remove_input), NULL);
    //
    //// create container
    //overlay->container = gtk_fixed_new();
    //gtk_container_add(overlay->overlay, overlay->container);

    //SimpleCSS simple_css = simple_css_start(WINDOW_CLASS);
    //simple_css_add(simple_css, WINDOW_CSS_COLOR, config->color_r, config->color_g, config->color_b, config->color_a);
    //simple_css_add(simple_css, WINDOW_CSS_FONT, config->font);
    //simple_css_add(simple_css, WINDOW_CSS_FONT_SIZE, config->font_size);
    //simple_css_add(simple_css, WINDOW_CSS_FONT_STYLE, (config->font_italic) ? "italic" : "normal");
    //simple_css_add(simple_css, WINDOW_CSS_FONT_WEIGHT, (config->font_bold) ? "bold" : "normal");
    //gchar *css = simple_css_finish(simple_css, style_context);

    //self.window = Gtk.Window(type=Gtk.WindowType.POPUP)
    //self.window.get_style_context().add_class("overlay_window")
    //self.window.set_title("goodnight_mouse")
    //self.window.set_visual(self.window.get_screen().get_rgba_visual())
    //self.window.set_accept_focus(False)
    //self.window.set_sensitive(False)
    //# self.window.set_type_hint(Gdk.WindowTypeHint.POPUP_MENU)
    //
    //self.container = Gtk.Fixed()
    //self.container.get_style_context().add_class("action_container")
    //self.window.add(self.container)
    //
    //def remove_input(*args):
    //    self.window.input_shape_combine_region(
    //        cairo.Region(cairo.RectangleInt()))
    //self.window.connect("draw", remove_input)
    //
    //self.x = self.y = self.width = self.height = 0
    //
    //self._css_provider = None
    //self.visible = False

    return overlay;
}

void overlay_destroy(Overlay *overlay)
{
    //if (!overlay->window)
    //    g_object_unref(overlay->window);
    //g_list_free(overlay->tags);
    //
    //gtk_widget_destroy(overlay->window);

    g_free(overlay);
}

//void overlay_add_tag(Overlay *overlay, Tag *tag)
//{
//    overlay->tags = g_list_append(overlay->tags, tag);
//}

void overlay_show(Overlay *overlay, AtspiAccessible *window)
{
    if (overlay->window)
        overlay_hide(overlay);

    overlay->window = g_object_ref(window);
}

void overlay_hide(Overlay *overlay)
{
    if (!overlay->window)
        return;

    g_object_unref(overlay->window);
    overlay->window = NULL;
}

static void *remove_input(GtkWidget *overlay, gpointer data)
{
    cairo_rectangle_int_t rectangle = {
        .x = 0,
        .y = 0,
        .width = 0,
        .height = 0,
    };
    cairo_region_t *region = cairo_region_create_rectangle(&rectangle);
    gdk_window_input_shape_combine_region(overlay, region, 0, 0);
    cairo_region_destroy(region);
}
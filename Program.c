#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

typedef struct _SearchBar
{
    GtkWidget *search_entry;
    GtkWidget *sbutton;
    GtkWidget *nbutton;
    GtkWidget *qbutton;
    GtkWidget *text_view;
} SearchBar;

typedef struct _OpenDialog
{
    GtkWidget *window;
    GtkTextBuffer *buffer;
    gchar *filename;
} OpenDialog;

typedef struct _NewDialog
{
    GtkWidget *window;
    GtkWidget *text_view;
    GtkTextBuffer *buffer;
    GtkWidget *statusbar;
} NewDialog;

void save_dialog_selected(GtkWidget *widget, OpenDialog *sdlog)
{
    GtkWidget *dialog;
    gint response;
    GtkTextBuffer *buffer;

    if (strcmp(gtk_window_get_title(GTK_WINDOW(sdlog -> window)), "Untitled") == 0)
    {
        dialog = gtk_file_chooser_dialog_new("Save File",
                                             GTK_WINDOW(sdlog -> window),
                                             GTK_FILE_CHOOSER_ACTION_SAVE,
                                             GTK_STOCK_SAVE, GTK_RESPONSE_APPLY,
                                             GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                             NULL);

        response = gtk_dialog_run(GTK_DIALOG(dialog));

        if (response == GTK_RESPONSE_APPLY)
        {
            gchar *filename;
            gchar *contents;
            GtkTextIter start, end;
            filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
            buffer = sdlog -> buffer;
            gtk_text_buffer_get_bounds(buffer, &start, &end);
            contents = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
            g_file_set_contents(filename, contents, -1, NULL);
            gtk_window_set_title(GTK_WINDOW(sdlog -> window), filename);
            sdlog -> filename = filename;
        }
        else if (response == GTK_RESPONSE_CANCEL)
        {
            gtk_widget_destroy(dialog);
            return;
        }
        gtk_widget_destroy(dialog);
    }
    else
    {
        GtkTextIter start, end;
        const gchar *filename = gtk_window_get_title(GTK_WINDOW(sdlog -> window)); 
        gchar *contents;
        gtk_text_buffer_get_bounds(sdlog -> buffer, &start, &end);
        contents = gtk_text_buffer_get_text(sdlog -> buffer, &start, &end, FALSE);
        g_file_set_contents(filename, contents, -1, NULL);
    }
}

void new_dialog_selected(GtkWidget *widget, NewDialog *ndlog)
{
    gtk_widget_show(ndlog -> text_view);
    gtk_widget_show(ndlog -> statusbar);
    gtk_window_set_title(GTK_WINDOW(ndlog -> window), "Untitled");
    gtk_text_buffer_set_text(ndlog -> buffer, "", -1);
}

void open_dialog_selected(GtkWidget *widget, OpenDialog *odlog)
{
    GtkWidget *dialog;

    dialog = gtk_file_chooser_dialog_new("Open File",
                                         GTK_WINDOW(odlog -> window),
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                         NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        gchar *filename;
        gchar *contents;
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        g_file_get_contents(filename, &contents, NULL, NULL);
        gtk_text_buffer_set_text(odlog -> buffer, contents, -1);
        gtk_window_set_title(GTK_WINDOW(odlog -> window), filename);
        odlog -> filename = filename;
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

void find(GtkTextView *text_view, const gchar *text, GtkTextIter *iter)
{
    GtkTextIter mstart, mend;
    gboolean found;
    GtkTextBuffer *buffer;
    GtkTextMark *last_pos;

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    found = gtk_text_iter_forward_search(iter, text, 0, &mstart, &mend, NULL);

    if (found)
    {
        gtk_text_buffer_select_range(buffer, &mstart, &mend);
        last_pos = gtk_text_buffer_create_mark(buffer, "last_pos",
                                               &mend, FALSE);
        gtk_text_view_scroll_mark_onscreen(text_view, last_pos);
    }
}

void close_button_clicked(GtkWidget *close_button, SearchBar *bar)
{
    gtk_widget_hide(bar -> search_entry);
    gtk_widget_hide(bar -> sbutton);
    gtk_widget_hide(bar -> nbutton);
    gtk_widget_hide(bar -> qbutton);
}

void find_menu_selected(GtkWidget *widget, SearchBar *bar)
{
    gtk_widget_show(bar -> search_entry);
    gtk_widget_show(bar -> sbutton);
    gtk_widget_show(bar -> nbutton);
    gtk_widget_show(bar -> qbutton);
}

void search_button_clicked(GtkWidget *search_button, SearchBar *bar)
{
    const gchar *text;
    GtkTextBuffer *buffer;
    GtkTextIter iter;

    text = gtk_entry_get_text(GTK_ENTRY(bar -> search_entry));

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(bar -> text_view));
    gtk_text_buffer_get_start_iter(buffer, &iter);
    
    find(GTK_TEXT_VIEW(bar -> text_view), text, &iter);
}

void next_button_clicked(GtkWidget *next_button, SearchBar *bar)
{
    const gchar *text;
    GtkTextBuffer *buffer;
    GtkTextMark *last_pos;
    GtkTextIter iter;

    text = gtk_entry_get_text(GTK_ENTRY(bar -> search_entry));
    
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(bar -> text_view));

    last_pos = gtk_text_buffer_get_mark(buffer, "last_pos");
    if (last_pos == NULL)
    {
        return;
    }

    gtk_text_buffer_get_iter_at_mark(buffer, &iter, last_pos);
    find(GTK_TEXT_VIEW(bar -> text_view), text, &iter);
}

void select_font(GtkWidget *widget, GtkWidget *widget1)
{
    GtkResponseType result;
    GtkWidget *dialog = gtk_font_selection_dialog_new("Select Font");
    result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_OK | result == GTK_RESPONSE_APPLY)
    {
        PangoFontDescription *font_desc;
        gchar *fontname = gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(dialog));

        font_desc = pango_font_description_from_string(fontname);

        gtk_widget_modify_font(GTK_WIDGET(widget1), font_desc);
                               g_free(fontname);
    }
    gtk_widget_destroy(dialog);
}

void show_about(GtkWidget *widget, gpointer data)
{
    GtkWidget *dialog = gtk_about_dialog_new();
    gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(dialog), "TextGTK");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog),
                                  "\nTextGTK is GTK Text Editor.\n");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void copy_to_clipboard(GtkWidget *widget, GtkTextBuffer *buffer)
{
    GtkClipboard *clipboard;
    clipboard = gtk_clipboard_get(GDK_NONE);
    gtk_text_buffer_copy_clipboard(buffer, clipboard);
}

void cut_to_clipboard(GtkWidget *widget, GtkTextBuffer *buffer)
{
    GtkClipboard *clipboard;
    clipboard = gtk_clipboard_get(GDK_NONE);
    gtk_text_buffer_cut_clipboard(buffer, clipboard, TRUE);
}

void paste_from_clipboard(GtkWidget *widget, GtkTextBuffer *buffer)
{
    GtkClipboard *clipboard;
    clipboard = gtk_clipboard_get(GDK_NONE);
    gtk_text_buffer_paste_clipboard(buffer, clipboard, NULL, TRUE);
}

void update_statusbar(GtkTextBuffer *buffer, GtkStatusbar *statusbar)
{
    gchar *msg;
    gint row, col;
    GtkTextIter iter;
    
    gtk_statusbar_pop(statusbar, 0);

    gtk_text_buffer_get_iter_at_mark(buffer,
                                     &iter,
                                     gtk_text_buffer_get_insert(buffer));

    row = gtk_text_iter_get_line(&iter);
    col = gtk_text_iter_get_line_offset(&iter);

    msg = g_strdup_printf("Col: %d, Ln: %d", col + 1, row + 1);

    gtk_statusbar_push(statusbar, 0, msg);

    g_free(msg);
}

void mark_set_callback(GtkTextBuffer *buffer,
    const GtkTextIter *new_location, GtkTextMark *mark, gpointer data)
    {
        update_statusbar(buffer, GTK_STATUSBAR(data));
    }

int main(int argc, char *argv[])
{
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *menusbar;
    GtkWidget *fileMenu;
    GtkWidget *editMenu;
    GtkWidget *optionsMenu;
    GtkWidget *helpMenu;
    GtkWidget *file;
    GtkWidget *new;
    GtkWidget *open;
    GtkWidget *save;
    GtkWidget *quit;
    GtkWidget *edit;
    GtkWidget *cut;
    GtkWidget *copy;
    GtkWidget *paste;
    GtkWidget *options;
    GtkWidget *find;
    GtkWidget *font;
    GtkWidget *help;
    GtkWidget *about;
    GtkWidget *statusbar;
    GtkWidget *text_view;
    GtkTextBuffer *buffer;
    GtkWidget *scrolledwindow;

    SearchBar bar;

    GtkAccelGroup *accel_group = NULL;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);
    gtk_window_set_title(GTK_WINDOW(window), "TextGTK");

    vbox = gtk_vbox_new(FALSE, 2);
    hbox = gtk_hbox_new(FALSE, 2);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    accel_group = gtk_accel_group_new();
    gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

    menusbar = gtk_menu_bar_new();

    fileMenu = gtk_menu_new();
    file = gtk_menu_item_new_with_mnemonic("_File");
    new = gtk_menu_item_new_with_label("New");
    open = gtk_menu_item_new_with_label("Open");
    save = gtk_menu_item_new_with_label("Save");
    quit = gtk_menu_item_new_with_label("Quit");

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), fileMenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), new);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), open);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), save);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), quit);

    gtk_widget_add_accelerator(new,
                               "activate",
                               accel_group,
                               GDK_n,
                               GDK_CONTROL_MASK,
                               GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(open,
                               "activate",
                               accel_group,
                               GDK_o,
                               GDK_CONTROL_MASK,
                               GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(save,
                               "activate",
                               accel_group,
                               GDK_s,
                               GDK_CONTROL_MASK,
                               GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(quit,
                               "activate",
                               accel_group,
                               GDK_q,
                               GDK_CONTROL_MASK,
                               GTK_ACCEL_VISIBLE);

    gtk_menu_shell_append(GTK_MENU_SHELL(menusbar), file);

    editMenu = gtk_menu_new();
    edit = gtk_menu_item_new_with_mnemonic("_Edit");
    cut = gtk_menu_item_new_with_label("Cut");
    copy = gtk_menu_item_new_with_label("Copy");
    paste = gtk_menu_item_new_with_label("Paste");

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit), editMenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(editMenu), cut);
    gtk_menu_shell_append(GTK_MENU_SHELL(editMenu), copy);
    gtk_menu_shell_append(GTK_MENU_SHELL(editMenu), paste);

    gtk_widget_add_accelerator(cut,
                               "activate",
                               accel_group,
                               GDK_x,
                               GDK_CONTROL_MASK,
                               GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(copy,
                               "activate",
                               accel_group,
                               GDK_c,
                               GDK_CONTROL_MASK,
                               GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(paste,
                               "activate",
                               accel_group,
                               GDK_v,
                               GDK_CONTROL_MASK,
                               GTK_ACCEL_VISIBLE);

    gtk_menu_shell_append(GTK_MENU_SHELL(menusbar), edit);

    optionsMenu = gtk_menu_new();
    options = gtk_menu_item_new_with_mnemonic("_Options");
    find = gtk_menu_item_new_with_label("Find");
    font = gtk_menu_item_new_with_label("Font");

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(options), optionsMenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(optionsMenu), find);
    gtk_menu_shell_append(GTK_MENU_SHELL(optionsMenu), font);

    gtk_widget_add_accelerator(find,
                               "activate",
                               accel_group,
                               GDK_f,
                               GDK_CONTROL_MASK,
                               GTK_ACCEL_VISIBLE);

    gtk_menu_shell_append(GTK_MENU_SHELL(menusbar), options);

    helpMenu = gtk_menu_new();
    help = gtk_menu_item_new_with_mnemonic("_Help");
    about = gtk_menu_item_new_with_label("About");

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), helpMenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(helpMenu), about);

    gtk_menu_shell_append(GTK_MENU_SHELL(menusbar), help);

    gtk_box_pack_start(GTK_BOX(vbox), menusbar, FALSE, FALSE, 0);

    scrolledwindow = gtk_scrolled_window_new(NULL, NULL);

    text_view = gtk_text_view_new();
    gtk_container_add(GTK_CONTAINER(scrolledwindow),text_view);
    gtk_box_pack_start(GTK_BOX(vbox), scrolledwindow, 1, 1, 0);

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

    bar.text_view = text_view;
    bar.search_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox), bar.search_entry, TRUE, TRUE, 0);

    bar.sbutton = gtk_button_new_with_label("Search");
    gtk_box_pack_start(GTK_BOX(hbox),bar.sbutton, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(bar.sbutton),
                     "clicked",
                     G_CALLBACK(search_button_clicked),
                     &bar);

    bar.nbutton = gtk_button_new_with_label("Next");
    gtk_box_pack_start(GTK_BOX(hbox), bar.nbutton, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(bar.nbutton),
                     "clicked",
                     G_CALLBACK(next_button_clicked),
                     &bar);

    bar.qbutton = gtk_button_new_with_label("Close");
    gtk_box_pack_start(GTK_BOX(hbox), bar.qbutton, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(bar.qbutton),
                     "clicked",
                     G_CALLBACK(close_button_clicked),
                     &bar);

    OpenDialog odlog;
    odlog.window = window;
    odlog.buffer= buffer;

    NewDialog ndlog;
    ndlog.window = window;
    ndlog.buffer = buffer;
    ndlog.text_view = text_view;

    OpenDialog sdlog;
    sdlog.window = window;
    sdlog.buffer = buffer;

    statusbar = gtk_statusbar_new();
    gtk_box_pack_end(GTK_BOX(vbox), statusbar, FALSE, FALSE, 0);

    ndlog.statusbar = statusbar;

    g_signal_connect(G_OBJECT(window),
                     "destroy",
                     G_CALLBACK(gtk_main_quit),
                     NULL);

    g_signal_connect(G_OBJECT(quit),
                     "activate",
                     G_CALLBACK(gtk_main_quit),
                     NULL);

    g_signal_connect(G_OBJECT(copy),
                     "activate",
                     G_CALLBACK(copy_to_clipboard),
                     buffer);

    g_signal_connect(G_OBJECT(cut),
                     "activate",
                     G_CALLBACK(cut_to_clipboard),
                     buffer);

    g_signal_connect(G_OBJECT(about),
                    "activate",
                    G_CALLBACK(show_about),
                    (gpointer)window);

    g_signal_connect(G_OBJECT(paste),
                     "activate",
                     G_CALLBACK(paste_from_clipboard),
                     buffer);

    g_signal_connect(G_OBJECT(font),
                     "activate",
                     G_CALLBACK(select_font),
                     text_view);

    g_signal_connect(G_OBJECT(find),
                     "activate",
                     G_CALLBACK(find_menu_selected),
                     &bar);

    g_signal_connect(G_OBJECT(open),
                     "activate",
                     G_CALLBACK(open_dialog_selected),
                     &odlog);

    g_signal_connect(G_OBJECT(new),
                     "activate",
                     G_CALLBACK(new_dialog_selected),
                     &ndlog);

    g_signal_connect(G_OBJECT(save),
                     "activate",
                     G_CALLBACK(save_dialog_selected),
                     &sdlog);

    g_signal_connect(buffer,
                     "changed",
                     G_CALLBACK(update_statusbar),
                     statusbar);

    g_signal_connect_object(buffer,
                            "mark_set", 
                            G_CALLBACK(mark_set_callback),
                            statusbar,
                            0);

    gtk_widget_show_all(window);

    gtk_widget_hide(bar.search_entry);
    gtk_widget_hide(bar.sbutton);
    gtk_widget_hide(bar.nbutton);
    gtk_widget_hide(bar.qbutton);

    gtk_widget_hide(text_view);

    gtk_widget_hide(statusbar);

    update_statusbar(buffer, GTK_STATUSBAR(statusbar));

    gtk_window_maximize(GTK_WINDOW(window));

    gtk_main();
}

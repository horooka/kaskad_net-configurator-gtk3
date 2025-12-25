#include "kaskad_net-configurator-gtk3/utils.hpp"
#include <filesystem>
#include <gdk/gdkkeysyms.h>
#include <gdkmm.h>
#include <gtkmm.h>

#define TREE_MIN_WIDTH 300
#define PADDIND_WIDTH 100
#define FORM_MIN_WIDTH 800

class NetworkInteractionConfigurator : public Gtk::Window {
    public:
        NetworkInteractionConfigurator(const std::string &project_path)
            : next_station_id(1),
              config_path(
                  std::filesystem::path(project_path).parent_path().string() +
                  "/Configurator/Stations.ini") {
            button_make_curr_server.set_label("Сделать текущим сервером");
            label_form_id.set_text("Идентификатор");
            label_form_name.set_text("Название");
            label_form_comments.set_text("Комментарии");
            button_make_default.set_label("По умолчанию");
            button_servers_swap.set_label("Поменять");
            button_clients_swap.set_label("Поменять");
            checkbutton_autodetection.set_label("Автоопределение");
            checkbutton_allow_write.set_label(
                "Разрешить управление с этой рабочей станции");
            checkbutton_is_reserved.set_label("Резервирование");
            checkbutton_proxy.set_label("Связь через посредника");
            checkbutton_free_read.set_label(
                "Разрешить чтение значений параметров");
            checkbutton_free_write.set_label(
                "Разрешить запись значений параметров");
            menuitem_save_label.set_text("Сохранить");
            menuitem_exit_label.set_text("Выход");
            menuitem_new_station_label.set_text("Добавить станцию");
            menuitem_delete_label.set_text("Удалить");
            menuitem_help_label.set_text("Справка");
            button_client_address1_dns.set_label("...");
            button_client_address2_dns.set_label("...");
            button_client_address3_dns.set_label("...");
            button_server_address1_dns.set_label("...");
            button_server_address2_dns.set_label("...");
            button_server_address3_dns.set_label("...");

            set_size_request(TREE_MIN_WIDTH + FORM_MIN_WIDTH + PADDIND_WIDTH,
                             600);
            setup_gresources();
            setup_menubuttons();
            setup_top_bar();
            setup_ui();
            setup_signals();
            setup_accel_groups();

            show_all_children();
            setup_data(project_path);
        }

    protected:
        // Чтобы отличать станции от их родителя "Серверы"
        bool is_station() {
            Gtk::TreeModel::iterator curr_station_iter =
                treeview.get_selection()->get_selected();
            if (!curr_station_iter)
                return false;
            return curr_station_iter->parent();
        }

        void button_save_enable() {
            button_save_icon.set(pixbuf_save_enabled);
            menuitem_save_icon.set(pixbuf_save_enabled);
            button_save.set_sensitive(true);
            menuitem_save.set_sensitive(true);
            unsaved = true;
        }

        void button_save_disable() {
            button_save_icon.set(pixbuf_save_disabled);
            menuitem_save_icon.set(pixbuf_save_disabled);
            button_save.set_sensitive(false);
            menuitem_save.set_sensitive(false);
            unsaved = false;
        }

        void on_station_selection_changed() {
            Gtk::TreeModel::Row curr_station_row =
                *treeview.get_selection()->get_selected();
            if (!curr_station_row) {
                vbox_form.hide();
                button_save_disable();
                button_delete.set_sensitive(false);
                return;
            }
            if (!is_station()) {
                vbox_form.hide();
                button_save_disable();
                button_delete.set_sensitive(false);
                return;
            }
            stored_curr_station_row = curr_station_row;

            vbox_form.show_all();
            button_delete.set_sensitive(true);
            if (checkbutton_autodetection.get_active()) {
                button_make_curr_server.set_sensitive(false);
            } else {
                button_make_curr_server.set_sensitive(true);
            }

            entry_id.set_text(
                std::to_string(stored_curr_station_row[station_cols.id]));
            entry_name.set_text(stored_curr_station_row[station_cols.name]);
            entry_comments.set_text(
                stored_curr_station_row[station_cols.comments]);
            checkbutton_allow_write.set_active(
                stored_curr_station_row[station_cols.allow_write]);
            spin_port.set_value(stored_curr_station_row[station_cols.port]);
            spin_timeout.set_value(
                stored_curr_station_row[station_cols.timeout]);
            spin_tries.set_value(stored_curr_station_row[station_cols.tries]);
            spin_timeout0.set_value(
                stored_curr_station_row[station_cols.timeout0]);
            spin_timeout1.set_value(
                stored_curr_station_row[station_cols.timeout1]);
            spin_timeout2.set_value(
                stored_curr_station_row[station_cols.timeout2]);
            spin_timeout3.set_value(
                stored_curr_station_row[station_cols.timeout3]);
            spin_coeff_k.set_value(
                stored_curr_station_row[station_cols.coeff_k]);
            spin_coeff_w.set_value(
                stored_curr_station_row[station_cols.coeff_w]);
            checkbutton_is_reserved.set_active(
                stored_curr_station_row[station_cols.is_reserved]);
            entry_server_address1.set_text(
                stored_curr_station_row[station_cols.server_address1]);
            entry_server_address2.set_text(
                stored_curr_station_row[station_cols.server_address2]);
            entry_server_address3.set_text(
                stored_curr_station_row[station_cols.server_address3]);
            entry_client_address1.set_text(
                stored_curr_station_row[station_cols.client_address1]);
            entry_client_address2.set_text(
                stored_curr_station_row[station_cols.client_address2]);
            entry_client_address3.set_text(
                stored_curr_station_row[station_cols.client_address3]);
            checkbutton_proxy.set_active(
                stored_curr_station_row[station_cols.proxy_address] != "Нет");
            entry_proxy_address.set_text(
                stored_curr_station_row[station_cols.proxy_address]);
            checkbutton_free_read.set_active(
                stored_curr_station_row[station_cols.free_read]);
            checkbutton_free_write.set_active(
                stored_curr_station_row[station_cols.free_write]);
        }

        void on_new_station_clicked() {
            int missing_idx = get_missing_id(servers_tree_row, station_cols);
            int new_id;
            if (missing_idx == -1) {
                main_settings.stations_amount++;
                new_id = main_settings.stations_amount;
            } else {
                auto sortable = Glib::RefPtr<Gtk::TreeSortable>::cast_dynamic(
                    treestore_stations);
                sortable->set_sort_column(station_cols.id,
                                          Gtk::SortType::SORT_ASCENDING);
                new_id = missing_idx;
                main_settings.stations_amount++;
            }

            Gtk::TreeModel::Row new_row =
                *(treestore_stations->append(servers_tree_row.children()));
            if (!treeview.row_expanded(
                    treestore_stations->get_path(servers_tree_row))) {
                // Раскрытие корня станций при добавлении новой
                treeview.expand_row(
                    treestore_stations->get_path(servers_tree_row), false);
            }
            int attempt = 1;
            treeview.get_selection()->select(new_row);
            bool unique = true;
            for (const Gtk::TreeModel::Row &row : servers_tree_row.children()) {
                if (row[station_cols.name] == "Новая станция") {
                    unique = false;
                    break;
                }
            }
            std::string new_name = "Новая станция";
            if (!unique) {
                while (true) {
                    new_name = "Новая станция " + std::to_string(attempt);
                    unique = true;
                    for (const Gtk::TreeModel::Row &row :
                         servers_tree_row.children()) {
                        if (row[station_cols.name] == new_name) {
                            unique = false;
                            break;
                        }
                    }
                    if (unique || attempt == 1000)
                        break;
                    attempt++;
                }
            }

            new_row[station_cols.icon] = pixbuf_station;
            entry_id.set_text(std::to_string(new_id));
            new_row[station_cols.id] = new_id;
            entry_name.set_text(new_name);
            entry_comments.set_text("");
            checkbutton_allow_write.set_active(false);
            spin_port.set_value(25923);
            spin_timeout.set_value(500);
            spin_tries.set_value(3);
            spin_timeout0.set_value(5);
            spin_timeout1.set_value(15);
            spin_timeout2.set_value(10);
            spin_timeout3.set_value(20);
            spin_coeff_k.set_value(12);
            spin_coeff_w.set_value(8);
            checkbutton_is_reserved.set_active(false);
            entry_server_address1.set_text("127.0.0.1");
            entry_server_address2.set_text("");
            entry_server_address3.set_text("");
            entry_client_address1.set_text("127.0.0.1");
            entry_client_address2.set_text("");
            entry_client_address3.set_text("");
            checkbutton_proxy.set_active(false);
            checkbutton_free_read.set_active(true);
            checkbutton_free_write.set_active(false);
        }

        void on_save_clicked() {
            std::string errors;
            int ret_backup = write_backup(config_path, errors);
            if (ret_backup == 0 || ret_backup == 14) {
                (void)write_config(config_path, main_settings, servers_tree_row,
                                   station_cols, errors);
            }
            if (errors.empty()) {
                button_save_disable();
            } else {
                Gtk::MessageDialog dialog(
                    "Не удалось сохранить файл конфигурации\n\n" + errors,
                    false, Gtk::MESSAGE_ERROR);
                dialog.run();
            }
        }

        bool on_exit_clicked() {
            if (unsaved) {
                Gtk::MessageDialog dialog("Сохранить изменения?", false,
                                          Gtk::MESSAGE_QUESTION,
                                          Gtk::BUTTONS_NONE);
                dialog.add_button("Да", Gtk::RESPONSE_YES);
                dialog.add_button("Нет", Gtk::RESPONSE_NO);
                dialog.add_button("Отмена", Gtk::RESPONSE_CANCEL);

                int res = dialog.run();
                if (res == Gtk::RESPONSE_NO)
                    return false;
                if (res == Gtk::RESPONSE_YES) {
                    on_save_clicked();
                    if (unsaved)
                        return true;
                    return false;
                }
                return true;
            }
            return false;
        }

        void on_delete_clicked() {
            Gtk::TreeModel::Row curr_station_row =
                *treeview.get_selection()->get_selected();
            if (!curr_station_row)
                return;

            Gtk::MessageDialog dialog(
                "Вы уверены, что хотите удалить эту станцию?", false,
                Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE);
            dialog.add_button("OK", Gtk::RESPONSE_YES);
            dialog.add_button("Отмена", Gtk::RESPONSE_CANCEL);
            if (dialog.run() != Gtk::RESPONSE_YES)
                return;
            treestore_stations->erase(curr_station_row);

            main_settings.stations_amount--;
            Gtk::TreeNodeChildren stations = servers_tree_row.children();
            if (stations.empty()) {
                button_save_disable();
                button_delete.set_sensitive(false);
                vbox_form.hide();
            }
            int missing_idx = get_missing_id(servers_tree_row, station_cols);
            if (missing_idx == -1) {
                next_station_id--;
            }
            button_save_enable();
        }

    private:
        void setup_ui() {
            set_title("Настройки сетевого взаимодействия");
            auto css = Gtk::CssProvider::create();
            css->load_from_data(R"( 
                * {
                    font-family: Sans;
                    font-size: 14px;
                }
                #read-only {
                    opacity: 0.5;
                }
                #button {
                    border-radius: 0;
                }
                #button-without-border {
                    border-style: none;
                    background-color: transparent;
                }
                #red-label {
                    color: red;
                    font-weight: bold;
                }
                #white_background {
                    background-color: white;
                }
            )");
            auto screen = get_screen();
            Gtk::StyleContext::add_provider_for_screen(
                screen, css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

            auto add_form_row_to_box = [](const Glib::ustring &label_text,
                                          Gtk::Widget &widget, Gtk::Box &box) {
                Gtk::Box *hbox =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
                Gtk::Label *label = Gtk::make_managed<Gtk::Label>(label_text);
                hbox->pack_start(*label, Gtk::PACK_SHRINK);
                hbox->pack_start(widget, Gtk::PACK_EXPAND_WIDGET);
                set_margin(widget, 5, 5);
                box.pack_start(*hbox, Gtk::PACK_SHRINK);
            };
            auto add_form_address_row_to_box =
                [](const Glib::ustring &label_text, Gtk::Widget &widget,
                   Gtk::Button &dns_resolve_button, Gtk::Box &box) {
                    Gtk::Box *hbox = Gtk::make_managed<Gtk::Box>(
                        Gtk::ORIENTATION_HORIZONTAL, 5);
                    Gtk::Label *label =
                        Gtk::make_managed<Gtk::Label>(label_text);
                    hbox->pack_start(*label, Gtk::PACK_SHRINK);
                    hbox->pack_start(widget, Gtk::PACK_EXPAND_WIDGET);
                    hbox->pack_start(dns_resolve_button, Gtk::PACK_SHRINK);
                    set_margin(widget, 5, 5);
                    box.pack_start(*hbox, Gtk::PACK_SHRINK);
                };

            set_titlebar(header_bar);
            header_bar.set_show_close_button(true);
            header_bar.set_decoration_layout("menu:minimize,maximize,close");
            add(vbox_main);
            vbox_main.set_hexpand(true);
            vbox_main.set_vexpand(true);
            scrolled_tree.set_name("white_background");
            scrolled_tree.set_hexpand(true);
            scrolled_tree.set_vexpand(true);
            scrolled_tree.set_min_content_height(600);
            scrolled_tree.set_min_content_width(TREE_MIN_WIDTH);
            treeview.set_hexpand(true);
            treeview.set_vexpand(true);
            scrolled_form.set_hexpand(true);
            scrolled_form.set_vexpand(true);
            scrolled_form.set_min_content_height(600);
            scrolled_form.set_min_content_width(FORM_MIN_WIDTH);
            vbox_form.set_hexpand(true);
            vbox_form.set_vexpand(true);
            frame_timeouts.set_shadow_type(Gtk::SHADOW_NONE);
            frame_coeffs.set_shadow_type(Gtk::SHADOW_NONE);
            button_make_default.set_name("button-without-border");
            button_make_default.set_relief(Gtk::RELIEF_NONE);
            button_servers_swap.set_name("button-without-border");
            button_servers_swap.set_relief(Gtk::RELIEF_NONE);
            button_clients_swap.set_name("button-without-border");
            button_clients_swap.set_relief(Gtk::RELIEF_NONE);
            button_server_address1_dns.set_name("button-without-border");
            button_server_address1_dns.set_relief(Gtk::RELIEF_NONE);
            button_server_address2_dns.set_name("button-without-border");
            button_server_address2_dns.set_relief(Gtk::RELIEF_NONE);
            button_server_address3_dns.set_name("button-without-border");
            button_server_address3_dns.set_relief(Gtk::RELIEF_NONE);
            button_client_address1_dns.set_name("button-without-border");
            button_client_address1_dns.set_relief(Gtk::RELIEF_NONE);
            button_client_address2_dns.set_name("button-without-border");
            button_client_address2_dns.set_relief(Gtk::RELIEF_NONE);
            button_client_address3_dns.set_name("button-without-border");
            button_client_address3_dns.set_relief(Gtk::RELIEF_NONE);
            set_margin(scrolled_tree, 10, 15);
            set_margin(scrolled_form, 10, 10);
            set_margin(treeview, 10, 10);
            set_margin(vbox_general, 10, 10);
            set_margin(hbox_udp_settings, 10, 10);
            set_margin(hbox_iec_settings, 10, 10);
            set_margin(vbox_main_servers, 10, 10);
            set_margin(vbox_main_clients, 10, 10);
            set_margin(vbox_restrictions, 10, 10);

            vbox_main.pack_start(paned_main, Gtk::PACK_EXPAND_WIDGET);
            paned_main.set_hexpand(true);
            paned_main.add1(scrolled_tree);
            paned_main.child_property_resize(scrolled_tree) = true;
            paned_main.child_property_shrink(scrolled_tree) = true;
            scrolled_tree.set_policy(Gtk::POLICY_AUTOMATIC,
                                     Gtk::POLICY_AUTOMATIC);
            treestore_stations = Gtk::TreeStore::create(station_cols);
            servers_tree_row = *(treestore_stations)->append();
            servers_tree_row[station_cols.name] = "Серверы";
            scrolled_tree.add(frame_tree);
            frame_tree.add(treeview);
            treeview.set_model(treestore_stations);
            treeview.set_headers_visible(false);
            renderer_icon.set_alignment(0.0, 0.0);
            treecolumn_icon_idx.pack_start(renderer_icon, false);
            treecolumn_icon_idx.pack_start(renderer_id, false);
            treecolumn_icon_idx.add_attribute(renderer_icon.property_pixbuf(),
                                              station_cols.icon);
            treecolumn_icon_idx.add_attribute(renderer_id.property_text(),
                                              station_cols.id);
            treeview.append_column(treecolumn_icon_idx);
            treecolumn_name.pack_start(renderer_name, true);
            treecolumn_name.add_attribute(renderer_name.property_text(),
                                          station_cols.name);
            treeview.append_column(treecolumn_name);
            treecolumn_comments.pack_start(renderer_comments, true);
            treecolumn_comments.add_attribute(renderer_comments.property_text(),
                                              station_cols.comments);
            treeview.append_column(treecolumn_comments);
            // Настройка отображения идентификаторов станций
            treecolumn_icon_idx.set_cell_data_func(
                renderer_id,
                [this](Gtk::CellRenderer *renderer,
                       const Gtk::TreeModel::const_iterator &iter) {
                    Gtk::CellRendererText *text_renderer =
                        dynamic_cast<Gtk::CellRendererText *>(renderer);
                    if (!text_renderer)
                        return;
                    if (!iter->parent()) {
                        text_renderer->property_visible() = false;
                        return;
                    }
                    text_renderer->property_visible() = true;
                    int value = (*iter)[station_cols.id];
                    std::ostringstream oss;
                    oss << std::setfill('0') << std::setw(3) << value;
                    text_renderer->property_text() = oss.str();
                });

            paned_main.add2(scrolled_form);
            paned_main.child_property_resize(scrolled_form) = true;
            paned_main.child_property_shrink(scrolled_form) = true;
            scrolled_form.add(vbox_form);
            scrolled_form.set_policy(Gtk::POLICY_AUTOMATIC,
                                     Gtk::POLICY_AUTOMATIC);
            vbox_form.pack_start(frame_general, Gtk::PACK_SHRINK);
            frame_general_label.set_markup("<b>Общие</b>");
            frame_general.set_label_widget(frame_general_label);
            frame_general.add(vbox_general);
            vbox_general.pack_start(grid_general, Gtk::PACK_SHRINK);
            grid_general.set_column_spacing(25);
            grid_general.set_row_spacing(5);
            entry_id.set_editable(false);
            entry_id.set_can_focus(false);
            entry_id.set_hexpand(true);
            entry_id.set_name("read-only");
            label_form_id.set_halign(Gtk::ALIGN_START);
            grid_general.attach(label_form_id, 0, 0, 1, 1);
            grid_general.attach(hbox_general_id, 1, 0, 1, 1);
            hbox_general_id.pack_start(entry_id, Gtk::PACK_EXPAND_WIDGET);
            hbox_general_id.pack_start(button_make_curr_server,
                                       Gtk::PACK_SHRINK);
            label_form_name.set_halign(Gtk::ALIGN_START);
            grid_general.attach(label_form_name, 0, 1, 1, 1);
            grid_general.attach(entry_name, 1, 1, 1, 1);
            label_form_comments.set_halign(Gtk::ALIGN_START);
            grid_general.attach(label_form_comments, 0, 2, 1, 1);
            grid_general.attach(entry_comments, 1, 2, 1, 1);
            vbox_general.pack_start(checkbutton_allow_write, Gtk::PACK_SHRINK);

            frame_udp_settings_label.set_markup("<b>Параметры протокола обмена "
                                                "UDP</b>");
            frame_udp_settings.set_label_widget(frame_udp_settings_label);
            spin_timeout.set_adjustment(
                Gtk::Adjustment::create(3, 0, std::numeric_limits<int>::max()));
            spin_timeout.set_numeric(true);
            spin_tries.set_adjustment(
                Gtk::Adjustment::create(3, 0, std::numeric_limits<int>::max()));
            spin_tries.set_numeric(true);
            Gtk::Label *port_label = Gtk::make_managed<Gtk::Label>("Порт");
            hbox_udp_settings.pack_start(*port_label, Gtk::PACK_SHRINK);
            hbox_udp_settings.pack_start(spin_port, Gtk::PACK_EXPAND_WIDGET);
            Gtk::Label *timeout_label =
                Gtk::make_managed<Gtk::Label>("Тайм-аут, мсек");
            hbox_udp_settings.pack_start(*timeout_label, Gtk::PACK_SHRINK);
            hbox_udp_settings.pack_start(spin_timeout, Gtk::PACK_EXPAND_WIDGET);
            Gtk::Label *tries_label = Gtk::make_managed<Gtk::Label>("Попытки");
            hbox_udp_settings.pack_start(*tries_label, Gtk::PACK_SHRINK);
            hbox_udp_settings.pack_start(spin_tries, Gtk::PACK_EXPAND_WIDGET);
            frame_udp_settings.add(hbox_udp_settings);
            vbox_form.pack_start(frame_udp_settings, Gtk::PACK_SHRINK);

            frame_iec_settings_label.set_markup(
                "<b>Параметры протокола IEC</b>");
            frame_iec_settings.set_label_widget(frame_iec_settings_label);
            frame_iec_settings.add(hbox_iec_settings);
            hbox_iec_settings.pack_start(frame_timeouts, Gtk::PACK_SHRINK);
            frame_timeouts_label.set_text("Тайм-ауты, сек");
            frame_timeouts.set_label_widget(frame_timeouts_label);
            frame_timeouts.add(hbox_timeouts);
            vbox_form.pack_start(frame_iec_settings, Gtk::PACK_SHRINK);
            hbox_timeouts.pack_start(vbox_timeouts1, Gtk::PACK_SHRINK);
            add_form_row_to_box("T0", spin_timeout0, vbox_timeouts1);
            add_form_row_to_box("T1", spin_timeout1, vbox_timeouts1);
            hbox_timeouts.pack_start(vbox_timeouts2, Gtk::PACK_SHRINK);
            add_form_row_to_box("T2", spin_timeout2, vbox_timeouts2);
            add_form_row_to_box("T3", spin_timeout3, vbox_timeouts2);
            spin_timeout0.set_adjustment(
                Gtk::Adjustment::create(5, 0, std::numeric_limits<int>::max()));
            spin_timeout0.set_numeric(true);
            spin_timeout1.set_adjustment(Gtk::Adjustment::create(
                15, 0, std::numeric_limits<int>::max()));
            spin_timeout1.set_numeric(true);
            spin_timeout2.set_adjustment(Gtk::Adjustment::create(
                10, 0, std::numeric_limits<int>::max()));
            spin_timeout2.set_numeric(true);
            spin_timeout3.set_adjustment(Gtk::Adjustment::create(
                10, 0, std::numeric_limits<int>::max()));
            spin_timeout3.set_numeric(true);
            spin_port.set_adjustment(Gtk::Adjustment::create(25923, 1, 65535));
            spin_port.set_numeric(true);

            hbox_iec_settings.pack_start(frame_coeffs, Gtk::PACK_SHRINK);
            frame_coeffs_label.set_text("Коэффиценты");
            frame_coeffs.set_label_widget(frame_coeffs_label);
            frame_coeffs.add(hbox_coeffs);
            hbox_coeffs.pack_start(vbox_coeffs, Gtk::PACK_SHRINK);
            add_form_row_to_box("K", spin_coeff_k, vbox_coeffs);
            add_form_row_to_box("W", spin_coeff_w, vbox_coeffs);
            hbox_coeffs.pack_start(button_make_default, Gtk::PACK_SHRINK);
            spin_coeff_k.set_adjustment(Gtk::Adjustment::create(
                12, 1, std::numeric_limits<int>::max()));
            spin_coeff_k.set_numeric(true);
            spin_coeff_w.set_adjustment(
                Gtk::Adjustment::create(8, 1, std::numeric_limits<int>::max()));
            spin_coeff_w.set_numeric(true);

            vbox_form.pack_start(frame_servers, Gtk::PACK_SHRINK);
            frame_servers_label.set_markup(
                "<b>Свойства TCP/IP для сервера</b>");
            frame_servers.set_label_widget(frame_servers_label);
            frame_servers.add(vbox_main_servers);
            vbox_main_servers.pack_start(checkbutton_is_reserved,
                                         Gtk::PACK_SHRINK);
            vbox_main_servers.pack_start(hbox_servers, Gtk::PACK_SHRINK);
            hbox_servers.pack_start(vbox_servers, Gtk::PACK_SHRINK);
            add_form_address_row_to_box("Адрес1", entry_server_address1,
                                        button_server_address1_dns,
                                        vbox_servers);
            add_form_address_row_to_box("Адрес2", entry_server_address2,
                                        button_server_address2_dns,
                                        vbox_servers);
            add_form_address_row_to_box("Адрес3", entry_server_address3,
                                        button_server_address3_dns,
                                        vbox_servers);
            button_servers_swap.set_hexpand(false);
            button_servers_swap.set_halign(Gtk::ALIGN_CENTER);
            button_servers_swap.set_valign(Gtk::ALIGN_CENTER);
            hbox_servers.set_homogeneous(false);
            hbox_servers.pack_start(button_servers_swap, Gtk::PACK_SHRINK);

            vbox_form.pack_start(frame_clients, Gtk::PACK_SHRINK);
            frame_clients_label.set_markup(
                "<b>Свойства TCP/IP для клиента</b>");
            frame_clients.set_label_widget(frame_clients_label);
            frame_clients.add(vbox_main_clients);
            vbox_main_clients.pack_start(hbox_clients, Gtk::PACK_SHRINK);
            hbox_clients.pack_start(vbox_clients, Gtk::PACK_SHRINK);
            add_form_address_row_to_box("Адрес1", entry_client_address1,
                                        button_client_address1_dns,
                                        vbox_clients);
            add_form_address_row_to_box("Адрес2", entry_client_address2,
                                        button_client_address2_dns,
                                        vbox_clients);
            add_form_address_row_to_box("Адрес3", entry_client_address3,
                                        button_client_address3_dns,
                                        vbox_clients);
            button_clients_swap.set_hexpand(false);
            button_clients_swap.set_halign(Gtk::ALIGN_CENTER);
            button_clients_swap.set_valign(Gtk::ALIGN_CENTER);
            hbox_clients.set_homogeneous(false);
            hbox_clients.pack_start(button_clients_swap, Gtk::PACK_SHRINK);
            vbox_main_clients.pack_start(checkbutton_proxy, Gtk::PACK_SHRINK);
            entry_proxy_address.set_hexpand(false);
            entry_proxy_address.set_halign(Gtk::ALIGN_START);
            add_form_row_to_box("Адрес", entry_proxy_address,
                                vbox_main_clients);

            vbox_form.pack_start(frame_restrictions, Gtk::PACK_SHRINK);
            frame_restrictions_label.set_markup("<b>Ограничение доступа</b>");
            frame_restrictions.set_label_widget(frame_restrictions_label);
            frame_restrictions.add(vbox_restrictions);
            label_restrictions_unregistered.set_text(
                "С незарегистрированных рабочих станций");
            label_restrictions_unregistered.set_halign(Gtk::ALIGN_START);
            vbox_restrictions.pack_start(label_restrictions_unregistered);
            vbox_restrictions.pack_start(checkbutton_free_read,
                                         Gtk::PACK_SHRINK);
            vbox_restrictions.pack_start(checkbutton_free_write,
                                         Gtk::PACK_SHRINK);
            entry_proxy_address.set_text("Нет");
            entry_proxy_address.set_sensitive(false);
        }

        void setup_gresources() {
            pixbuf_save_enabled = Gdk::Pixbuf::create_from_resource(
                "/org/icons/save-enabled.png");
            pixbuf_station =
                Gdk::Pixbuf::create_from_resource("/org/icons/server.png");
            pixbuf_new_station =
                Gdk::Pixbuf::create_from_resource("/org/icons/server-add.png");
            pixbuf_delete =
                Gdk::Pixbuf::create_from_resource("/org/icons/delete.png");
            pixbuf_help = Gdk::Pixbuf::create_from_resource(
                "/org/icons/question-mark.png");
            pixbuf_save_disabled = Gdk::Pixbuf::create_from_resource(
                "/org/icons/save-disabled.png");
        }

        void setup_menubuttons() {
            menubutton_file.set_label("Файл");
            menubutton_file.set_name("button-without-border");
            menubutton_file.set_relief(Gtk::RELIEF_NONE);
            header_bar.pack_start(menubutton_file);
            menu_file.append(menuitem_save);
            menuitem_save.add(menuitem_save_box);
            menuitem_save_box.pack_start(menuitem_save_icon, Gtk::PACK_SHRINK);
            menuitem_save_box.pack_start(menuitem_save_label, Gtk::PACK_SHRINK);
            menu_file.append(menuitem_exit);
            menuitem_exit.add(menuitem_exit_box);
            menuitem_exit_box.pack_start(menuitem_exit_label, Gtk::PACK_SHRINK);
            menubutton_file.set_popup(menu_file);
            menu_file.show_all();

            menuitem_new_station_icon = Gtk::Image(pixbuf_new_station);
            menubutton_edit.set_label("Правка");
            menubutton_edit.set_name("button-without-border");
            menubutton_edit.set_relief(Gtk::RELIEF_NONE);
            header_bar.pack_start(menubutton_edit);
            menu_edit.append(menuitem_new_station);
            menuitem_new_station.add(menuitem_new_station_box);
            menuitem_new_station_box.pack_start(menuitem_new_station_icon,
                                                Gtk::PACK_SHRINK);
            menuitem_new_station_box.pack_start(menuitem_new_station_label,
                                                Gtk::PACK_SHRINK);
            menuitem_delete_icon = Gtk::Image(pixbuf_delete);
            menu_edit.append(menuitem_delete);
            menuitem_delete.add(menuitem_delete_box);
            menuitem_delete_box.pack_start(menuitem_delete_icon,
                                           Gtk::PACK_SHRINK);
            menuitem_delete_box.pack_start(menuitem_delete_label,
                                           Gtk::PACK_SHRINK);
            menubutton_edit.set_popup(menu_edit);
            menu_edit.show_all();

            menuitem_help_icon = Gtk::Image(pixbuf_help);
            menubutton_help.set_label("Справка");
            menubutton_help.set_name("button-without-border");
            menubutton_help.set_relief(Gtk::RELIEF_NONE);
            header_bar.pack_start(menubutton_help);
            menu_help.append(menuitem_help);
            menuitem_help.add(menuitem_help_box);
            menuitem_help_box.pack_start(menuitem_help_icon, Gtk::PACK_SHRINK);
            menuitem_help_box.pack_start(menuitem_help_label, Gtk::PACK_SHRINK);
            menubutton_help.set_popup(menu_help);
            menu_help.show_all();
        }

        void setup_top_bar() {
            vbox_main.pack_start(top_bar, Gtk::PACK_SHRINK);

            top_bar.pack_start(button_save, Gtk::PACK_SHRINK);
            button_save.set_tooltip_text("Сохранить (Ctrl+S)");
            button_save.set_image(button_save_icon);
            button_save.set_always_show_image(true);
            button_save_disable();
            top_bar.pack_start(top_bar_separator1, Gtk::PACK_SHRINK);

            button_new_station_icon = Gtk::Image(pixbuf_new_station);
            top_bar.pack_start(button_new_station, Gtk::PACK_SHRINK);
            button_new_station.set_tooltip_text("Добавить станцию");
            button_new_station.set_image(button_new_station_icon);
            button_new_station.set_always_show_image(true);

            button_delete_icon = Gtk::Image(pixbuf_delete);
            top_bar.pack_start(button_delete, Gtk::PACK_SHRINK);
            button_delete.set_tooltip_text("Удалить станцию");
            button_delete.set_image(button_delete_icon);
            button_delete.set_always_show_image(true);
            button_delete.set_sensitive(false);
            top_bar.pack_start(top_bar_separator2, Gtk::PACK_SHRINK);

            combobox_stations_label.set_text("Текущий сервер");
            top_bar.pack_start(combobox_stations_label, Gtk::PACK_SHRINK);
            top_bar.pack_start(stack_autodetection, Gtk::PACK_SHRINK);
            stack_autodetection.set_transition_type(
                Gtk::STACK_TRANSITION_TYPE_NONE);
            stack_autodetection.add(combobox_stations_error_label);
            stack_autodetection.add(combobox_stations);
            combobox_stations_error_label.set_text("Станция не определена");
            combobox_stations_error_label.set_name("red-label");
            top_bar.pack_start(checkbutton_autodetection, Gtk::PACK_SHRINK);
            checkbutton_autodetection.set_tooltip_text(
                "Автоопределение текущей станции");
            set_margin(top_bar, 0, 5);
            set_margin(combobox_stations_label, 5, 0);
            set_margin(combobox_stations_error_label, 5, 0);
            set_margin(combobox_stations, 5, 0);
            button_new_station.set_margin_left(5);
        }

        void setup_signals() {
            signal_delete_event().connect([this](GdkEventAny *event) {
                (void)event;
                bool ret = on_exit_clicked();
                return ret;
            });
            treeview.get_selection()->signal_changed().connect(sigc::mem_fun(
                *this,
                &NetworkInteractionConfigurator::on_station_selection_changed));
            paned_main.property_position().signal_changed().connect([&]() {
                int pos = paned_main.get_position();
                int total_width = paned_main.get_allocation().get_width();

                if (pos < TREE_MIN_WIDTH)
                    paned_main.set_position(TREE_MIN_WIDTH);
                else if (pos > total_width - FORM_MIN_WIDTH)
                    paned_main.set_position(total_width - FORM_MIN_WIDTH);
            });

            menuitem_save.signal_activate().connect(
                [this]() { on_save_clicked(); });
            menuitem_exit.signal_activate().connect(
                [this]() { on_exit_clicked(); });
            menuitem_new_station.signal_activate().connect(
                [this]() { on_new_station_clicked(); });
            menuitem_delete.signal_activate().connect(
                [this]() { on_delete_clicked(); });

            button_new_station.signal_clicked().connect(sigc::mem_fun(
                *this,
                &NetworkInteractionConfigurator::on_new_station_clicked));
            button_save.signal_clicked().connect(sigc::mem_fun(
                *this, &NetworkInteractionConfigurator::on_save_clicked));
            button_delete.signal_clicked().connect(sigc::mem_fun(
                *this, &NetworkInteractionConfigurator::on_delete_clicked));
            button_make_curr_server.signal_clicked().connect([this]() {
                int idx =
                    std::stoi(static_cast<std::string>(entry_id.get_text()));
                Gtk::TreeModel::Children stations =
                    filter_combobox_stations->children();
                for (Gtk::TreeModel::iterator station_iter = stations.begin();
                     station_iter != stations.end(); ++station_iter) {
                    if ((*station_iter)[station_cols.id] == idx) {
                        combobox_stations.set_active(station_iter);
                        main_settings.local_station_id = idx;
                        button_save_enable();
                        break;
                    }
                }
            });
            button_make_default.signal_clicked().connect([this]() {
                spin_port.set_value(25923);
                spin_timeout0.set_value(5);
                spin_timeout1.set_value(15);
                spin_timeout2.set_value(10);
                spin_timeout3.set_value(20);
                spin_coeff_k.set_value(12);
                spin_coeff_w.set_value(8);
            });
            button_server_address1_dns.signal_clicked().connect([this]() {
                resolve_dns_async(this, entry_server_address1.get_text(),
                                  entry_server_address1);
                button_save_enable();
            });
            button_server_address2_dns.signal_clicked().connect([this]() {
                resolve_dns_async(this, entry_server_address2.get_text(),
                                  entry_server_address2);
                button_save_enable();
            });
            button_server_address3_dns.signal_clicked().connect([this]() {
                resolve_dns_async(this, entry_server_address3.get_text(),
                                  entry_server_address3);
                button_save_enable();
            });
            button_client_address1_dns.signal_clicked().connect([this]() {
                resolve_dns_async(this, entry_client_address1.get_text(),
                                  entry_client_address1);
                button_save_enable();
            });
            button_client_address2_dns.signal_clicked().connect([this]() {
                resolve_dns_async(this, entry_client_address2.get_text(),
                                  entry_client_address2);
                button_save_enable();
            });
            button_client_address3_dns.signal_clicked().connect([this]() {
                resolve_dns_async(this, entry_client_address3.get_text(),
                                  entry_client_address3);
                button_save_enable();
            });
            button_servers_swap.signal_clicked().connect([this]() {
                if (entry_server_address3.get_text().empty()) {
                    std::string tmp = entry_server_address1.get_text();
                    entry_server_address1.set_text(
                        entry_server_address2.get_text());
                    entry_server_address2.set_text(tmp);
                } else {
                    std::string tmp = entry_server_address1.get_text();
                    entry_server_address1.set_text(
                        entry_server_address3.get_text());
                    entry_server_address3.set_text(
                        entry_server_address2.get_text());
                    entry_server_address2.set_text(tmp);
                }
            });
            button_clients_swap.signal_clicked().connect([this]() {
                if (entry_client_address3.get_text().empty()) {
                    std::string tmp = entry_client_address1.get_text();
                    entry_client_address1.set_text(
                        entry_client_address2.get_text());
                    entry_client_address2.set_text(tmp);
                } else {
                    std::string tmp = entry_client_address1.get_text();
                    entry_client_address1.set_text(
                        entry_client_address3.get_text());
                    entry_client_address3.set_text(
                        entry_client_address2.get_text());
                    entry_client_address2.set_text(tmp);
                }
            });

            combobox_stations.signal_changed().connect([this]() {
                Gtk::TreeModel::iterator iter = combobox_stations.get_active();
                if (!iter) {
                    return;
                }
                if ((*iter)[station_cols.id] ==
                    main_settings.local_station_id) {
                    return;
                }
                main_settings.local_station_id = (*iter)[station_cols.id];
                button_save_enable();
            });

            checkbutton_autodetection.signal_toggled().connect([this]() {
                if (checkbutton_autodetection.get_active()) {
                    std::string active_ip = get_active_ipv4_address();
                    int i = 0;
                    if (active_ip.empty())
                        stack_autodetection.set_visible_child(
                            combobox_stations_error_label);
                    bool found = false;
                    Gtk::TreeModel::Children stations =
                        servers_tree_row.children();
                    for (Gtk::TreeModel::iterator station_iter =
                             stations.begin();
                         station_iter != stations.end(); ++station_iter) {
                        if ((*station_iter)[station_cols.server_address1] ==
                                active_ip ||
                            (*station_iter)[station_cols.server_address2] ==
                                active_ip ||
                            (*station_iter)[station_cols.server_address3] ==
                                active_ip) {
                            treeview.get_selection()->select(station_iter);
                            combobox_stations.set_active(i);
                            found = true;
                            break;
                        }
                        i++;
                    }
                    if (!found) {
                        stack_autodetection.set_visible_child(
                            combobox_stations_error_label);
                        if (!stored_curr_station_row)
                            treeview.get_selection()->select(
                                servers_tree_row.children().begin());
                    }
                    if (main_settings.local_station_id != -1)
                        button_save_enable();
                    main_settings.local_station_id = -1;

                    combobox_stations.set_sensitive(false);
                    button_make_curr_server.set_sensitive(false);
                } else {
                    stack_autodetection.set_visible_child(combobox_stations);
                    main_settings.local_station_id = 1;
                    combobox_stations.set_sensitive(true);
                    button_make_curr_server.set_sensitive(true);
                }
            });
            checkbutton_allow_write.signal_toggled().connect([this]() {
                if (stored_curr_station_row[station_cols.allow_write] ==
                    checkbutton_allow_write.get_active()) {
                    return;
                }
                stored_curr_station_row[station_cols.allow_write] =
                    checkbutton_allow_write.get_active();
                button_save_enable();
            });
            checkbutton_is_reserved.signal_toggled().connect([this]() {
                if (stored_curr_station_row[station_cols.is_reserved] ==
                    checkbutton_is_reserved.get_active()) {
                    return;
                }
                stored_curr_station_row[station_cols.is_reserved] =
                    checkbutton_is_reserved.get_active();
                button_save_enable();
            });
            checkbutton_proxy.signal_toggled().connect([this]() {
                if ((stored_curr_station_row[station_cols.proxy_address] ==
                     "Нет") == !checkbutton_proxy.get_active()) {
                    return;
                }
                entry_proxy_address.set_sensitive(
                    checkbutton_proxy.get_active());
                if (checkbutton_proxy.get_active()) {
                    entry_proxy_address.set_text("");
                    stored_curr_station_row[station_cols.proxy_address] = "";
                } else {
                    entry_proxy_address.set_text("Нет");
                    stored_curr_station_row[station_cols.proxy_address] = "Нет";
                }
                button_save_enable();
            });
            checkbutton_free_read.signal_toggled().connect([this]() {
                if (stored_curr_station_row[station_cols.free_read] ==
                    checkbutton_free_read.get_active()) {
                    return;
                }
                stored_curr_station_row[station_cols.free_read] =
                    checkbutton_free_read.get_active();
                button_save_enable();
            });
            checkbutton_free_write.signal_toggled().connect([this]() {
                if (stored_curr_station_row[station_cols.free_write] ==
                    checkbutton_free_write.get_active()) {
                    return;
                }
                stored_curr_station_row[station_cols.free_write] =
                    checkbutton_free_write.get_active();
                button_save_enable();
            });

            entry_name.signal_changed().connect([this]() {
                if (stored_curr_station_row[station_cols.name] ==
                    entry_name.get_text()) {
                    return;
                }
                stored_curr_station_row[station_cols.name] =
                    entry_name.get_text();
                button_save_enable();
            });
            entry_comments.signal_changed().connect([this]() {
                if (stored_curr_station_row[station_cols.comments] ==
                    entry_comments.get_text()) {
                    return;
                }
                stored_curr_station_row[station_cols.comments] =
                    entry_comments.get_text();
                button_save_enable();
            });
            entry_server_address1.signal_changed().connect([this]() {
                if (stored_curr_station_row[station_cols.server_address1] ==
                    entry_server_address1.get_text()) {
                    return;
                }
                stored_curr_station_row[station_cols.server_address1] =
                    entry_server_address1.get_text();
                button_save_enable();
            });
            entry_server_address2.signal_changed().connect([this]() {
                if (stored_curr_station_row[station_cols.server_address2] ==
                    entry_server_address2.get_text()) {
                    return;
                }
                stored_curr_station_row[station_cols.server_address2] =
                    entry_server_address2.get_text();
                button_save_enable();
            });
            entry_server_address3.signal_changed().connect([this]() {
                if (stored_curr_station_row[station_cols.server_address3] ==
                    entry_server_address3.get_text()) {
                    return;
                }
                stored_curr_station_row[station_cols.server_address3] =
                    entry_server_address3.get_text();
                button_save_enable();
            });
            entry_client_address1.signal_changed().connect([this]() {
                if (stored_curr_station_row[station_cols.client_address1] ==
                    entry_client_address1.get_text()) {
                    return;
                }
                stored_curr_station_row[station_cols.client_address1] =
                    entry_client_address1.get_text();
                button_save_enable();
            });
            entry_client_address2.signal_changed().connect([this]() {
                if (stored_curr_station_row[station_cols.client_address2] ==
                    entry_client_address2.get_text()) {
                    return;
                }
                stored_curr_station_row[station_cols.client_address2] =
                    entry_client_address2.get_text();
                button_save_enable();
            });
            entry_client_address3.signal_changed().connect([this]() {
                if (stored_curr_station_row[station_cols.client_address3] ==
                    entry_client_address3.get_text()) {
                    return;
                }
                stored_curr_station_row[station_cols.client_address3] =
                    entry_client_address3.get_text();
                button_save_enable();
            });
            entry_proxy_address.signal_changed().connect([this]() {
                if (stored_curr_station_row[station_cols.proxy_address] ==
                    entry_proxy_address.get_text()) {
                    return;
                }
                if (!entry_proxy_address.get_text().empty()) {
                    stored_curr_station_row[station_cols.proxy_address] =
                        entry_proxy_address.get_text();
                    button_save_enable();
                }
            });

            spin_timeout.signal_value_changed().connect([this]() {
                if (stored_curr_station_row[station_cols.timeout] ==
                    (int)spin_timeout.get_value()) {
                    return;
                }
                stored_curr_station_row[station_cols.timeout] =
                    (int)spin_timeout.get_value();
                button_save_enable();
            });
            spin_tries.signal_value_changed().connect([this]() {
                if (stored_curr_station_row[station_cols.tries] ==
                    (int)spin_tries.get_value()) {
                    return;
                }
                stored_curr_station_row[station_cols.tries] =
                    (int)spin_tries.get_value();
                button_save_enable();
            });
            spin_port.signal_value_changed().connect([this]() {
                if (stored_curr_station_row[station_cols.port] ==
                    (int)spin_port.get_value()) {
                    return;
                }
                stored_curr_station_row[station_cols.port] =
                    (int)spin_port.get_value();
                button_save_enable();
            });
            spin_timeout0.signal_value_changed().connect([this]() {
                if (stored_curr_station_row[station_cols.timeout0] ==
                    (int)spin_timeout0.get_value()) {
                    return;
                }
                stored_curr_station_row[station_cols.timeout0] =
                    (int)spin_timeout0.get_value();
                button_save_enable();
            });
            spin_timeout1.signal_value_changed().connect([this]() {
                if (stored_curr_station_row[station_cols.timeout1] ==
                    (int)spin_timeout1.get_value()) {
                    return;
                }
                stored_curr_station_row[station_cols.timeout1] =
                    (int)spin_timeout1.get_value();
                button_save_enable();
            });
            spin_timeout2.signal_value_changed().connect([this]() {
                if (stored_curr_station_row[station_cols.timeout2] ==
                    (int)spin_timeout2.get_value()) {
                    return;
                }
                stored_curr_station_row[station_cols.timeout2] =
                    (int)spin_timeout2.get_value();
                button_save_enable();
            });
            spin_timeout3.signal_value_changed().connect([this]() {
                if (stored_curr_station_row[station_cols.timeout3] ==
                    (int)spin_timeout3.get_value()) {
                    return;
                }
                stored_curr_station_row[station_cols.timeout3] =
                    (int)spin_timeout3.get_value();
                button_save_enable();
            });
            spin_coeff_k.signal_value_changed().connect([this]() {
                if (stored_curr_station_row[station_cols.coeff_k] ==
                    (int)spin_coeff_k.get_value()) {
                    return;
                }
                stored_curr_station_row[station_cols.coeff_k] =
                    (int)spin_coeff_k.get_value();
                button_save_enable();
            });
            spin_coeff_w.signal_value_changed().connect([this]() {
                if (stored_curr_station_row[station_cols.coeff_w] ==
                    (int)spin_coeff_w.get_value()) {
                    return;
                }
                stored_curr_station_row[station_cols.coeff_w] =
                    (int)spin_coeff_w.get_value();
                button_save_enable();
            });
        }

        void setup_accel_groups() {
            auto accel_group = Gtk::AccelGroup::create();
            add_accel_group(accel_group);
            button_save.add_accelerator("clicked", accel_group, GDK_KEY_s,
                                        Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
        }

        void setup_data(const std::string &project_path) {
            std::filesystem::path p(project_path);
            if (!std::filesystem::exists(p) || p.filename() != "kaskad.kpr") {
                Gtk::MessageDialog dialog(
                    std::string("Не удалось найти файл проекта: ") +
                        project_path,
                    false, Gtk::MESSAGE_ERROR);
                dialog.run();
                // TODO :
                std::exit(1);
            }

            std::string errors;
            parse_config(config_path, main_settings, servers_tree_row,
                         treestore_stations, station_cols, next_station_id,
                         errors);
            if (!errors.empty()) {
                Gtk::MessageDialog dialog(
                    std::string("Не удалось прочитать файл конфигурации\n\n") +
                        errors,
                    false, Gtk::MESSAGE_ERROR);
                dialog.run();
            }
            main_settings.stations_amount = next_station_id - 1;

            if (next_station_id == 1) {
                // Создание первой станции и сохранение файла если конфиг
                // пуст
                Gtk::TreeModel::Row row =
                    *(treestore_stations->append(servers_tree_row.children()));
                main_settings.stations_amount++;
                row[station_cols.icon] = pixbuf_station;
                row[station_cols.id] = next_station_id++;
                row[station_cols.name] = "Новая станция";
                row[station_cols.comments] = "";
                row[station_cols.port] = (int)spin_port.get_value();
                row[station_cols.timeout] = (int)spin_timeout.get_value();
                row[station_cols.tries] = (int)spin_tries.get_value();
                row[station_cols.timeout0] = (int)spin_timeout0.get_value();
                row[station_cols.timeout1] = (int)spin_timeout1.get_value();
                row[station_cols.timeout2] = (int)spin_timeout2.get_value();
                row[station_cols.timeout3] = (int)spin_timeout3.get_value();
                row[station_cols.coeff_k] = (int)spin_coeff_k.get_value();
                row[station_cols.coeff_w] = (int)spin_coeff_w.get_value();
                row[station_cols.is_reserved] =
                    checkbutton_is_reserved.get_active();
                row[station_cols.server_address1] = "127.0.0.1";
                row[station_cols.server_address2] = "";
                row[station_cols.server_address3] = "";
                row[station_cols.client_address1] = "127.0.0.1";
                row[station_cols.client_address2] = "";
                row[station_cols.client_address3] = "";
                row[station_cols.proxy_address] = "Нет";
                row[station_cols.free_read] = true;
                row[station_cols.free_write] = false;
                if (errors.empty())
                    on_save_clicked();
            } else {
                // Установка иконки серверов
                Gtk::TreeModel::Children stations = servers_tree_row.children();
                for (Gtk::TreeModel::iterator station_iter = stations.begin();
                     station_iter != stations.end(); ++station_iter) {
                    (*station_iter)[station_cols.icon] = pixbuf_station;
                }
            }
            servers_tree_row[station_cols.icon] = pixbuf_station;
            treeview.expand_all();

            Gtk::TreeModel::Path servers_path =
                treestore_stations->get_path(servers_tree_row);
            filter_combobox_stations =
                Gtk::TreeModelFilter::create(treestore_stations, servers_path);
            filter_combobox_stations->set_visible_func(
                [](const Gtk::TreeModel::const_iterator &iter) {
                    (void)iter;
                    return true;
                });
            combobox_stations.clear();
            combobox_stations.set_model(filter_combobox_stations);
            combobox_stations.pack_start(renderer_name);
            combobox_stations.add_attribute(renderer_name.property_text(),
                                            station_cols.name);
            stack_autodetection.set_visible_child(combobox_stations);

            // Выбор текущей станции по LocalStationId
            Gtk::TreeModel::Children stations = servers_tree_row.children();
            if (main_settings.local_station_id > 0) {
                int combobox_stations_idx = 0;
                for (Gtk::TreeModel::iterator station_iter = stations.begin();
                     station_iter != stations.end();
                     ++station_iter, ++combobox_stations_idx) {
                    if ((*station_iter)[station_cols.id] ==
                        main_settings.local_station_id) {
                        treeview.get_selection()->select(station_iter);
                        combobox_stations.set_active(combobox_stations_idx);
                        break;
                    }
                }
            } else {
                checkbutton_autodetection.set_active(true);
                treeview.get_selection()->select(
                    servers_tree_row.children().begin());
                button_save_disable();
            }
        }

        Gtk::HeaderBar header_bar;
        Gtk::Box top_bar{Gtk::ORIENTATION_HORIZONTAL, 10};
        Gtk::Box vbox_main{Gtk::ORIENTATION_VERTICAL, 10};
        Gtk::Separator top_bar_separator1{Gtk::ORIENTATION_VERTICAL};
        Gtk::Separator top_bar_separator2{Gtk::ORIENTATION_VERTICAL};
        Gtk::Label combobox_stations_label;
        Gtk::Paned paned_main{Gtk::ORIENTATION_HORIZONTAL};
        Gtk::Frame frame_tree;
        Gtk::Box vbox_form{Gtk::ORIENTATION_VERTICAL, 10};
        Gtk::Frame frame_general;
        Gtk::Label frame_general_label;
        Gtk::Box vbox_general{Gtk::ORIENTATION_VERTICAL, 10};
        Gtk::Grid grid_general;
        Gtk::Label label_form_id, label_form_name, label_form_comments;
        Gtk::Box hbox_general_id{Gtk::ORIENTATION_HORIZONTAL};
        Gtk::Frame frame_udp_settings;
        Gtk::Label frame_udp_settings_label;
        Gtk::Box hbox_udp_settings{Gtk::ORIENTATION_HORIZONTAL, 10};
        Gtk::Frame frame_iec_settings;
        Gtk::Label frame_iec_settings_label;
        Gtk::Box hbox_iec_settings{Gtk::ORIENTATION_HORIZONTAL, 10};
        Gtk::Frame frame_timeouts;
        Gtk::Label frame_timeouts_label;
        Gtk::Box hbox_timeouts{Gtk::ORIENTATION_HORIZONTAL};
        Gtk::Box vbox_timeouts1{Gtk::ORIENTATION_VERTICAL};
        Gtk::Box vbox_timeouts2{Gtk::ORIENTATION_VERTICAL};
        Gtk::Frame frame_coeffs;
        Gtk::Label frame_coeffs_label;
        Gtk::Box hbox_coeffs{Gtk::ORIENTATION_HORIZONTAL};
        Gtk::Box vbox_coeffs{Gtk::ORIENTATION_VERTICAL};
        Gtk::Frame frame_servers;
        Gtk::Label frame_servers_label;
        Gtk::Box vbox_main_servers{Gtk::ORIENTATION_VERTICAL, 10};
        Gtk::Box hbox_servers{Gtk::ORIENTATION_HORIZONTAL};
        Gtk::Box vbox_servers{Gtk::ORIENTATION_VERTICAL};
        Gtk::Frame frame_clients;
        Gtk::Label frame_clients_label;
        Gtk::Box vbox_main_clients{Gtk::ORIENTATION_VERTICAL, 10};
        Gtk::Box hbox_clients{Gtk::ORIENTATION_HORIZONTAL};
        Gtk::Box vbox_clients{Gtk::ORIENTATION_VERTICAL};
        Gtk::Frame frame_restrictions;
        Gtk::Label frame_restrictions_label;
        Gtk::Label label_restrictions_unregistered;
        Gtk::Box vbox_restrictions{Gtk::ORIENTATION_VERTICAL, 10};

        Gtk::MenuButton menubutton_file, menubutton_edit, menubutton_help;
        Gtk::Menu menu_file, menu_edit, menu_help;
        Gtk::MenuItem menuitem_save, menuitem_exit, menuitem_new_station,
            menuitem_delete, menuitem_help;
        Gtk::Label menuitem_save_label, menuitem_exit_label,
            menuitem_new_station_label, menuitem_delete_label,
            menuitem_help_label;
        Gtk::Box menuitem_save_box{Gtk::ORIENTATION_HORIZONTAL, 5},
            menuitem_exit_box{Gtk::ORIENTATION_HORIZONTAL, 5},
            menuitem_new_station_box{Gtk::ORIENTATION_HORIZONTAL, 5},
            menuitem_delete_box{Gtk::ORIENTATION_HORIZONTAL, 5},
            menuitem_help_box{Gtk::ORIENTATION_HORIZONTAL, 5};
        Gtk::Image menuitem_save_icon, menuitem_new_station_icon,
            menuitem_delete_icon, menuitem_help_icon;

        Gtk::ComboBoxText combobox_stations;
        Gtk::ScrolledWindow scrolled_tree, scrolled_form;
        Gtk::TreeView treeview;
        Gtk::Button button_new_station, button_save, button_delete,
            button_make_curr_server, button_make_default,
            button_server_address1_dns, button_server_address2_dns,
            button_server_address3_dns, button_client_address1_dns,
            button_client_address2_dns, button_client_address3_dns,
            button_servers_swap, button_clients_swap;
        Gtk::CheckButton checkbutton_autodetection, checkbutton_allow_write,
            checkbutton_is_reserved, checkbutton_proxy, checkbutton_free_read,
            checkbutton_free_write;
        Gtk::Entry entry_id, entry_name, entry_comments, entry_server_address1,
            entry_server_address2, entry_server_address3, entry_client_address1,
            entry_client_address2, entry_client_address3, entry_proxy_address;
        Gtk::SpinButton spin_timeout, spin_timeout0, spin_timeout1,
            spin_timeout2, spin_timeout3, spin_coeff_k, spin_coeff_w, spin_port,
            spin_tries;
        Gtk::Stack stack_autodetection;
        Gtk::Label combobox_stations_error_label;
        Gtk::Image button_save_icon, button_delete_icon,
            button_new_station_icon;
        Glib::RefPtr<Gdk::Pixbuf> pixbuf_save_enabled, pixbuf_save_disabled,
            pixbuf_station, pixbuf_new_station, pixbuf_delete, pixbuf_help;
        Gtk::CellRendererPixbuf renderer_icon;
        Gtk::CellRendererText renderer_id, renderer_name, renderer_comments;
        Gtk::TreeViewColumn treecolumn_icon_idx, treecolumn_comments,
            treecolumn_name;

        Glib::RefPtr<Gtk::TreeStore> treestore_stations;
        Glib::RefPtr<Gtk::TreeModelFilter> filter_combobox_stations;
        // Используется при обновлении элементов формы для эффективного
        // последовательного доступа к данным выбранной станции
        Gtk::TreeModel::Row stored_curr_station_row;
        Gtk::TreeModel::Row servers_tree_row;
        MainSettings main_settings;
        int next_station_id;
        StationCols station_cols;
        std::string config_path;
        bool unsaved = false;
};

int main(int argc, char *argv[]) {
    std::string project_path;
    if (argc > 1) {
        project_path = argv[1];
    } else {
        project_path = "/usr/share/SCADAProject/kaskad.kpr";
    }

    auto app = Gtk::Application::create("kascad.net-configurator");
    NetworkInteractionConfigurator window(project_path);
    return app->run(window);
}

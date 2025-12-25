#include <gtkmm.h>

class StationCols : public Gtk::TreeModel::ColumnRecord {
    public:
        StationCols() {
            add(icon);
            add(id);
            add(name);
            add(comments);
            add(allow_write);
            add(port);
            add(timeout);
            add(tries);
            add(timeout0);
            add(timeout1);
            add(timeout2);
            add(timeout3);
            add(coeff_k);
            add(coeff_w);
            add(is_reserved);
            add(server_address1);
            add(server_address2);
            add(server_address3);
            add(client_address1);
            add(client_address2);
            add(client_address3);
            add(proxy_address);
            add(free_read);
            add(free_write);
        }

        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> icon;
        Gtk::TreeModelColumn<int> id;
        Gtk::TreeModelColumn<Glib::ustring> name;
        Gtk::TreeModelColumn<Glib::ustring> comments;
        Gtk::TreeModelColumn<bool> allow_write;
        Gtk::TreeModelColumn<int> port;
        Gtk::TreeModelColumn<int> timeout;
        Gtk::TreeModelColumn<int> tries;
        Gtk::TreeModelColumn<int> timeout0;
        Gtk::TreeModelColumn<int> timeout1;
        Gtk::TreeModelColumn<int> timeout2;
        Gtk::TreeModelColumn<int> timeout3;
        Gtk::TreeModelColumn<int> coeff_k;
        Gtk::TreeModelColumn<int> coeff_w;
        Gtk::TreeModelColumn<bool> is_reserved;
        Gtk::TreeModelColumn<Glib::ustring> server_address1;
        Gtk::TreeModelColumn<Glib::ustring> server_address2;
        Gtk::TreeModelColumn<Glib::ustring> server_address3;
        Gtk::TreeModelColumn<Glib::ustring> client_address1;
        Gtk::TreeModelColumn<Glib::ustring> client_address2;
        Gtk::TreeModelColumn<Glib::ustring> client_address3;
        Gtk::TreeModelColumn<Glib::ustring> proxy_address;
        Gtk::TreeModelColumn<bool> free_read;
        Gtk::TreeModelColumn<bool> free_write;
};

struct MainSettings {
    public:
        std::string file_description;
        int file_version;
        bool ingore_local_settings;
        int local_station_id;
        int stations_amount;
        int client_stations_amount;
        bool debug_reg;

        MainSettings() {
            file_description = "SCADA list stations";
            file_version = 1;
            ingore_local_settings = false;
            local_station_id = -1;
            stations_amount = 0;
            client_stations_amount = 0;
            debug_reg = false;
        }
};

void set_margin(Gtk::Widget &widget, int margin_horizontal,
                int margin_vertical);
int get_missing_id(const Gtk::TreeModel::Row &servers_row,
                   const StationCols &station_cols);
void resolve_dns_async(Gtk::Window *window, const std::string &dns,
                       Gtk::Entry &entry);
std::string get_active_ipv4_address();

bool parse_config(const std::string &config_path, MainSettings &main_settings,
                  Gtk::TreeModel::Row &servers_row,
                  Glib::RefPtr<Gtk::TreeStore> treestore,
                  const StationCols &station_cols, int &newest_station_id,
                  std::string &errors);
int write_config(const std::string &config_path,
                 const MainSettings &main_settings,
                 const Gtk::TreeModel::Row &servers_row,
                 const StationCols &station_cols, std::string &errors);
int write_backup(const std::string &config_path, std::string &errors);

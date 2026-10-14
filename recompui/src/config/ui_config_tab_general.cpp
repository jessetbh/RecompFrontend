#include "librecomp/config.hpp"
#include "librecomp/save_backups.hpp"
#include "recompui/config.h"
#include "recompui/recompui.h"
#include "util/steam_deck.h"

namespace recompui {

static bool created_general_config = false;

namespace config {
    recomp::config::Config &get_general_config() {
        if (!created_general_config) {
            throw std::runtime_error("General config has not been created yet. Call create_general_tab() first.");
        }
        return config::get_config(config::general::id);
    }

    using EnumOptionVector = const std::vector<recomp::config::ConfigOptionEnumOption>;
    enum class BackgroundInputMode {
        Off,
        On,
        OptionCount
    };
    static EnumOptionVector background_input_options = {
        {BackgroundInputMode::Off, "Off"},
        {BackgroundInputMode::On, "On"},
    };

    // [wcw saves] "Restore Save Backup" (aki-roadmap#50): pick the newest, 2nd newest, ... automatic backup; a prompt
    // says which file and when, and the restore happens the next time the game starts (librecomp save_backups.hpp).
    static EnumOptionVector restore_backup_options = {
        {0u, "None", "Choose..."},
        {1u, "Newest", "Newest backup"},
        {2u, "Backup2", "2nd newest"},
        {3u, "Backup3", "3rd newest"},
        {4u, "Backup4", "4th newest"},
        {5u, "Backup5", "5th newest"},
        {6u, "Backup6", "6th newest"},
        {7u, "Backup7", "7th newest"},
        {8u, "Backup8", "8th newest"},
        {9u, "Backup9", "9th newest"},
        {10u, "Backup10", "10th newest"},
    };

    static std::string save_kind(const recomp::save_backups::BackupEntry &entry) {
        return entry.controller_pak ? "Controller Pak" : "game save";
    }

    static void reset_restore_choice() {
        get_general_config().update_option_value(general::options::restore_save_backup, 0u);
    }

    static void on_restore_backup_chosen(uint32_t choice) {
        if (choice == 0) {
            return;
        }

        const std::filesystem::path main_save = recomp::save_backups::main_save_file();
        const std::vector<recomp::save_backups::BackupEntry> backups = recomp::save_backups::list_backups(main_save);
        if ((choice - 1) >= backups.size()) {
            const std::string text = backups.empty() ?
                "There are no save backups yet. They're made automatically when the game starts and while you play." :
                "There are only " + std::to_string(backups.size()) + " save backups.";
            recompui::open_info_prompt("Restore Save Backup", text, "OK", reset_restore_choice, recompui::ButtonStyle::Primary);
            return;
        }

        const recomp::save_backups::BackupEntry entry = backups[choice - 1];
        std::string text = "Restore the " + save_kind(entry) + " backed up on " + recomp::save_backups::display_time(entry.when) + "?";
        if (const std::optional<recomp::save_backups::Timestamp> last = recomp::save_backups::last_write_time(entry.save_file)) {
            text += "<br/><br/>The current " + save_kind(entry) + " was last saved on " + recomp::save_backups::display_time(*last) + ".";
        }
        text += "<br/><br/>It will be restored the next time the game starts. The current save is backed up first, so you can undo this.";
        recompui::open_choice_prompt("Restore Save Backup", text, "Restore", "Cancel",
            [entry]() {
                const bool ok = recomp::save_backups::request_restore(entry);
                reset_restore_choice();
                recompui::open_notification("Restore Save Backup", ok ?
                    "Restore scheduled: it happens the next time the game starts." :
                    "Couldn't schedule the restore (check the save folder's permissions).");
            },
            reset_restore_choice,
            recompui::ButtonStyle::Success, recompui::ButtonStyle::Danger, true);
    }

    template <typename T = uint32_t>
    T get_general_config_enum_value(const std::string& option_id) {
        return static_cast<T>(std::get<uint32_t>(get_general_config().get_option_value(option_id)));
    }

    template <typename T = uint32_t>
    T get_general_config_number_value(const std::string& option_id) {
        return static_cast<T>(std::get<double>(get_general_config().get_option_value(option_id)));
    }

    bool get_general_config_bool_value(const std::string& option_id) {
        return std::get<bool>(get_general_config().get_option_value(option_id));
    }
    
    bool general::has_rumble_strength_option() {
        return get_general_config().has_option(general::options::rumble_strength);
    }

    double general::get_rumble_strength() {
        return get_general_config_number_value<double>(general::options::rumble_strength);
    }

    bool general::has_gyro_sensitivity_option() {
        return get_general_config().has_option(general::options::gyro_sensitivity);
    }
    
    double general::get_gyro_sensitivity() {
        return get_general_config_number_value<double>(general::options::gyro_sensitivity);
    }
    
    bool general::has_mouse_sensitivity_option() {
        return get_general_config().has_option(general::options::mouse_sensitivity);
    }
    
    double general::get_mouse_sensitivity() {
        return get_general_config_number_value<double>(general::options::mouse_sensitivity);
    }
    
    double general::get_joystick_deadzone() {
        return get_general_config_number_value<double>(general::options::joystick_deadzone);
    }
    
    bool general::get_background_input_mode_enabled() {
        return get_general_config_enum_value<BackgroundInputMode>(general::options::background_input_mode) == BackgroundInputMode::On;
    }
    
    bool general::get_debug_mode_enabled() {
        return get_general_config_bool_value(general::options::debug_mode);
    }

    recomp::config::Config &create_general_tab(const GeneralTabOptions& options, const std::string &name) {
        created_general_config = true;
        recomp::config::Config &config = recompui::config::create_config_tab(name, general::id, false);

        config.add_bool_option(
            general::options::debug_mode,
            "Debug Mode",
            "Enables debugging features.",
            false,
            true // hidden by default
        );

        if (options.has_rumble_strength) {
            config.add_percent_number_option(
                general::options::rumble_strength,
                "Rumble Strength",
                "Controls the strength of rumble when using a controller that supports it. "
                "<b>Setting this to zero will disable rumble.</b>",
                25.0
            );
        }

        if (options.has_gyro_sensitivity) {
            config.add_percent_number_option(
                general::options::gyro_sensitivity,
                "Gyro Sensitivity",
                "Controls the sensitivity of gyro aiming when using items in first person for controllers that support it."
                "<b>Setting this to zero will disable gyro.</b>"
                "<br />"
                "<br />"
                "<b>Note: To recalibrate controller gyro, set the controller down on a still, flat surface for 5 seconds.</b>",
                25.0
            );
        }

        if (options.has_mouse_sensitivity) {
            config.add_percent_number_option(
                general::options::mouse_sensitivity,
                "Mouse Sensitivity",
                "Controls the sensitivity of mouse aiming when using items in first person. <b>Setting this to zero will disable mouse aiming.</b>"
                "<br />"
                "<br />"
                "<b>Note: This option does not allow mouse buttons to activate items. Mouse aiming is intended to be used with inputs that are mapped to mouse movement, such as gyro on Steam Deck.</b>",
                is_steam_deck() ? 50.0 : 0.0 // steam deck overrides default to 50%
            );
        }

        config.add_percent_number_option(
            general::options::joystick_deadzone,
            "Joystick Deadzone",
            "Applies a deadzone to joystick inputs.",
            5.0
        );

        config.add_enum_option(
            general::options::background_input_mode,
            "Background Input Mode",
            "Allows the game to read controller input when out of focus."
            "<br/>"
            "<b>This setting does not affect keyboard input.</b>",
            background_input_options,
            BackgroundInputMode::On
        );

        config.add_enum_option(
            general::options::restore_save_backup,
            "Restore Save Backup",
            "Your saves are backed up automatically when the game starts and while you play (the last 10 of each, in "
            "the <b>saves\\backups</b> folder). Pick one to restore it: you'll be asked to confirm, and the restore "
            "happens the next time the game starts.",
            restore_backup_options,
            0u
        );
        config.add_option_change_callback(general::options::restore_save_backup,
            [](recomp::config::ConfigValueVariant cur_value, recomp::config::ConfigValueVariant, recomp::config::OptionChangeContext change_context) {
                if (change_context == recomp::config::OptionChangeContext::Permanent) {
                    on_restore_backup_chosen(std::get<uint32_t>(cur_value));
                }
            });

        return config;
    }
} // namespace config
} // namespace recompui

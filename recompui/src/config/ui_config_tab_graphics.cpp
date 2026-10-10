#include "librecomp/config.hpp"
#include "recompui/config.h"
#include "recompui/renderer.h"
#include "util/steam_deck.h"
#include "render/rt64_crt_filter.h"
#include "render/rt64_hdr_output.h"
#include "render/rt64_wcw_raytracer.h"

static bool created_graphics_config = false;

namespace recompui {
    namespace config {
        recomp::config::Config &get_graphics_config() {
            if (!created_graphics_config) {
                throw std::runtime_error("Graphics config has not been created yet. Call create_graphics_tab() first.");
            }
            return config::get_config(config::graphics::id);
        }

        static ultramodern::renderer::WindowMode wm_default() {
            return is_steam_deck() ? ultramodern::renderer::WindowMode::Fullscreen : ultramodern::renderer::WindowMode::Windowed;
        }

        using EnumOptionVector = const std::vector<recomp::config::ConfigOptionEnumOption>;
        static EnumOptionVector resolution_options = {
            {ultramodern::renderer::Resolution::Original, "Original", "Original"},
            {ultramodern::renderer::Resolution::Original2x, "Original2x", "Original 2x"},
            {ultramodern::renderer::Resolution::Auto, "Auto", "Auto"},
        };

        enum class DownsamplingOption {
            Off = 0,
            X2 = 2,
            X4 = 4,
        };
        static EnumOptionVector downsampling_options = {
            {DownsamplingOption::Off, "Off"},
            {DownsamplingOption::X2, "2x"},
            {DownsamplingOption::X4, "4x"},
        };

        static EnumOptionVector window_mode_options = {
            {ultramodern::renderer::WindowMode::Windowed, "Windowed"},
            {ultramodern::renderer::WindowMode::Fullscreen, "Fullscreen"}
        };

        #if defined(_WIN32)
        #define ALLOW_D3D12
        #endif
        #if defined(_WIN32) || defined(__linux__)
        #define ALLOW_VULKAN
        #endif
        #if defined(__APPLE__)
        #define ALLOW_METAL
        #endif

        static EnumOptionVector graphics_api_options = {
            {ultramodern::renderer::GraphicsApi::Auto, "Auto"},
        #ifdef ALLOW_D3D12
            {ultramodern::renderer::GraphicsApi::D3D12, "D3D12"},
        #endif
        #ifdef ALLOW_VULKAN
            {ultramodern::renderer::GraphicsApi::Vulkan, "Vulkan"},
        #endif
        #ifdef ALLOW_METAL
            {ultramodern::renderer::GraphicsApi::Metal, "Metal"},
        #endif
        };

        // [wcw rt] Experimental hybrid ray tracing.
        enum class RayTracingOption {
            Off = 0,
            On = 1
        };

        enum class RayTracingQuality {
            Performance = 0,
            Balanced = 1,
            Quality = 2
        };

        static EnumOptionVector ray_tracing_options = {
            {RayTracingOption::Off, "Off", "Off"},
            {RayTracingOption::On, "On", "On"},
        };

        static EnumOptionVector ray_tracing_quality_options = {
            {RayTracingQuality::Performance, "Performance", "Performance"},
            {RayTracingQuality::Balanced, "Balanced", "Balanced"},
            {RayTracingQuality::Quality, "Quality", "Quality"},
        };

        // [wcw crt] CRT filter: on/off, the video signal (a preset in crt/presets/ next to the
        // executable) and the TV's color standard.
        enum class CrtFilterOption {
            Off = 0,
            On = 1
        };

        enum class CrtSignalOption {
            Composite = 0,
            SVideo = 1,
            RF = 2,
            RGB = 3
        };

        enum class CrtColorsOption {
            USA = 0,
            Japan = 1
        };

        static EnumOptionVector crt_filter_options = {
            {CrtFilterOption::Off, "Off", "Off"},
            {CrtFilterOption::On, "On", "On"},
        };

        static EnumOptionVector crt_signal_options = {
            {CrtSignalOption::Composite, "Composite", "Composite"},
            {CrtSignalOption::SVideo, "SVideo", "S-Video"},
            {CrtSignalOption::RF, "RF", "RF"},
            {CrtSignalOption::RGB, "RGB", "RGB"},
        };

        static EnumOptionVector crt_colors_options = {
            {CrtColorsOption::USA, "USA", "USA"},
            {CrtColorsOption::Japan, "Japan", "Japan"},
        };

        // [wcw hdr] HDR output.
        enum class HdrOption {
            Off = 0,
            On = 1
        };

        static EnumOptionVector hdr_options = {
            {HdrOption::Off, "Off", "Off"},
            {HdrOption::On, "On", "On"},
        };

        enum class CrtBeamOption {
            Off = 0,
            On = 1
        };

        static EnumOptionVector crt_beam_options = {
            {CrtBeamOption::Off, "Off", "Off"},
            {CrtBeamOption::On, "On", "On"},
        };

        static const char *crt_signal_preset(CrtSignalOption option) {
            switch (option) {
            case CrtSignalOption::SVideo: return "aki-svideo";
            case CrtSignalOption::RF: return "aki-rf";
            case CrtSignalOption::RGB: return "aki-rgb";
            case CrtSignalOption::Composite:
            default: return "aki-composite";
            }
        }

        static EnumOptionVector aspect_ratio_options = {
            {ultramodern::renderer::AspectRatio::Original, "Original"},
            {ultramodern::renderer::AspectRatio::Expand, "Expand"},
            // {ultramodern::renderer::AspectRatio::Manual, "Manual"},
        };

        static EnumOptionVector antialiasing_options = {
            {ultramodern::renderer::Antialiasing::None, "None"},
            {ultramodern::renderer::Antialiasing::MSAA2X, "MSAA2X", "2X"},
            {ultramodern::renderer::Antialiasing::MSAA4X, "MSAA4X", "4X"},
            // {ultramodern::renderer::Antialiasing::MSAA8X, "MSAA8X"},
        };

        static EnumOptionVector refresh_rate_options = {
            {ultramodern::renderer::RefreshRate::Original, "Original"},
            {ultramodern::renderer::RefreshRate::Display, "Display"},
            {ultramodern::renderer::RefreshRate::Manual, "Manual"},
        };

        static EnumOptionVector hpfb_options = {
            {ultramodern::renderer::HighPrecisionFramebuffer::Auto, "Auto"},
            {ultramodern::renderer::HighPrecisionFramebuffer::On, "On"},
            {ultramodern::renderer::HighPrecisionFramebuffer::Off, "Off"},
        };

        static EnumOptionVector hud_ratio_mode_options = {
            {ultramodern::renderer::HUDRatioMode::Original, "Original"},
            {ultramodern::renderer::HUDRatioMode::Clamp16x9, "Clamp16x9", "16:9"},
            {ultramodern::renderer::HUDRatioMode::Full, "Expand"},
        };

        static const std::string get_downsampling_details(ultramodern::renderer::Resolution res_option, DownsamplingOption ds_option) {
            switch (res_option) {
                default:
                case ultramodern::renderer::Resolution::Auto:
                    return "Downsampling is not available at auto resolution";
                case ultramodern::renderer::Resolution::Original:
                    if (ds_option == DownsamplingOption::X2) {
                        return "Rendered in 480p and scaled to 240p";
                    } else if (ds_option == DownsamplingOption::X4) {
                        return "Rendered in 960p and scaled to 240p";
                    }
                    return "";
                case ultramodern::renderer::Resolution::Original2x:
                    if (ds_option == DownsamplingOption::X2) {
                        return "Rendered in 960p and scaled to 480p";
                    } else if (ds_option == DownsamplingOption::X4) {
                        return "Rendered in 4K and scaled to 480p";
                    }
                    return "";
            }
        }

        using OptionChangeContext = recomp::config::OptionChangeContext;

        static recomp::config::ConfigValueVariant get_graphics_value_variant(const std::string& key, OptionChangeContext change_context) {
            if (change_context == OptionChangeContext::Temporary) {
                return get_graphics_config().get_temp_option_value(key);
            } else {
                return get_graphics_config().get_option_value(key);
            }
        }

        template <typename T, typename ValType>
        T get_graphics_value(const std::string& key, OptionChangeContext change_context = OptionChangeContext::Permanent) {
            return static_cast<T>(std::get<ValType>(get_graphics_value_variant(key, change_context)));
        }

        template <typename T = uint32_t>
        T get_graphics_enum_value(const std::string& key, OptionChangeContext change_context = OptionChangeContext::Permanent) {
            return get_graphics_value<T, uint32_t>(key, change_context);
        }
    
        template <typename T = uint32_t>
        T get_graphics_number_value(const std::string& key, OptionChangeContext change_context = OptionChangeContext::Permanent) {
            return get_graphics_value<T, double>(key, change_context);
        }

        static bool get_graphics_bool_value(const std::string& key, OptionChangeContext change_context = OptionChangeContext::Permanent) {
            return std::get<bool>(get_graphics_value_variant(key, change_context));
        }

        static void determine_downsampling_display(ultramodern::renderer::Resolution res_option, OptionChangeContext change_context) {
            DownsamplingOption ds_opt = get_graphics_enum_value<DownsamplingOption>(graphics::options::ds_option, change_context);
            get_graphics_config().update_option_enum_details(graphics::options::ds_option, get_downsampling_details(res_option, ds_opt));
        }

        static std::string get_framerate_text(uint32_t refresh_rate) {
            // [wcw fix] Framerate is LOCKED to Original for this game — see the lock below.
            return
                "Sets the game's output framerate. This option does not affect gameplay."
                "<br />"
                "<br />"
                "<recomp-color warning>Locked to Original for this game:</recomp-color> frame interpolation warps geometry with this engine's display lists (it submits pre-multiplied matrices only, which defeats the renderer's motion matching)."
                "<br />"
                "<br />"
                "<recomp-color primary>Detected display refresh rate: " + std::to_string(refresh_rate) + "hz</recomp-color>";
        }

        static void apply_graphics_config() {
            ultramodern::renderer::GraphicsConfig new_config;
            new_config.developer_mode = get_graphics_bool_value(graphics::options::developer_mode);
            new_config.res_option = get_graphics_enum_value<ultramodern::renderer::Resolution>(graphics::options::res_option);
            new_config.wm_option = get_graphics_enum_value<ultramodern::renderer::WindowMode>(graphics::options::wm_option);
            new_config.hr_option = get_graphics_enum_value<ultramodern::renderer::HUDRatioMode>(graphics::options::hr_option);
            new_config.api_option = get_graphics_enum_value<ultramodern::renderer::GraphicsApi>(graphics::options::api_option);
            new_config.ar_option = get_graphics_enum_value<ultramodern::renderer::AspectRatio>(graphics::options::ar_option);
            new_config.msaa_option = get_graphics_enum_value<ultramodern::renderer::Antialiasing>(graphics::options::msaa_option);
            new_config.rr_option = get_graphics_enum_value<ultramodern::renderer::RefreshRate>(graphics::options::rr_option);
            new_config.hpfb_option = get_graphics_enum_value<ultramodern::renderer::HighPrecisionFramebuffer>(graphics::options::hpfb_option);
            new_config.rr_manual_value = get_graphics_number_value<int>(graphics::options::rr_manual_value);

            new_config.ds_option = get_graphics_enum_value<int>(graphics::options::ds_option);

            ultramodern::renderer::set_graphics_config(new_config);

            // [wcw rt] Takes effect on the next frame (no renderer restart needed).
            RT64::WcwRaytracer::setUserSettings(
                get_graphics_enum_value<RayTracingOption>(graphics::options::rt_option) == RayTracingOption::On,
                uint32_t(get_graphics_enum_value<RayTracingQuality>(graphics::options::rt_quality_option)));

            // [wcw crt] Takes effect on the next frame; the first use of a preset compiles its shaders.
            RT64::CrtFilter::Settings crt_settings;
            crt_settings.enabled = get_graphics_enum_value<CrtFilterOption>(graphics::options::crt_filter_option) == CrtFilterOption::On;
            crt_settings.preset = crt_signal_preset(get_graphics_enum_value<CrtSignalOption>(graphics::options::crt_signal_option));
            // guest-advanced "LUT Colors": 0 = none, 1 = NTSC-J.
            const bool crt_japan = get_graphics_enum_value<CrtColorsOption>(graphics::options::crt_colors_option) == CrtColorsOption::Japan;
            crt_settings.parameters.emplace_back("TNTC", crt_japan ? 1.0f : 0.0f);
            crt_settings.beamSimulator = get_graphics_enum_value<CrtBeamOption>(graphics::options::crt_beam_option) == CrtBeamOption::On;
            RT64::CrtFilter::setUserSettings(crt_settings);

            graphics::apply_hdr_settings();
        }

        void graphics::update_msaa_supported(bool supported) {
            recomp::config::Config &config = get_graphics_config();
            if (!supported) {
                config.update_option_enum_details(
                    graphics::options::msaa_option,
                    supported ? "Available" : "Not available (missing sample positions support)"
                );
                config.update_option_disabled(
                    graphics::options::msaa_option,
                    true
                );
            } else {
                auto max_msaa = recompui::renderer::RT64MaxMSAA();
                if (max_msaa < RT64::UserConfiguration::Antialiasing::MSAA2X) {
                    config.update_enum_option_disabled(graphics::options::msaa_option, static_cast<uint32_t>(ultramodern::renderer::Antialiasing::MSAA2X), true);
                }
                if (max_msaa < RT64::UserConfiguration::Antialiasing::MSAA4X) {
                    config.update_enum_option_disabled(graphics::options::msaa_option, static_cast<uint32_t>(ultramodern::renderer::Antialiasing::MSAA4X), true);
                }
            }
        }

        // [wcw hdr] On/off takes effect when the renderer next creates its swap chain (a restart);
        // the paper white brightness applies immediately.
        void graphics::apply_hdr_settings() {
            if (!created_graphics_config) {
                return;
            }

            RT64::HdrOutput::Settings hdr_settings;
            hdr_settings.enabled = get_graphics_enum_value<HdrOption>(graphics::options::hdr_option) == HdrOption::On;
            hdr_settings.paperWhiteNits = float(get_graphics_number_value<int>(graphics::options::hdr_paper_white));
            RT64::HdrOutput::setUserSettings(hdr_settings);
        }

        void graphics::update_refresh_rate(uint32_t refresh_rate) {
            recomp::config::Config& config = get_graphics_config();
            std::string framerate_text = get_framerate_text(refresh_rate);
            config.update_option_description(graphics::options::rr_option, framerate_text);
            config.update_option_description(graphics::options::rr_manual_value, framerate_text);
        }

        void graphics::toggle_fullscreen() {
            auto current_wm = get_graphics_enum_value<ultramodern::renderer::WindowMode>(graphics::options::wm_option, OptionChangeContext::Temporary);
            auto new_wm = current_wm == ultramodern::renderer::WindowMode::Windowed ? ultramodern::renderer::WindowMode::Fullscreen : ultramodern::renderer::WindowMode::Windowed;
            get_graphics_config().set_option_value(graphics::options::wm_option, static_cast<uint32_t>(new_wm));
            get_graphics_config().apply_option_value(graphics::options::wm_option);
            apply_graphics_config();
        }

        recomp::config::Config &create_graphics_tab(const std::string &name) {
            created_graphics_config = true;

            recomp::config::Config &config = recompui::config::create_config_tab(name, graphics::id, true);
            config.set_save_callback(apply_graphics_config);
            config.set_load_callback(apply_graphics_config);

            config.add_bool_option(
                graphics::options::developer_mode,
                "Dev Mode",
                "Enables developer features.",
                false,
                true
            );

            config.add_enum_option(
                graphics::options::res_option,
                "Resolution",
                "Sets the output resolution of the game. <recomp-color primary>Original</recomp-color> matches the game's original 240p resolution. <recomp-color primary>Original 2x</recomp-color> will render at 480p. <recomp-color primary>Auto</recomp-color> will scale based on the game window's resolution.",
                resolution_options,
                ultramodern::renderer::Resolution::Auto
            );
            {
                config.add_option_change_callback(
                    graphics::options::res_option,
                    [](recomp::config::ConfigValueVariant cur_value, recomp::config::ConfigValueVariant prev_value, OptionChangeContext change_context) {
                        auto new_opt = static_cast<ultramodern::renderer::Resolution>(std::get<uint32_t>(cur_value));
                        determine_downsampling_display(new_opt, change_context);
                    }
                );
            }

            config.add_enum_option(
                graphics::options::ds_option,
                "Downsampling Quality",
                "Renders at a higher resolution and scales it down to the output resolution for increased quality. Only available in <recomp-color primary>Original</recomp-color> and <recomp-color primary>Original 2x</recomp-color> resolution."
                "<br />"
                "<br />"
                "Note: <recomp-color primary>4x</recomp-color> downsampling quality at <recomp-color primary>Original 2x</recomp-color> resolution may cause performance issues on low end devices, as it will cause the game to render <recomp-color warning>at almost 4k internal resolution</recomp-color>.",
                downsampling_options,
                DownsamplingOption::Off
            );
            {
                config.add_option_change_callback(
                    graphics::options::ds_option,
                    [](recomp::config::ConfigValueVariant cur_value, recomp::config::ConfigValueVariant prev_value, OptionChangeContext change_context) {
                        auto resolution_opt = get_graphics_enum_value<ultramodern::renderer::Resolution>(graphics::options::res_option, change_context);
                        determine_downsampling_display(resolution_opt, change_context);
                    }
                );

                config.add_option_disable_dependency(
                    graphics::options::ds_option,
                    graphics::options::res_option,
                    ultramodern::renderer::Resolution::Auto
                );
    
                config.on_json_parse_option(graphics::options::ds_option, [](const nlohmann::json& j) {
                    return j.get<uint32_t>();
                });
    
                config.on_json_serialize_option(graphics::options::ds_option, [](const recomp::config::ConfigValueVariant& value) {
                    return nlohmann::json(std::get<uint32_t>(value));
                });
            }

            config.add_enum_option(
                graphics::options::ar_option,
                "Aspect Ratio",
                "Sets the horizontal aspect ratio. <recomp-color primary>Original</recomp-color> uses the game's original 4:3 aspect ratio. <recomp-color primary>Expand</recomp-color> will adjust to match the game window's aspect ratio.",
                aspect_ratio_options,
                ultramodern::renderer::AspectRatio::Expand
            );

            config.add_enum_option(
                graphics::options::wm_option,
                "Window Mode",
                "Sets whether the game should display <recomp-color primary>Windowed</recomp-color> or <recomp-color primary>Fullscreen</recomp-color>. You can also use <recomp-color primary>F11</recomp-color> or <recomp-color primary>Alt + Enter</recomp-color> to toggle this option.",
                window_mode_options,
                wm_default()
            );

            config.add_enum_option(
                graphics::options::rr_option,
                "Framerate",
                get_framerate_text(60),
                refresh_rate_options,
                // [wcw fix] Default Original, not Display. WCW builds one visual frame from
                // MULTIPLE RSP workloads, but RT64's frame matching assumes 1 workload = 1 frame
                // (rt64_workload_queue.cpp TODO), so interpolated frames re-render only a slice
                // of the frame — pure-black / partially-drawn presents (verified via swapchain
                // readback: constant black bursts in menus at Display, zero at Original).
                ultramodern::renderer::RefreshRate::Original
            );
            {
                // [wcw fix] LOCK the framerate to Original (2026-07-05 decision). The black-frame
                // flicker WAS fixed once (multi-workload frame grouping + present-fb interpolation
                // target — preserved in git history at commit bd35fac and lib-patches there), but
                // the interpolated frames still warp geometry badly: WCW is G_FORCEMTX-only, so
                // RT64's heuristic transform matching lerps between mispaired full MVPs
                // ("polygons bounce all over"). Until game-side matrix-group patches exist, the
                // control is disabled in the UI and any saved config value (graphics.json AND its
                // .bak) is coerced back to Original at parse time.
                config.update_option_disabled(graphics::options::rr_option, true);
                config.on_json_parse_option(graphics::options::rr_option, [](const nlohmann::json &) {
                    return static_cast<uint32_t>(ultramodern::renderer::RefreshRate::Original);
                });
            }

            config.add_number_option(
                graphics::options::rr_manual_value,
                "",
                get_framerate_text(60),
                20.0, 240.0, 1.0, 0, false, 60.0
            );
            {
                config.add_option_hidden_dependency(
                    graphics::options::rr_manual_value,
                    graphics::options::rr_option,
                    ultramodern::renderer::RefreshRate::Original,
                    ultramodern::renderer::RefreshRate::Display
                );
            }

            config.add_enum_option(
                graphics::options::msaa_option,
                "MS Anti-Aliasing",
                "Sets the multisample anti-aliasing (MSAA) quality level. This reduces jagged edges in the final image at the expense of rendering performance."
                "<br />"
                "<br />"
                "<recomp-color primary>Note: This option won't be available if your GPU does not support programmable MSAA sample positions, as it is currently required to avoid rendering glitches.</recomp-color>",
                antialiasing_options,
                ultramodern::renderer::Antialiasing::MSAA2X
            );

            config.add_enum_option(
                graphics::options::hr_option,
                "HUD Placement",
                "Adjusts the placement of HUD elements to fit the selected aspect ratio. <recomp-color primary>Expand</recomp-color> will use the aspect ratio of the game's output window.",
                hud_ratio_mode_options,
                ultramodern::renderer::HUDRatioMode::Clamp16x9
            );

            config.add_enum_option(
                graphics::options::api_option,
                "Graphics API",
                "Selects the graphics API to use.",
                graphics_api_options,
                ultramodern::renderer::GraphicsApi::Auto,
                true
            );

            config.add_enum_option(
                graphics::options::hpfb_option,
                "High Precision Framebuffer",
                "Sets whether to use a high precision framebuffer.",
                hpfb_options,
                ultramodern::renderer::HighPrecisionFramebuffer::Off,
                true
            );

            config.add_enum_option(
                graphics::options::hdr_option,
                "HDR",
                "Outputs in HDR on displays with Windows HDR turned on. The CRT filter's phosphor masks can then run at full strength."
                "<br />"
                "<br />"
                "<recomp-color primary>Takes effect after restarting the game.</recomp-color> Has no effect when Windows HDR is off for the display.",
                hdr_options,
                HdrOption::Off
            );

            config.add_number_option(
                graphics::options::hdr_paper_white,
                "HDR Paper White",
                "Brightness of white in the game picture and menus with HDR on, in nits. 200 is a typical starting point; raise it in a bright room.",
                80, 400, 10, 0, false, 200
            );
            {
                config.add_option_hidden_dependency(
                    graphics::options::hdr_paper_white,
                    graphics::options::hdr_option,
                    HdrOption::Off
                );
            }

            config.add_enum_option(
                graphics::options::crt_filter_option,
                "CRT Filter",
                "Simulates a period CRT television: the video signal, scanlines, phosphor mask and glow. "
                "Looks best at higher window resolutions."
                "<br />"
                "<br />"
                "The first use of each signal takes a few seconds to prepare.",
                crt_filter_options,
                CrtFilterOption::Off
            );

            config.add_enum_option(
                graphics::options::crt_signal_option,
                "CRT Signal",
                "<recomp-color primary>Composite</recomp-color>: how most players saw the N64. "
                "<recomp-color primary>S-Video</recomp-color>: a cleaner picture on a Trinitron. "
                "<recomp-color primary>RF</recomp-color>: the RF switch, the softest and noisiest. "
                "<recomp-color primary>RGB</recomp-color>: a sharp professional monitor (enthusiast look; a stock N64 has no RGB output).",
                crt_signal_options,
                CrtSignalOption::Composite
            );

            config.add_enum_option(
                graphics::options::crt_colors_option,
                "TV Colors",
                "The color standard of the simulated TV. <recomp-color primary>Japan</recomp-color> uses NTSC-J colors (cooler whites), as on a Japanese set.",
                crt_colors_options,
                CrtColorsOption::USA
            );
            config.add_enum_option(
                graphics::options::crt_beam_option,
                "CRT Beam Simulator",
                "Simulates the CRT's rolling scan for CRT-like motion clarity (Blur Busters CRT Beam Simulator)."
                "<br />"
                "<br />"
                "<recomp-color primary>Requires a 120 Hz or faster display running at a multiple of 60 Hz, with V-Sync on.</recomp-color> "
                "Works best at 240 Hz and up, and on OLED. Not recommended with variable refresh rate (G-Sync/FreeSync) "
                "or local dimming. It has no effect otherwise.",
                crt_beam_options,
                CrtBeamOption::Off
            );
            {
                config.add_option_hidden_dependency(
                    graphics::options::crt_beam_option,
                    graphics::options::crt_filter_option,
                    CrtFilterOption::Off
                );
                config.add_option_hidden_dependency(
                    graphics::options::crt_signal_option,
                    graphics::options::crt_filter_option,
                    CrtFilterOption::Off
                );
                config.add_option_hidden_dependency(
                    graphics::options::crt_colors_option,
                    graphics::options::crt_filter_option,
                    CrtFilterOption::Off
                );
            }

            config.add_enum_option(
                graphics::options::rt_option,
                "Ray Tracing",
                "<recomp-color warning>Experimental.</recomp-color> Adds ray-traced shadows from the arena lights and contact shadows, replacing the game's blob shadows."
                "<br />"
                "<br />"
                "<recomp-color primary>Requires an RTX-class GPU with DirectX Raytracing 1.1 and the D3D12 graphics API.</recomp-color> It has no effect otherwise.",
                ray_tracing_options,
                RayTracingOption::Off
            );

            config.add_enum_option(
                graphics::options::rt_quality_option,
                "Ray Tracing Quality",
                "<recomp-color primary>Performance</recomp-color> traces at half resolution with fewer rays. <recomp-color primary>Balanced</recomp-color> traces at half resolution. <recomp-color primary>Quality</recomp-color> traces at full resolution and costs the most.",
                ray_tracing_quality_options,
                RayTracingQuality::Balanced
            );
            {
                config.add_option_hidden_dependency(
                    graphics::options::rt_quality_option,
                    graphics::options::rt_option,
                    RayTracingOption::Off
                );
            }

            return config;
        }
    }
} // namespace recompui

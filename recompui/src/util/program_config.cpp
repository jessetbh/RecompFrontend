#include "program_config.h"
#include "common/rt64_math.h"

static std::string program_name = "";
static std::u8string program_id = u8"";
static recompui::programconfig::FrameInterpolation frame_interpolation{};

namespace recompui {
    namespace programconfig {
        void set_program_name(const std::string& name) {
            program_name = name;
        }

        void set_program_id(const std::u8string& id) {
            program_id = id;
        }

        const std::string &get_program_name() {
            if (program_name.empty()) {
                throw std::runtime_error("Program name has not been set. Run recompui::programconfig::set_program_name(...) first.");
            }
            return program_name;
        }

        void set_frame_interpolation(const FrameInterpolation &interpolation) {
            frame_interpolation = interpolation;
            RT64::wcwSetInterpolationProfile(interpolation.hookless_objects, interpolation.prime_generated_frames,
                interpolation.skip_clear_only_targets, interpolation.even_pacing);
        }

        const FrameInterpolation &get_frame_interpolation() {
            return frame_interpolation;
        }

        const std::u8string &get_program_id() {
            if (program_id.empty()) {
                throw std::runtime_error("Program ID has not been set. Run recompui::programconfig::set_program_id(...) first.");
            }
            return program_id;
        }
    }
}

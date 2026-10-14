// [wcw] Unit tests for the graphics presets (src/config/graphics_presets). Built and run as part of every recompui
// build (RECOMPUI_WCW_TESTS); a failure fails the build.

#include "graphics_presets.h"

#include "wcw_test.h"

using ultramodern::renderer::Antialiasing;
using ultramodern::renderer::AspectRatio;
using ultramodern::renderer::RefreshRate;
using namespace recompui::config;
using namespace recompui::config::presets;

template <typename T>
static int val(T value) {
    return static_cast<int>(value);
}

// A game with frame interpolation (smooth rate 60) on a 144 Hz display with an MSAA 4x GPU.
static Limits interpolating() {
    Limits limits;
    limits.smooth_rate = 60;
    limits.display_rate = 144;
    limits.max_msaa = Antialiasing::MSAA4X;
    return limits;
}

static Settings preset(Preset p, const Limits &limits = interpolating()) {
    return settings_for(p, limits).value();
}

TEST_CASE("Authentic is the game as it shipped on a composite TV") {
    const Settings s = preset(Preset::Authentic);
    CHECK_EQ(val(s.aspect_ratio), val(AspectRatio::Original));
    CHECK_EQ(val(s.framerate), val(RefreshRate::Original));
    CHECK_EQ(val(s.msaa), val(Antialiasing::None));
    CHECK_EQ(val(s.crt), val(CrtFilterOption::On));
    CHECK_EQ(val(s.crt_signal), val(CrtSignalOption::Composite));
    CHECK_EQ(val(s.rt), val(RayTracingOption::Off));
    CHECK_EQ(val(s.hdr), val(HdrOption::Off));
}

TEST_CASE("Enhanced is Expand, 60 fps (Smooth), MSAA 4x, RT Balanced, no CRT or HDR") {
    const Settings s = preset(Preset::Enhanced);
    CHECK_EQ(val(s.aspect_ratio), val(AspectRatio::Expand));
    CHECK_EQ(val(s.framerate), val(RefreshRate::Manual));
    CHECK_EQ(val(s.msaa), val(Antialiasing::MSAA4X));
    CHECK_EQ(val(s.crt), val(CrtFilterOption::Off));
    CHECK_EQ(val(s.rt), val(RayTracingOption::On));
    CHECK_EQ(val(s.rt_quality), val(RayTracingQuality::Balanced));
    CHECK_EQ(val(s.hdr), val(HdrOption::Off));
}

TEST_CASE("Max is 120 fps (Smooth2x), MSAA 4x, RT Ultra, HDR Auto") {
    const Settings s = preset(Preset::Max);
    CHECK_EQ(val(s.aspect_ratio), val(AspectRatio::Expand));
    CHECK_EQ(val(s.framerate), val(RefreshRate::Display));
    CHECK_EQ(val(s.msaa), val(Antialiasing::MSAA4X));
    CHECK_EQ(val(s.crt), val(CrtFilterOption::Off));
    CHECK_EQ(val(s.rt), val(RayTracingOption::On));
    CHECK_EQ(val(s.rt_quality), val(RayTracingQuality::Ultra));
    CHECK_EQ(val(s.hdr), val(HdrOption::On));
    CHECK_EQ(val(s.hdr_picture), val(HdrPictureOption::Auto));
}

TEST_CASE("Max runs 120 only where the display allows it, otherwise the smooth rate") {
    Limits limits = interpolating();
    limits.display_rate = 60;
    CHECK_EQ(val(preset(Preset::Max, limits).framerate), val(RefreshRate::Manual));
    limits.display_rate = 100;
    CHECK_EQ(val(preset(Preset::Max, limits).framerate), val(RefreshRate::Manual));
    limits.display_rate = 119; // 119.88 Hz
    CHECK_EQ(val(preset(Preset::Max, limits).framerate), val(RefreshRate::Display));
    limits.display_rate = 120;
    CHECK_EQ(val(preset(Preset::Max, limits).framerate), val(RefreshRate::Display));
    limits.display_rate = 0; // not known yet
    CHECK_EQ(val(preset(Preset::Max, limits).framerate), val(RefreshRate::Display));
}

TEST_CASE("a game with a locked framerate stays at Original in every preset") {
    Limits limits = interpolating();
    limits.smooth_rate = 0;
    for (Preset p : { Preset::Authentic, Preset::Enhanced, Preset::Max }) {
        CHECK_EQ(val(preset(p, limits).framerate), val(RefreshRate::Original));
    }
}

TEST_CASE("MSAA is capped at what the GPU offers") {
    Limits limits = interpolating();
    limits.max_msaa = Antialiasing::MSAA2X;
    CHECK_EQ(val(preset(Preset::Enhanced, limits).msaa), val(Antialiasing::MSAA2X));
    CHECK_EQ(val(preset(Preset::Max, limits).msaa), val(Antialiasing::MSAA2X));
    limits.max_msaa = Antialiasing::None;
    CHECK_EQ(val(preset(Preset::Max, limits).msaa), val(Antialiasing::None));
}

TEST_CASE("Custom sets nothing") {
    CHECK(!settings_for(Preset::Custom, interpolating()).has_value());
    CHECK(!matches(Preset::Custom, preset(Preset::Enhanced), interpolating()));
}

TEST_CASE("each preset's settings match that preset") {
    for (Preset p : { Preset::Authentic, Preset::Enhanced, Preset::Max }) {
        CHECK_EQ(val(matching_preset(preset(p), interpolating())), val(p));
    }

    Limits locked = interpolating();
    locked.smooth_rate = 0;
    locked.max_msaa = Antialiasing::None;
    for (Preset p : { Preset::Authentic, Preset::Enhanced, Preset::Max }) {
        CHECK_EQ(val(matching_preset(preset(p, locked), locked)), val(p));
    }
}

TEST_CASE("changing any one option a preset sets makes it Custom") {
    for (Preset p : { Preset::Authentic, Preset::Enhanced, Preset::Max }) {
        const Settings base = preset(p);
        Settings s;

        s = base;
        s.aspect_ratio = (base.aspect_ratio == AspectRatio::Expand) ? AspectRatio::Original : AspectRatio::Expand;
        CHECK_EQ(val(matching_preset(s, interpolating())), val(Preset::Custom));

        s = base;
        s.framerate = (base.framerate == RefreshRate::Original) ? RefreshRate::Manual : RefreshRate::Original;
        CHECK_EQ(val(matching_preset(s, interpolating())), val(Preset::Custom));

        s = base;
        s.msaa = (base.msaa == Antialiasing::MSAA2X) ? Antialiasing::None : Antialiasing::MSAA2X;
        CHECK_EQ(val(matching_preset(s, interpolating())), val(Preset::Custom));

        s = base;
        s.crt = (base.crt == CrtFilterOption::On) ? CrtFilterOption::Off : CrtFilterOption::On;
        CHECK_EQ(val(matching_preset(s, interpolating())), val(Preset::Custom));

        s = base;
        s.rt = (base.rt == RayTracingOption::On) ? RayTracingOption::Off : RayTracingOption::On;
        CHECK_EQ(val(matching_preset(s, interpolating())), val(Preset::Custom));

        s = base;
        s.hdr = (base.hdr == HdrOption::On) ? HdrOption::Off : HdrOption::On;
        CHECK_EQ(val(matching_preset(s, interpolating())), val(Preset::Custom));
    }

    Settings s = preset(Preset::Authentic);
    s.crt_signal = CrtSignalOption::RGB;
    CHECK_EQ(val(matching_preset(s, interpolating())), val(Preset::Custom));

    s = preset(Preset::Enhanced);
    s.rt_quality = RayTracingQuality::Quality;
    CHECK_EQ(val(matching_preset(s, interpolating())), val(Preset::Custom));

    s = preset(Preset::Max);
    s.hdr_picture = HdrPictureOption::Brighter;
    CHECK_EQ(val(matching_preset(s, interpolating())), val(Preset::Custom));
}

TEST_CASE("hidden sub-options don't count: CRT signal with CRT off, RT quality with RT off, HDR picture with HDR off") {
    Settings s = preset(Preset::Enhanced);
    s.crt_signal = CrtSignalOption::RF;
    s.hdr_picture = HdrPictureOption::CrtDetail;
    CHECK_EQ(val(matching_preset(s, interpolating())), val(Preset::Enhanced));

    s = preset(Preset::Authentic);
    s.rt_quality = RayTracingQuality::Ultra;
    CHECK_EQ(val(matching_preset(s, interpolating())), val(Preset::Authentic));
}

TEST_CASE("options the presets leave alone don't count") {
    // Window mode, resolution, HUD placement, CRT colors and beam simulator aren't part of Settings at all; a framerate
    // the game locks doesn't count either.
    Limits locked = interpolating();
    locked.smooth_rate = 0;
    Settings s = preset(Preset::Enhanced, locked);
    s.framerate = RefreshRate::Manual;
    CHECK_EQ(val(matching_preset(s, locked)), val(Preset::Enhanced));
}

TEST_CASE("MSAA above what the GPU offers counts as the GPU's highest level") {
    // A saved MSAA 4x on a 2x-only GPU runs as 2x, which is what Enhanced asks for there.
    Limits limits = interpolating();
    limits.max_msaa = Antialiasing::MSAA2X;
    Settings s = preset(Preset::Enhanced, interpolating());
    CHECK_EQ(val(s.msaa), val(Antialiasing::MSAA4X));
    CHECK_EQ(val(matching_preset(s, limits)), val(Preset::Enhanced));
}

TEST_CASE("Max on a 60 Hz display matches at the smooth rate, not at 120") {
    Limits limits = interpolating();
    limits.display_rate = 60;
    Settings s = preset(Preset::Max, limits);
    CHECK_EQ(val(matching_preset(s, limits)), val(Preset::Max));
    s.framerate = RefreshRate::Display;
    CHECK_EQ(val(matching_preset(s, limits)), val(Preset::Custom));
}

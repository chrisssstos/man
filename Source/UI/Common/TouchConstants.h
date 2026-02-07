#pragma once

namespace TouchUI
{
    // Touch targets
    static constexpr int kMinTouchTarget   = 56;
    static constexpr int kIdealTouchTarget = 64;
    static constexpr int kLargeTouchTarget = 80;

    // Gaps
    static constexpr int kMinGap      = 8;
    static constexpr int kStandardGap = 10;
    static constexpr int kLargeGap    = 12;

    // Tiles
    static constexpr int kTileSize = 88;
    static constexpr int kTileGap  = 10;

    // Transport
    static constexpr int kTransportHeight  = 64;
    static constexpr int kTransportButtonW = 64;

    // Timeline
    static constexpr int kTrackRowHeight   = 72;
    static constexpr int kTrackHeaderWidth = 56;
    static constexpr int kClipResizeHandle = 20;

    // Navigation
    static constexpr int kModeBarHeight  = 48;
    static constexpr int kSwipeThreshold = 60;

    // Typography
    static constexpr float kFontTiny   = 12.0f;
    static constexpr float kFontSmall  = 14.0f;
    static constexpr float kFontNormal = 16.0f;
    static constexpr float kFontLarge  = 18.0f;
    static constexpr float kFontTitle  = 20.0f;

    // Corner radii
    static constexpr float kCornerRadius   = 10.0f;
    static constexpr float kCornerRadiusLg = 14.0f;

    // Colors — enhanced dark gaming theme
    static constexpr juce::uint32 kBgDeep    = 0xff080818;
    static constexpr juce::uint32 kBgPanel   = 0xff0e0e24;
    static constexpr juce::uint32 kBgCard    = 0xff1a1a3a;
    static constexpr juce::uint32 kAccentPink  = 0xffe94560;
    static constexpr juce::uint32 kAccentGreen = 0xff2ecc71;
    static constexpr juce::uint32 kAccentCyan  = 0xff00ddff;
}

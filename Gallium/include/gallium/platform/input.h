#ifndef GALLIUM__PLATFORM__INPUT_H
#define GALLIUM__PLATFORM__INPUT_H
#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <glm/vec2.hpp>

namespace ga::platform
{
    class Platform;

    enum class EKey
        : uint16_t
    {
        Space        = 32,
        Apostrophe   = 39,
        Comma        = 44,
        Minus        = 45,
        Period       = 46,
        Slash        = 47,
        Num0 = 48, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
        Semicolon    = 59,
        Equal        = 61,
        A = 65, B, C, D, E, F, G, H, I, J, K, L, M,
        N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        LeftBracket  = 91,
        Backslash    = 92,
        RightBracket = 93,
        GraveAccent  = 96,

        Escape       = 256,
        Enter        = 257,
        Tab          = 258,
        Backspace    = 259,
        Insert       = 260,
        Delete       = 261,
        Right        = 262,
        Left         = 263,
        Down         = 264,
        Up           = 265,
        PageUp       = 266,
        PageDown     = 267,
        Home         = 268,
        End          = 269,
        CapsLock     = 280,
        ScrollLock   = 281,
        NumLock      = 282,
        PrintScreen  = 283,
        Pause        = 284,
        F1  = 290, F2,  F3,  F4,  F5,  F6,
        F7  = 296, F8,  F9,  F10, F11, F12,

        LeftShift    = 340,
        LeftControl  = 341,
        LeftAlt      = 342,
        LeftSuper    = 343,
        RightShift   = 344,
        RightControl = 345,
        RightAlt     = 346,
        RightSuper   = 347,

        Numpad0 = 320, Numpad1, Numpad2, Numpad3, Numpad4,
        Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,
        NumpadDecimal  = 330,
        NumpadDivide   = 331,
        NumpadMultiply = 332,
        NumpadSubtract = 333,
        NumpadAdd      = 334,
        NumpadEnter    = 335,
        NumpadEqual    = 336,

        Count = 512
    };

    enum class EMouseButton
        : uint8_t
    {
        Left   = 0,
        Right  = 1,
        Middle = 2,
        Count  = 8
    };

    enum class EMouseMode
        : uint8_t
    {
        Free,     // cursor visible, absolute position
        Captured, // cursor hidden, relative delta only
    };

    class Input
    {
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;

    public:
        Input(const ga::platform::Platform& platform);
       ~Input() = default;

        Input(const Input&)            = delete;
        Input& operator=(const Input&) = delete;

        Impl& GetImpl() const;

        bool IsPressed (EKey key) const;  // true only the frame it went down
        bool IsHeld    (EKey key) const;  // true while held
        bool IsReleased(EKey key) const;  // true only the frame it went up
        bool IsPressed (EMouseButton button) const;
        bool IsHeld    (EMouseButton button) const;
        bool IsReleased(EMouseButton button) const;

        using MultiKey    = std::vector<EKey>;
        using VariableKey = std::variant<
            EKey,
            MultiKey
        >;
        using KeyCombo = std::vector<VariableKey>;

        KeyCombo CreateKeyCombo(const std::string& combo);
        bool IsKeyComboPressed(const KeyCombo& combo);
        bool IsKeyComboPressed(const std::string& combo);

        glm::vec2 MousePosition() const;  // absolute, pixels
        glm::vec2 MouseDelta()    const;  // frame delta
        glm::vec2 ScrollDelta()   const;  // wheel delta, frame

        void       SetMouseMode(EMouseMode mode);
        EMouseMode MouseMode()  const;

        const std::vector<uint32_t> InputChars() const;

        void OnPressed(EKey key, std::function<void()> callback);
        void OnHeld(EKey key, std::function<void()> callback);
        void OnReleased(EKey key, std::function<void()> callback);
        void OnPressed(EMouseButton btn, std::function<void()> callback);
        void OnReleased(EMouseButton btn, std::function<void()> callback);
    };
}

#endif /* GALLIUM__PLATFORM__INPUT_H */

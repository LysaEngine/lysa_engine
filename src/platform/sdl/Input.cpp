/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_keycode.h>
module lysa.input;

import lysa.log;

namespace lysa {

    std::map<SDL_JoystickID, SDL_Gamepad*> Input::_activeGamepads;


    void Input::initInput() {}

    void Input::closeInput() {}

    uint32 Input::getConnectedJoypads() {
        return static_cast<uint32_t>(_activeGamepads.size());
    }

    bool Input::isGamepad(const uint32 index) {
        return SDL_IsGamepad(index);
    }

    // GamepadButton Input::mapSdlButtonToInternal(const uint8_t sdlButton) {
    //     switch (sdlButton) {
    //     case SDL_GAMEPAD_BUTTON_SOUTH:          return GamepadButton::A;
    //     case SDL_GAMEPAD_BUTTON_EAST:           return GamepadButton::B;
    //     case SDL_GAMEPAD_BUTTON_WEST:           return GamepadButton::X;
    //     case SDL_GAMEPAD_BUTTON_NORTH:          return GamepadButton::Y;
    //     case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:  return GamepadButton::LB;
    //     case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: return GamepadButton::RB;
    //     case SDL_GAMEPAD_BUTTON_BACK:           return GamepadButton::SHARE;
    //     case SDL_GAMEPAD_BUTTON_START:          return GamepadButton::START;
    //     case SDL_GAMEPAD_BUTTON_LEFT_STICK:     return GamepadButton::LT;
    //     case SDL_GAMEPAD_BUTTON_RIGHT_STICK:    return GamepadButton::RB;
    //     case SDL_GAMEPAD_BUTTON_DPAD_UP:        return GamepadButton::DPAD_UP;
    //     case SDL_GAMEPAD_BUTTON_DPAD_DOWN:      return GamepadButton::DPAD_DOWN;
    //     case SDL_GAMEPAD_BUTTON_DPAD_LEFT:      return GamepadButton::DPAD_LEFT;
    //     case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:     return GamepadButton::DPAD_RIGHT;
    //     case SDL_GAMEPAD_BUTTON_GUIDE:
    //     default:                                return GamepadButton::START;
    //     }
    // }
    //
    SDL_GamepadButton mapInternalToSdlButton(const GamepadButton button) {
        switch (button) {
        case GamepadButton::A:     return SDL_GAMEPAD_BUTTON_SOUTH;
        case GamepadButton::B:     return SDL_GAMEPAD_BUTTON_EAST;
        case GamepadButton::X:     return SDL_GAMEPAD_BUTTON_WEST;
        case GamepadButton::Y:     return SDL_GAMEPAD_BUTTON_NORTH;
        case GamepadButton::LB:    return SDL_GAMEPAD_BUTTON_LEFT_SHOULDER;
        case GamepadButton::RB:    return SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER;
        case GamepadButton::BACK:  return SDL_GAMEPAD_BUTTON_BACK;
        case GamepadButton::START: return SDL_GAMEPAD_BUTTON_START;
        case GamepadButton::LT:    return SDL_GAMEPAD_BUTTON_LEFT_STICK;
        case GamepadButton::RT:    return SDL_GAMEPAD_BUTTON_RIGHT_STICK;
        case GamepadButton::DPAD_UP:    return SDL_GAMEPAD_BUTTON_DPAD_UP;
        case GamepadButton::DPAD_DOWN:  return SDL_GAMEPAD_BUTTON_DPAD_DOWN;
        case GamepadButton::DPAD_LEFT:  return SDL_GAMEPAD_BUTTON_DPAD_LEFT;
        case GamepadButton::DPAD_RIGHT: return SDL_GAMEPAD_BUTTON_DPAD_RIGHT;
        default: return SDL_GAMEPAD_BUTTON_INVALID;
        }
    }

    bool Input::isGamepadButtonPressed(const uint32_t joypadId, const GamepadButton gamepadButton) {
        auto* gamepad = SDL_GetGamepadFromID(joypadId);
        if (!gamepad) {
            return false;
        }
        const auto sdlBtn = mapInternalToSdlButton(gamepadButton);
        if (sdlBtn == SDL_GAMEPAD_BUTTON_INVALID) {
            return false;
        }
        return SDL_GetGamepadButton(gamepad, sdlBtn);
    }

    float2 Input::getGamepadVector(const uint32_t joypadId, const GamepadAxisJoystick axisJoystick) {
        auto* gamepad = SDL_GetGamepadFromID(joypadId);
        if (!gamepad) return FLOAT2ZERO;

        SDL_GamepadAxis axisX, axisY;
        if (axisJoystick == GamepadAxisJoystick::LEFT) {
            axisX = SDL_GAMEPAD_AXIS_LEFTX;
            axisY = SDL_GAMEPAD_AXIS_LEFTY;
        } else {
            axisX = SDL_GAMEPAD_AXIS_RIGHTX;
            axisY = SDL_GAMEPAD_AXIS_RIGHTY;
        }
        const auto rawX = SDL_GetGamepadAxis(gamepad, axisX) / 32767.0f;
        const auto rawY = SDL_GetGamepadAxis(gamepad, axisY) / 32767.0f;
        constexpr auto deadzone = 0.1f;
        auto vector = float2 {
            applyDeadzone(rawX, deadzone),
            applyDeadzone(-rawY, deadzone)
        };
        const float l = length(vector);
        return (l > 1.0f) ? vector / l : vector;
    }

    std::string Input::getJoypadName(const uint32 index) {
        const auto* name = SDL_GetGamepadNameForID(index);
        if (name) {
            return std::string(name);
        }
        return "Unknown Controller";
    }

    int _getKeyboardModifiers() {
        int modifiers = 0;
        const auto sdlMods = SDL_GetModState();
        if (sdlMods & SDL_KMOD_SHIFT)   modifiers |= static_cast<int>(KeyModifier::SHIFT);
        if (sdlMods & SDL_KMOD_CTRL)    modifiers |= static_cast<int>(KeyModifier::CONTROL);
        if (sdlMods & SDL_KMOD_ALT)     modifiers |= static_cast<int>(KeyModifier::ALT);
        if (sdlMods & SDL_KMOD_GUI)  modifiers |= static_cast<int>(KeyModifier::SUPER);
        return modifiers;
    }

    uint32_t _getMouseButtonState() {
        uint32_t state = 0;
        float x, y;
        const auto sdlButtons = SDL_GetMouseState(&x, &y);
        if (sdlButtons & SDL_BUTTON_MASK(SDL_BUTTON_LEFT))   state |= static_cast<uint32_t>(MouseButton::LEFT);
        if (sdlButtons & SDL_BUTTON_MASK(SDL_BUTTON_MIDDLE)) state |= static_cast<uint32_t>(MouseButton::MIDDLE);
        if (sdlButtons & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT))  state |= static_cast<uint32_t>(MouseButton::RIGHT);
        return state;
    }

    MouseButton _mapSdlButton(const uint8_t sdlButton) {
        switch (sdlButton) {
        case SDL_BUTTON_LEFT:   return MouseButton::LEFT;
        case SDL_BUTTON_RIGHT:  return MouseButton::RIGHT;
        case SDL_BUTTON_MIDDLE: return MouseButton::MIDDLE;
        default:                return MouseButton::LEFT;
        }
    }

    void Input::_processEvent(const SDL_Event& event) {
        switch (event.type) {
            case SDL_EVENT_TEXT_INPUT:{
                const auto window = RenderingWindow::_getFromId(event.text.windowID);
                if (window) {
                    auto textInputEvent = InputEventTextInput{
                        event.text.text
                    };
                    window->_input({InputEventType::TEXT_INPUT, textInputEvent});
                }
                break;
            }
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP: {
                const auto window = RenderingWindow::_getFromId(event.key.windowID);
                if (window) {
                    const auto isDown = (event.type == SDL_EVENT_KEY_DOWN);
                    const auto key = osKeyToKey(static_cast<OsKey>(event.key.scancode));
                    keyJustPressedStates[key] = isDown && !keyPressedStates[key];
                    keyPressedStates[key] = isDown;
                    keyJustReleasedStates[key] = !isDown;
                    auto keyEvent = InputEventKey{
                        key, isDown, 1, _getKeyboardModifiers()
                    };
                    window->_input({InputEventType::KEY, keyEvent});
                }
                break;
            }
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:{
                const auto window = RenderingWindow::_getFromId(event.key.windowID);
                if (window) {
                    const auto isDown = (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN);
                    const auto btn = _mapSdlButton(event.button.button);
                    mouseButtonPressedStates[btn] = isDown;
                    auto btnEvent = InputEventMouseButton {
                        float2(event.button.x, window->getRenderTarget().getHeight() - event.button.y),
                        _getMouseButtonState(),
                        _getKeyboardModifiers(),
                        btn,
                        isDown
                    };
                    window->_input({InputEventType::MOUSE_BUTTON, btnEvent});
                }
                break;
            }
        case SDL_EVENT_MOUSE_MOTION: {
                const auto window = RenderingWindow::_getFromId(event.key.windowID);
                if (window) {
                    const float xPos = event.motion.x;
                    const float yPos = window->getRenderTarget().getHeight() - event.motion.y;
                    if (!RenderingWindow::_resettingMousePosition) {
                        auto motionEvent = InputEventMouseMotion {
                            float2(xPos, yPos),
                            _getMouseButtonState(),
                            _getKeyboardModifiers(),
                            float2(event.motion.xrel, -event.motion.yrel)
                        };
                        window->_input({InputEventType::MOUSE_MOTION, motionEvent});
                    } else {
                        RenderingWindow::_resettingMousePosition = false;
                    }
                }
                break;
        }
        case SDL_EVENT_MOUSE_WHEEL:{
                const auto window = RenderingWindow::_getFromId(event.key.windowID);
                if (window) {
                    auto wheelEvent = InputEventMouseButton {
                        float2(0, 0),
                        _getMouseButtonState(),
                        _getKeyboardModifiers(),
                        MouseButton::WHEEL,
                        event.wheel.y < 0 // Direction
                    };
                    window->_input({InputEventType::MOUSE_BUTTON, wheelEvent});
                }
                break;
            }
        case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
                // // SDL_GamepadAxis goes from -32768 to 32767
                // // Normalize to -1.0f to 1.0f
                // float value = event.gaxis.value / 32767.0f;
                // if (value < -1.0f) value = -1.0f;
                //
                // auto axisEvent = InputEventGam{
                //     event.gaxis.axis, // e.g., SDL_GAMEPAD_AXIS_LEFTX
                //     value
                // };
                // window->_input({InputEventType::GAMEPAD_AXIS, axisEvent});
                break;
            }
            case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
            case SDL_EVENT_GAMEPAD_BUTTON_UP: {
                // const auto window = RenderingWindow::_getFromId(event.key.windowID);
                // if (window) {
                //     bool isDown = (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN);
                //     // SDL_GamepadButton mapping (e.g., SDL_GAMEPAD_BUTTON_SOUTH is 'A' on Xbox)
                //     auto buttonEvent = InputEventGamepadButton {
                //         event.gbutton.button,
                //         isDown
                //     };
                //     window->_input({InputEventType::GAMEPAD_BUTTON, buttonEvent});
                // }
                break;
            }
            case SDL_EVENT_GAMEPAD_ADDED: {
                auto* gamepad = SDL_OpenGamepad(event.gdevice.which);
                if (gamepad) {
                    const auto id = SDL_GetGamepadID(gamepad);
                    _activeGamepads[id] = gamepad;
                }
                break;
            }
            case SDL_EVENT_GAMEPAD_REMOVED: {
                const auto id = event.gdevice.which;
                if (_activeGamepads.contains(id)) {
                    SDL_CloseGamepad(_activeGamepads[id]);
                    _activeGamepads.erase(id);
                }
                break;
            }
            default:
                break;
        }
    }

}

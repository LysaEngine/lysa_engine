/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
module lysa.input;


namespace lysa {


    void Input::initInput() {

    }

    void Input::closeInput() {

    }

    uint32 Input::getConnectedJoypads() {
        uint32 count = 0;
        return count;
    }

    bool Input::isGamepad(const uint32 index) {
        return false;
    }

    void Input::generateGamepadButtonEvent(RenderingWindow& window, const GamepadButton button, const bool pressed) {
    }

    bool Input::isGamepadButtonPressed(const uint32 index, const GamepadButton gamepadButton) {
        return false;
    }

    float2 Input::getGamepadVector(const uint32 index, const GamepadAxisJoystick axisJoystick) {
        return FLOAT2ZERO;
    }

    std::string Input::getJoypadName(const uint32 index) {
        return "??";
    }

    float2 Input::getKeyboardVector(const Key keyNegX, const Key keyPosX, const Key keyNegY, const Key keyPosY) {
        return {};
    }

    std::string Input::keyToChar(const Key key) {
        return "";
    }

}

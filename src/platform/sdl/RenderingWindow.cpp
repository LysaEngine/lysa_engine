/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
#include <SDL3/SDL.h>
module lysa.resources.rendering_window;

import lysa.exception;
import lysa.input;
import lysa.log;
import lysa.utils;

namespace lysa {

    void RenderingWindow::show() const {
    }

    void RenderingWindow::close() const {
    }

    vireo::PlatformWindowHandle RenderingWindow::openPlatformWindow(const RenderingWindowConfiguration& config) {

        if (config.mode == RenderingWindowMode::FULLSCREEN) {

        }
        // if (w == 0 || h == 0 || config.mode != RenderingWindowMode::WINDOWED) {
            // if (config.mode == RenderingWindowMode::WINDOWED_FULLSCREEN || config.mode == RenderingWindowMode::FULLSCREEN) {
            // } else {
            // }

        // }
        if (config.mode == RenderingWindowMode::WINDOWED || config.mode == RenderingWindowMode::WINDOWED_MAXIMIZED) {

        }

        // if (windowId == nullptr) { throw Exception("Error creating window", std::to_string(SDL_GetError())); }

        rect = {

        };
        return nullptr;
    }

    void RenderingWindow::setTitle(const std::string& title) const {
    }

    float2 RenderingWindow::getMousePosition() const {
        return { };
    }

    void RenderingWindow::setMousePosition(const float2& position) const {
    }

    void RenderingWindow::setMouseCursor(const MouseCursor cursor) const {
    }

    void RenderingWindow::resetMousePosition() const {
    }

    bool RenderingWindow::isMouseHidden() const {
        return false;
    }

    void RenderingWindow::setMouseMode(const MouseMode mode) const {
        // while (PeekMessageW(&msg, handle, 0, 0, PM_REMOVE)) {
            // TranslateMessage(&msg);
            // DispatchMessageW(&msg);
        // }
        switch (mode) {
        case MouseMode::VISIBLE:
            resetMousePosition();
            break;
        case MouseMode::HIDDEN:
            break;
        case MouseMode::VISIBLE_CAPTURED: {
            resetMousePosition();
            break;
        }
        case MouseMode::HIDDEN_CAPTURED: {
            resetMousePosition();
            break;
        }
        default:
            throw Exception("Unknown mouse mode");
        }
    }

}
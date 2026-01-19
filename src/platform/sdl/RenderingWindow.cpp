/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
module lysa.resources.rendering_window;

import lysa.exception;
import lysa.input;
import lysa.log;
import lysa.utils;

namespace lysa {

    void RenderingWindow::show() const {
        if (!SDL_ShowWindow(handle)) {
            throw Exception("Error SDL_ShowWindow : ", SDL_GetError());
        }
    }

    void RenderingWindow::close() const {
        if (SDL_GetWindowFromID(windowId)) {
            SDL_DestroyWindow(handle);
        }
    }

    vireo::PlatformWindowHandle RenderingWindow::openPlatformWindow(const RenderingWindowConfiguration& config) {
        SDL_WindowFlags flags = SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN;
        int w = config.width;
        int h = config.height;
        if (w == 0 || h == 0 || config.mode == RenderingWindowMode::WINDOWED_FULLSCREEN || config.mode == RenderingWindowMode::FULLSCREEN) {
            flags |= SDL_WINDOW_FULLSCREEN;
            const auto mode = SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay());
            if (!mode) {
                throw Exception("Error SDL_GetCurrentDisplayMode : ", SDL_GetError());
            }
            w = mode->w;
            h = mode->h;
        }
        if (config.mode == RenderingWindowMode::WINDOWED) {
            flags |= SDL_WINDOW_RESIZABLE;
        }
        else if (config.mode == RenderingWindowMode::WINDOWED_MAXIMIZED) {
            flags |= SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED;
            SDL_Rect usable;
            if (!SDL_GetDisplayUsableBounds(SDL_GetPrimaryDisplay(), &usable)) {
                throw Exception("Error SDL_GetDisplayUsableBounds : ", SDL_GetError());
            }
            w  = usable.w;
            h = usable.h;
        }

        SDL_Window* window{nullptr};
        if (!(window = SDL_CreateWindow(config.title.c_str(), w, h, flags))) {
            throw vireo::Exception("Error creating SDL window : ", SDL_GetError());
        }

        if (!SDL_GetWindowSizeInPixels(window, &w, &h)) {
            throw Exception("Error SDL_GetWindowSizeInPixels : ", SDL_GetError());
        }
        int x, y;
        if (!SDL_GetWindowPosition(window, &x, &y)) {
            throw Exception("Error SDL_GetWindowPosition : ", SDL_GetError());
        }
        rect = Rect{
            static_cast<float>(x),
            static_cast<float>(y),
            static_cast<float>(w),
            static_cast<float>(h)
        };
        windowId = SDL_GetWindowID(window);
        SDL_SetPointerProperty(SDL_GetWindowProperties(window), USER_DATA_PROPERTY_NAME, this);
        return window;
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
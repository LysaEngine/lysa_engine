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

    std::map<MouseCursor, SDL_Cursor *> RenderingWindow::_mouseCursors;

    void RenderingWindow::show() const {
        if (!SDL_ShowWindow(handle)) {
            throw Exception("Error SDL_ShowWindow : ", SDL_GetError());
        }
    }

    void RenderingWindow::close() const {
        if (SDL_GetWindowFromID(_windowId)) {
            SDL_DestroyWindow(handle);
        }
    }

    bool RenderingWindow::isMinimized() const {
        const auto flags = SDL_GetWindowFlags(handle);
        return (flags & SDL_WINDOW_MINIMIZED) != 0;
    }

    RenderingWindow* RenderingWindow::_getFromId(const SDL_WindowID windowId) {
        const auto window = SDL_GetWindowFromID(windowId);
        if (window) {
            return static_cast<RenderingWindow*>(SDL_GetPointerProperty(
                SDL_GetWindowProperties(window),
                _USER_DATA_PROPERTY_NAME,
                nullptr));
        }
        return nullptr;
    }

    vireo::PlatformWindowHandle RenderingWindow::openPlatformWindow(const RenderingWindowConfiguration& config) {
        if (_mouseCursors.empty()) {
            _mouseCursors[MouseCursor::ARROW]    = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
            _mouseCursors[MouseCursor::WAIT]     = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
            _mouseCursors[MouseCursor::RESIZE_H] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_EW_RESIZE);
            _mouseCursors[MouseCursor::RESIZE_V] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NS_RESIZE);
        }

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
        SDL_StartTextInput(window);
        _windowId = SDL_GetWindowID(window);
        SDL_SetPointerProperty(SDL_GetWindowProperties(window), _USER_DATA_PROPERTY_NAME, this);
        return window;
    }

    void RenderingWindow::_processEvent(const SDL_Event& event) {
        switch (event.type) {
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:{
                auto* renderingWindow = RenderingWindow::_getFromId(event.window.windowID);
                if (renderingWindow) {
                    renderingWindow->_closing();
                }
                break;
            }
            case SDL_EVENT_WINDOW_MINIMIZED:{
                auto* renderingWindow = RenderingWindow::_getFromId(event.window.windowID);
                if (renderingWindow) {
                    renderingWindow->setPause(renderingWindow->isMinimized());
                }
            }
            case SDL_EVENT_WINDOW_RESIZED:
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
                auto* renderingWindow = RenderingWindow::_getFromId(event.window.windowID);
                if (renderingWindow) {
                    if (renderingWindow->isMinimized()) {
                        renderingWindow->setPause(true);
                    } else {
                        renderingWindow->setPause(false);
                        int x, y, w, h;
                        SDL_GetWindowPosition(renderingWindow->getHandle(), &x, &y);
                        SDL_GetWindowSize(renderingWindow->getHandle(), &w, &h);
                        renderingWindow->_resized({
                            static_cast<float>(x),
                            static_cast<float>(y),
                        static_cast<float>(w),
                        static_cast<float>(h)});
                    }
                }
                break;
            }
        default:
            break;
        }
    }

    void RenderingWindow::setTitle(const std::string& title) const {
        SDL_SetWindowTitle(handle, title.c_str());
    }

    float2 RenderingWindow::getMousePosition() const {
        float x, y;
        SDL_GetMouseState(&x, &y);
        return { x, y };
    }

    void RenderingWindow::setMousePosition(const float2& position) const {
        SDL_WarpMouseInWindow(handle, position.x, position.y);
    }

    void RenderingWindow::setMouseCursor(const MouseCursor cursor) const {
        SDL_SetCursor(_mouseCursors[cursor]);
    }

    void RenderingWindow::resetMousePosition() const {
        _resettingMousePosition = true;
        int w, h;
        SDL_GetWindowSize(handle, &w, &h);
        SDL_WarpMouseInWindow(handle, static_cast<float>(w) / 2.0f, static_cast<float>(h) / 2.0f);
    }

    bool RenderingWindow::isMouseHidden() const {
        return !SDL_CursorVisible();;
    }

    void RenderingWindow::setMouseMode(const MouseMode mode) const {
        SDL_SetWindowRelativeMouseMode(handle, false);
        SDL_SetWindowMouseGrab(handle, false);
        SDL_ShowCursor();
        switch (mode) {
        case MouseMode::VISIBLE:
            resetMousePosition();
            break;
        case MouseMode::HIDDEN:
            SDL_HideCursor();
            break;
        case MouseMode::VISIBLE_CAPTURED:
            SDL_SetWindowMouseGrab(handle, true);
            resetMousePosition();
            break;
        case MouseMode::HIDDEN_CAPTURED:
            SDL_SetWindowRelativeMouseMode(handle, true);
            break;
        default:
            throw Exception("Unknown mouse mode");
        }
    }

}
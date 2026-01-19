/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
#include <SDL3/SDL.h>
module lysa;

import lysa.event;

namespace lysa {

    void Lysa::processPlatformEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_EVENT_QUIT:
                ctx.exit = true;
                break;
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:{
                const auto window = SDL_GetWindowFromID(event.window.windowID);
                if (window) {
                    auto* renderingWindow = static_cast<RenderingWindow*>(SDL_GetPointerProperty(
                        SDL_GetWindowProperties(window),
                        RenderingWindow::USER_DATA_PROPERTY_NAME,
                        nullptr));
                    if (renderingWindow) {
                        renderingWindow->_closing();
                    }
                }
                break;
            }
            case SDL_EVENT_WINDOW_RESIZED:
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                break;
            case SDL_EVENT_KEY_DOWN:
                break;
            case SDL_EVENT_KEY_UP:
                break;
            default:
                break;
            }
        }
    }
}


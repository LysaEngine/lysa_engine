/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.resources.rendering_window;

import lysa.context;

namespace lysa {

    bool RenderingWindow::_resettingMousePosition{false};

    RenderingWindow::RenderingWindow(const RenderingWindowConfiguration& config) :
        handle(openPlatformWindow(config)),
        renderTarget(config.renderTargetConfiguration, handle) {
        ctx().events.push({ .type = static_cast<event_type>(RenderingWindowEvent::READY), .id = id});
    }

    RenderingWindow::~RenderingWindow() {
        if (!closed) {
            renderTarget.setPause(false);
            _closing();
            close();
        }
    }

    void RenderingWindow::_input(const InputEvent& inputEvent) const {
        if (closed || renderTarget.isPaused()) { return; }
        ctx().events.push({static_cast<event_type>(RenderingWindowEvent::INPUT), inputEvent, id});
    }

    void RenderingWindow::_closing() {
        if (closed || renderTarget.isPaused()) { return; }
        renderTarget.setPause(true);
        closed = true;
        ctx().events.push({.type = static_cast<event_type>(RenderingWindowEvent::CLOSING), .id = id});
    }

    void RenderingWindow::_resized(const Rect& rect) {
        if (closed || renderTarget.isPaused()) { return; }
        this->rect = rect;
        renderTarget.resize();
    }

}
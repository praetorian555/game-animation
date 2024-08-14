#include "opal/container/ref.h"
#include "opal/container/scope-ptr.h"
#include "opal/container/string.h"
#include "opal/time.h"

#include "rndr/render-api.h"
#include "rndr/renderer-base.h"
#include "rndr/rndr.h"
#include "rndr/window.h"

#include "imgui-wrapper.h"
#include "types.h"

void Run();

struct AppState
{
    f32 delta_seconds = 1 / 60.0f;
};

class UIRenderer final : public Rndr::RendererBase
{
public:
    UIRenderer(const Opal::StringUtf8& name, const Rndr::RendererBaseDesc& desc, Rndr::Window* window, AppState* app_state)
        : Rndr::RendererBase(name, desc), m_window(window), m_app_state(app_state)
    {
        ImGuiWrapper::Init(*window, desc.graphics_context);
    }

    ~UIRenderer() override { ImGuiWrapper::Destroy(); }

    bool Render() override
    {
        ImGuiWrapper::StartFrame();

        ImGui::Begin("Game Animation");
        ImGui::Text("Frame Rate: %.1f FPS", 1.0f / m_app_state->delta_seconds);
        ImGui::End();

        ImGuiWrapper::EndFrame();
        return true;
    }

private:
    Opal::Ref<Rndr::Window> m_window;
    Opal::Ref<AppState> m_app_state;
};

int main()
{
    Rndr::Init(Rndr::RndrDesc{.enable_input_system = true});
    Run();
    Rndr::Destroy();
    return 0;
}

void Run()
{
    Rndr::Window window(Rndr::WindowDesc{.width = 800, .height = 600, .name = "Game Animation"});
    Rndr::GraphicsContext graphics_context(Rndr::GraphicsContextDesc{.window_handle = window.GetNativeWindowHandle()});
    RNDR_ASSERT(graphics_context.IsValid());
    Rndr::SwapChain swap_chain(graphics_context,
                               Rndr::SwapChainDesc{.width = window.GetWidth(), .height = window.GetHeight(), .enable_vsync = false});
    RNDR_ASSERT(swap_chain.IsValid());

    AppState app_state;

    Opal::ScopePtr<Rndr::RendererBase> clear_renderer = Opal::MakeDefaultScoped<Rndr::ClearRenderer>(
        u8"Clear Renderer", Rndr::RendererBaseDesc{.graphics_context = Opal::Ref(&graphics_context), .swap_chain = Opal::Ref(&swap_chain)},
        Rndr::Colors::k_black);
    Opal::ScopePtr<Rndr::RendererBase> ui_renderer = Opal::MakeDefaultScoped<UIRenderer>(
        u8"UI Renderer", Rndr::RendererBaseDesc{.graphics_context = Opal::Ref(&graphics_context), .swap_chain = Opal::Ref(&swap_chain)},
        &window, &app_state);
    Opal::ScopePtr<Rndr::RendererBase> present_renderer = Opal::MakeDefaultScoped<Rndr::PresentRenderer>(
        u8"Present Renderer",
        Rndr::RendererBaseDesc{.graphics_context = Opal::Ref(&graphics_context), .swap_chain = Opal::Ref(&swap_chain)});

    Rndr::RendererManager renderer_manager;
    renderer_manager.AddRenderer(clear_renderer.Get());
    renderer_manager.AddRenderer(ui_renderer.Get());
    renderer_manager.AddRenderer(present_renderer.Get());

    f32 delta_seconds = 1 / 60.0f;
    while (!window.IsClosed())
    {
        const f64 start_time = Opal::GetSeconds();

        window.ProcessEvents();

        renderer_manager.Render();

        const f64 end_time = Opal::GetSeconds();
        delta_seconds = static_cast<f32>(end_time - start_time);
        app_state.delta_seconds = delta_seconds;
    }

    graphics_context.Destroy();
    window.Destroy();
}
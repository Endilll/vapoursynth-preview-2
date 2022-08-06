// #include <cairo.h>
// #include <elements.hpp>
#include <VapourSynth4.h>
#include <VSScript4.h>

#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>
#include <SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

// using namespace cycfi::elements;

int main(int argc, char** argv) try
{
    int default_frame_number{0};
    const char* script_file_path{[&] {
        return argc >= 2 ? argv[1] : throw std::invalid_argument{"VSPreview error: Script file not specified"};
    }()};
    int frame_number{[&] {
        if (argc < 3) {
            return default_frame_number;
        }
        int number{std::stoi(argv[2])};
        return number >= 0 ? number : throw std::invalid_argument{"VSPreview error: frame number must be a non-negative"};
    }()};

    // Init VS api and video data
    const VSAPI* vs_api{[] {
        const VSAPI* ptr{getVapourSynthAPI(VAPOURSYNTH_API_VERSION)};
        return ptr ? ptr : throw std::runtime_error{"VSPreview error: VSApi loading error"};
    }()};
    const VSSCRIPTAPI* vs_script_api{[] {
        const VSSCRIPTAPI* ptr{getVSScriptAPI(VSSCRIPT_API_VERSION)};
        return ptr ? ptr : throw std::runtime_error{"VSPreview error: VSScriptApi loading error"};
    }()};

    VSScript* vs_script{[&] {
        VSScript* ptr{vs_script_api->createScript(nullptr)};
        return ptr ? ptr : throw std::runtime_error{"VSPreview error: Script creating error"};
    }()};
    vs_script_api->evaluateFile(vs_script, script_file_path);
    VSNode* video_node{[&] {
        VSNode* ptr{vs_script_api->getOutputNode(vs_script, 0)};
        return ptr ? ptr : throw std::runtime_error{vs_script_api->getError(vs_script)};
    }()};

    // Safe max frame number
    int video_frames_quantity{vs_api->getVideoInfo(video_node)->numFrames};
    if (video_frames_quantity <= frame_number) {
        throw std::invalid_argument{
            "VSPreview error: frame number out of range. Selected frame: "
            + std::to_string(frame_number)
            + ". Max frame number: "
            + std::to_string(video_frames_quantity-1)
        };
    }

    // Convert video format to RGB24
    VSCore* vs_core{vs_script_api->getCore(vs_script)};
    VSPlugin* pResizePlugin{vs_api->getPluginByID("com.vapoursynth.resize", vs_core)};
    VSMap* pArgumentMap{vs_api->createMap()};
    vs_api->mapSetNode(pArgumentMap, "clip", video_node, 0);
    vs_api->mapSetInt(pArgumentMap, "format", pfRGB24, 0);
    vs_api->mapSetData(pArgumentMap, "matrix_in_s", "709", 3, dtUtf8, 0);
    VSMap* pResultMap{vs_api->invoke(pResizePlugin, "Bicubic", pArgumentMap)};
    VSNode* pPreviewNode{vs_api->mapGetNode(pResultMap, "clip", 0, nullptr)};
    vs_api->freeMap(pArgumentMap);
    vs_api->freeMap(pResultMap);
    const VSFrame* video_frame{vs_api->getFrame(frame_number, pPreviewNode, nullptr, 0)};


    // Prepare video frame for elements
    int frame_width{vs_api->getFrameWidth(video_frame, 0)};
    int frame_height{vs_api->getFrameHeight(video_frame, 0)};
    const uint8_t* frame_raw_plane_red{vs_api->getReadPtr(video_frame, 0)};
    const uint8_t* frame_raw_plane_green{vs_api->getReadPtr(video_frame, 1)};
    const uint8_t* frame_raw_plane_blue{vs_api->getReadPtr(video_frame, 2)};

    unsigned char* surface_data = new unsigned char[frame_width*frame_height*4];
    for (int i{0}; i < frame_width*frame_height*4; i += 4) {
        surface_data[i] = frame_raw_plane_red[i/4];
        surface_data[i+1] = frame_raw_plane_green[i/4];
        surface_data[i+2] = frame_raw_plane_blue[i/4];
        surface_data[i+3] = 255; // Alpha
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    #if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame_width, frame_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface_data);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::Begin("OpenGL Texture Text");
            ImGui::Text("pointer = %p", &image_texture);
            ImGui::Text("size = %d x %d", frame_width, frame_height);
            ImGui::Image((void*)(intptr_t)image_texture, ImVec2(frame_width, frame_height));
            ImGui::End();
        }

        ImGui::ShowMetricsWindow();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        // if (show_demo_window)
        //     ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        // {
        //     static float f = 0.0f;
        //     static int counter = 0;

        //     ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        //     ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        //     ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
        //     ImGui::Checkbox("Another Window", &show_another_window);

        //     ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        //     ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        //     if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
        //         counter++;
        //     ImGui::SameLine();
        //     ImGui::Text("counter = %d", counter);

        //     ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        //     ImGui::End();
        // }

        // 3. Show another simple window.
        // if (show_another_window)
        // {
        //     ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        //     ImGui::Text("Hello from another window!");
        //     if (ImGui::Button("Close Me"))
        //         show_another_window = false;
        //     ImGui::End();
        // }

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
        // // Init elements pixmap
        // pixmap_ptr pixmap_obj{std::make_shared<pixmap>(point{static_cast<float>(frame_width), static_cast<float>(frame_height)})};
        // pixmap_context pixmap_context{*pixmap_obj};
        // cairo_t* cairo_context{pixmap_context.context()};

        // cairo_surface_t* cairo_surface{cairo_get_target(cairo_context)};
        // unsigned char* surface_data{cairo_image_surface_get_data(cairo_surface)};

        // // copy frame to pixmap context
        // cairo_surface_flush(cairo_surface);
        // for (int i{0}; i < frame_width*frame_height*4; i += 4) {
        //     surface_data[i] = frame_raw_plane_blue[i/4];
        //     surface_data[i+1] = frame_raw_plane_green[i/4];
        //     surface_data[i+2] = frame_raw_plane_red[i/4];
        //     surface_data[i+3] = 255; // Alpha
        // }
        // cairo_surface_mark_dirty(cairo_surface);


        // // Init elements and create image from pixmap
        // app _app(argc, argv, "VapourSynth Preview 2", "com.endill.vapoursynth-preview-2");
        // window _win(_app.name());
        // _win.on_close = [&_app]() { _app.stop(); };

        // view view_(_win);

        // view_.content(
        //     image{pixmap_obj}
        // );

        // _app.run();

    vs_script_api->freeScript(vs_script);

    return 0;
} catch (std::exception &exception) {
    std::cout << exception.what() << std::endl;
    return 0;
}

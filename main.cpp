#include <elements.hpp>
#include <VSScript4.h>

#include <string>

using namespace cycfi::elements;

int main(int argc, char** argv)
{
    try {
        // Init variables
        int default_frame_number{0};
        const char* script_file_path{[&] {
            return argc >= 2 ? argv[1] : throw std::runtime_error{"VSPreview error: Script file not specified"};
        }()};
        int frame_number{[&] {
            if (argc < 3) {
                return default_frame_number;
            }
            int number{std::stoi(argv[2])};
            return number >= 0 ? number : default_frame_number;
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
        int video_frame_quantity{vs_api->getVideoInfo(video_node)->numFrames};
        if (frame_number >= video_frame_quantity) {
            frame_number = video_frame_quantity;
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


        // Prepare frame for elements
        int frame_width{vs_api->getFrameWidth(video_frame, 0)};
        int frame_height{vs_api->getFrameHeight(video_frame, 0)};
        const uint8_t* frame_raw_plane_red{vs_api->getReadPtr(video_frame, 0)};
        const uint8_t* frame_raw_plane_green{vs_api->getReadPtr(video_frame, 1)};
        const uint8_t* frame_raw_plane_blue{vs_api->getReadPtr(video_frame, 2)};


        // Init elements pixmap
        pixmap pixmap{{static_cast<float>(frame_width), static_cast<float>(frame_height)}};
        pixmap_context pixmap_context{pixmap};
        cairo_t* cairo_context{pixmap_context.context()};

        cairo_surface_t* cairo_surface{cairo_get_target(cairo_context)};
        unsigned char* surface_data{cairo_image_surface_get_data(cairo_surface)};

        // copy frame to pixmap context
        cairo_surface_flush(cairo_surface);
        for (int i{0}; i < frame_width * frame_height * 4; i += 4) {
            surface_data[i] = frame_raw_plane_blue[i / 4];
            surface_data[i + 1] = frame_raw_plane_green[i / 4];
            surface_data[i + 2] = frame_raw_plane_red[i / 4];
            surface_data[i + 3] = 255; // Alpha
        }
        cairo_surface_mark_dirty(cairo_surface);


        // Init elements and create image from pixmap
        app _app(argc, argv, "VapourSynth Preview 2", "com.endill.vapoursynth-preview-2");
        window _win(_app.name());
        _win.on_close = [&_app]() { _app.stop(); };

        view view_(_win);

        view_.content(
            image{pixmap_ptr{&pixmap}}
        );

        _app.run();

        // Free VS script
        vs_script_api->freeScript(vs_script);
    } catch (std::exception &exception) {
        std::cout << exception.what() << std::endl;
    }

    return 0;
}

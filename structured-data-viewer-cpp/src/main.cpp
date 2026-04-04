#include "app.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>
#include <cstdio>
#include <string>
#include <vector>

// Global app pointer for GLFW callbacks
static App* g_app = nullptr;
static FILE* g_log = nullptr;

static void dbg(const char* msg) {
    if (g_log) { fprintf(g_log, "%s\n", msg); fflush(g_log); }
}

static void glfwErrorCallback(int error, const char* description) {
    if (g_log) { fprintf(g_log, "GLFW Error %d: %s\n", error, description); fflush(g_log); }
}

static void glfwDropCallback(GLFWwindow* /*window*/, int count, const char** paths) {
    if (g_app && count > 0) {
        std::vector<std::string> files;
        files.reserve(count);
        for (int i = 0; i < count; i++)
            files.emplace_back(paths[i]);
        g_app->onFileDrop(files);
    }
}

static void glfwCloseCallback(GLFWwindow* window) {
    if (g_app) {
        glfwSetWindowShouldClose(window, GLFW_FALSE); // Cancel the close
        g_app->requestClose();
    }
}

int main(int /*argc*/, char** /*argv*/) {
    g_log = fopen("C:\\Users\\hanaf\\Downloads\\sdv_debug.txt", "w");
    dbg("1: main entered");

    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) {
        dbg("FATAL: glfwInit failed");
        if (g_log) fclose(g_log);
        return 1;
    }
    dbg("2: glfwInit OK");

    // OpenGL 3.0 compat
    const char* glslVersion = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);  // HiDPI support
    dbg("3: window hints set");

    GLFWwindow* window = glfwCreateWindow(1280, 800, "Structured Data Viewer", nullptr, nullptr);
    if (!window) {
        dbg("FATAL: glfwCreateWindow failed");
        glfwTerminate();
        if (g_log) fclose(g_log);
        return 1;
    }
    dbg("4: window created");

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetDropCallback(window, glfwDropCallback);
    glfwSetWindowCloseCallback(window, glfwCloseCallback);
    dbg("5: context made current");

    // Print OpenGL version
    const char* glVer = (const char*)glGetString(GL_VERSION);
    const char* glRend = (const char*)glGetString(GL_RENDERER);
    if (g_log) {
        fprintf(g_log, "   GL Version:  %s\n", glVer ? glVer : "(null)");
        fprintf(g_log, "   GL Renderer: %s\n", glRend ? glRend : "(null)");
        fflush(g_log);
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    dbg("6: ImGui context created");

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // DPI scaling - use a moderate font size, let GLFW handle window scaling
    float xscale = 1.0f, yscale = 1.0f;
    glfwGetWindowContentScale(window, &xscale, &yscale);
    float dpiScale = xscale;
    if (g_log) { fprintf(g_log, "   DPI scale: %.2f\n", dpiScale); fflush(g_log); }

    // Load font with Japanese glyph support
    {
        float fontSize = 16.0f * dpiScale;
        bool fontLoaded = false;
        const char* jpFonts[] = {
            "C:\\Windows\\Fonts\\meiryo.ttc",
            "C:\\Windows\\Fonts\\msgothic.ttc",
            "C:\\Windows\\Fonts\\YuGothM.ttc",
            nullptr
        };
        for (const char** p = jpFonts; *p; ++p) {
            FILE* f = fopen(*p, "rb");
            if (f) {
                fclose(f);
                io.Fonts->AddFontFromFileTTF(*p, fontSize, nullptr, io.Fonts->GetGlyphRangesJapanese());
                fontLoaded = true;
                if (g_log) { fprintf(g_log, "   Font: %s\n", *p); fflush(g_log); }
                break;
            }
        }
        if (!fontLoaded) io.Fonts->AddFontDefault();
    }

    // Scale ImGui style for HiDPI
    ImGui::GetStyle().ScaleAllSizes(dpiScale);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glslVersion);
    dbg("7: ImGui backends initialized");

    App app;
    g_app = &app;
    dbg("8: App created, entering loop");

    int frameCount = 0;
    while (!glfwWindowShouldClose(window) && !app.shouldClose()) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        app.render();

        ImGui::Render();
        int displayW, displayH;
        glfwGetFramebufferSize(window, &displayW, &displayH);
        glViewport(0, 0, displayW, displayH);
        glClearColor(0.067f, 0.067f, 0.106f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

        frameCount++;
        if (frameCount == 1) dbg("9: first frame rendered");
        if (frameCount == 10) dbg("10: 10 frames rendered");
    }

    dbg("11: loop exited");
    g_app = nullptr;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    dbg("12: cleanup done");
    if (g_log) fclose(g_log);
    return 0;
}

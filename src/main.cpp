#include <iostream>
#include <cmath>
#include <vector>
#include <string>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "cpp-httplib/httplib.h"
#include "json/single_include/nlohmann/json.hpp"

#include "Graph.h"

// ImGui + GLFW + OpenGL headers
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

using namespace std;
using json = nlohmann::json;

using json = nlohmann::json;
using namespace std;

// Variables for frontend
static std::vector<std::string> log_messages;
static bool showVisualization = false;

// Draw the shortest‑path visualization using ImGui drawing APIs
void drawShortestPath(const Graph& graph) {
    showVisualization = true;
    ImGui::Text("Shortest Path:");
    ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();

    auto* drawList = ImGui::GetWindowDrawList();
    ImVec2 origin = ImGui::GetCursorScreenPos();

    const float radius  = 30.0f;
    const float spacing = 160.0f;
    ImVec2 pos = { origin.x + radius * 2, origin.y };

    size_t count = min(graph.get_size(), size_t(10));
    for (size_t i = 0; i < count; ++i) {
        // Alternate circle colors
        ImU32 circleColor = (i % 2 == 0)
            ? IM_COL32(225, 140,  65, 255)
            : IM_COL32(100, 200, 255, 255);

        // Draw node
        drawList->AddCircleFilled(pos, radius, circleColor);
        string label = to_string(i);
        drawList->AddText({ pos.x - 8, pos.y + radius + 10 },
                          IM_COL32(255,255,255,255),
                          label.c_str());

        // Draw edge to next node
        if (i + 1 < count) {
            ImVec2 next = { pos.x + spacing, pos.y };
            drawList->AddLine({ pos.x + radius, pos.y },
                              next,
                              IM_COL32(200,200,200,255),
                              2.0f);
            pos = next;
        }
    }

    ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
    ImGuiTableFlags flags = ImGuiTableFlags_HighlightHoveredColumn
                          | ImGuiTableFlags_SizingFixedFit
                          | ImGuiTableFlags_RowBg
                          | ImGuiTableFlags_BordersH
                          | ImGuiTableFlags_BordersV;

    ImGui::BeginChild("TableScrolling");
    if (ImGui::BeginTable("PaperInfo", 3, flags)) {
        // Header row
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0); ImGui::Text("Node");
        ImGui::TableSetColumnIndex(1); ImGui::Text("Paper Title");
        ImGui::TableSetColumnIndex(2); ImGui::Text("Paper ID");

        // Data rows
        for (size_t row = 0; row < graph.get_size(); ++row) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%zu", row);
            ImGui::TableSetColumnIndex(1);
            ImGui::TextWrapped("%s", graph.nodes().at(row).get_title().c_str());
            ImGui::TableSetColumnIndex(2);
            ImGui::TextWrapped("%s", graph.nodes().at(row).get_id().c_str());
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();
}


int main() {
    // Set up HTTP client for OpenAlex
    httplib::SSLClient cli("api.openalex.org", 443);
    cli.enable_server_certificate_verification(true);

    Graph paperGraph;

    // Initialize GLFW
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Knowledge Path", nullptr, nullptr);
    if (!window) return -1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.FontGlobalScale = 2.0f;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    bool showWindow = true;
    static char paper1[128] = "";
    static char paper2[128] = "";

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (showWindow) {
            // Centered window
            ImGui::SetNextWindowSize({1900, 1080}, ImGuiCond_Always);
            ImGui::Begin("Shortest Path", &showWindow, ImGuiWindowFlags_NoResize);

            if (ImGui::BeginTabBar("MainTabs")) {
                if (ImGui::BeginTabItem("Home")) {
                    ImGui::SetWindowFontScale(1.8f);
                    ImGui::Text("Find Shortest Path Between Papers");
                    ImGui::SetWindowFontScale(1.0f);
                    ImGui::Spacing();

                    ImGui::InputText("Paper 1", paper1, IM_ARRAYSIZE(paper1));
                    ImGui::InputText("Paper 2", paper2, IM_ARRAYSIZE(paper2));
                    ImGui::Spacing();

                    if (ImGui::Button("Find Shortest Path")) {
                        log_messages.emplace_back("Finding path...");
                        paperGraph.graph_by_befs(cli, paper1, paper2);
                        if (paperGraph.get_size() != 0) {
                            log_messages.emplace_back(
                                "Path size: " + to_string(paperGraph.get_size())
                            );
                        } else {
                            log_messages.emplace_back("No connection found.");
                        }
                    }

                    ImGui::Separator();
                    ImGui::Text("Output Log:");
                    ImGui::BeginChild("OutputLog", {0,200}, true);
                    for (auto& msg : log_messages)
                        ImGui::Text("%s", msg.c_str());
                    ImGui::EndChild();

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Visualization")) {
                    drawShortestPath(paperGraph);
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }

            ImGui::End();
        }

        // Render
        ImGui::Render();
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
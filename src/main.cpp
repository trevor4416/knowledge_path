#include <iostream>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "cpp-httplib/httplib.h"
#include "json/single_include/nlohmann/json.hpp"
#include "openalex.h"
#include "Graph.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <cmath>

using json = nlohmann::json;
using namespace std;

// variables for frontend
static std::vector<std::string> log_messages;
static bool showVisualization = false;

// function to draw the shortest path
void drawShortestPath(Graph graph) {
    // starts showing the visualization, displays text and spaces the graph
    showVisualization = true;
    ImGui::Text("Shortest Path:");
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    // some imgui drawing setup
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 startPos = ImGui::GetCursorScreenPos();

    // define circle radius, spacing, and position
    float radius = 30.0f;
    float spacing = 160.0f;
    ImVec2 currentPos = startPos;
    currentPos.x = startPos.x + radius * 2;
    // for every node in graph
    for (size_t i = 0; i < graph.get_size(); ++i) {
        // defines circle color
        ImU32 circleColor = IM_COL32(225, 140, 65, 255);
        if (i % 2 != 0)
            circleColor = IM_COL32(100, 200, 255, 255);

        ImU32 textColor = IM_COL32(255, 255, 255, 255);

        // adds circle and associated text to the draw list
        draw_list->AddCircleFilled(currentPos, radius, circleColor);
        const char* nodeNum = std::to_string(i).c_str();
        draw_list->AddText(ImVec2(currentPos.x - 7.5, currentPos.y + radius + 10), textColor, nodeNum);

        // adds the line that connects the nodes
        if (i < graph.get_size() - 1) {
            ImVec2 nextPos = ImVec2(currentPos.x + spacing, currentPos.y);
            currentPos.x = currentPos.x + radius;;
            draw_list->AddLine(currentPos, nextPos, IM_COL32(200, 200, 200, 255), 2.0f);
            currentPos = nextPos;
        }
        // cuts off the visualization at 10 nodes
        if (i == 10)
            break;
    }
    // spacing for table
    for (int i = 0; i < 7; i++)
        ImGui::Spacing();

    // flags for table formatting
    ImGuiTableFlags flags = ImGuiTableFlags_HighlightHoveredColumn | ImGuiTableFlags_SizingFixedFit |
        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersH | ImGuiTableFlags_BordersV;

    // defines a scrolling behavior
    ImGui::BeginChild("Scrolling");
    // begin table
    if (ImGui::BeginTable("Paper Information", 3, flags)) {
        // iterate through every cell in table
        for (size_t row = 0; row < graph.get_size() + 1; row++) {
            ImGui::TableNextRow();
            for (int column = 0; column < 3; column++) {
                ImGui::TableSetColumnIndex(column);
                // if we aren't in the header row, add text from node
                if (row != 0) {
                    switch (column) {
                        case 0:
                            ImGui::Text("%d", static_cast<int>(row - 1));
                            break;
                        case 1:
                            ImGui::Text("%s", graph.nodes().at(row - 1).get_title().c_str());
                            break;
                        case 2:
                            ImGui::Text("%s", graph.nodes().at(row - 1).get_id().c_str());
                            break;
                        /*
                        case 3:
                            ImGui::Text("N/A");
                            break;
                        */
                    }
                }
                // if we are in the header row, print header
                else {
                    switch (column) {
                        case 0:
                            ImGui::Text("Node:");
                            break;
                        case 1:
                            ImGui::Text("Paper Title:");
                            break;
                        case 2:
                            ImGui::Text("Paper ID:");
                            break;
                        /*
                        case 3:
                            ImGui::Text("DOI:");
                            break;
                        */
                    }
                }
            }
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();
}


int main() {
    httplib::SSLClient cli("api.openalex.org",443);
    cli.enable_server_certificate_verification(true);

    Graph paperGraph;

    // init GLFW (backend for imGui)
    if (!glfwInit())
        return -1;

    // sets glfw and openGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // starts window
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Project 3", nullptr, nullptr);
    if (!window)
        return -1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // sets up imGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.FontGlobalScale = 2.0f; // scales imGui stuff
    ImGuiStyle& style = ImGui::GetStyle();
    // some custom spacing stuff
    style.WindowPadding = ImVec2(20, 20);
    style.ItemSpacing = ImVec2(15, 15);
    style.FramePadding = ImVec2(10, 6);
    ImGui::StyleColorsDark(); // dark mode :O

    // connects imgui to glfw and openGL
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // vars to know if window is open and to take in user input
    bool showWindow = true;
    static char paper1[128] = "";
    static char paper2[128] = "";

    // the main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // generate new frames
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // imgui window open
        if (showWindow) {
            // define window parameters
            const ImGuiIO& io = ImGui::GetIO();
            ImVec2 windowSize(1900, 1080);
            ImVec2 windowPos((io.DisplaySize.x - windowSize.x) * 0.5f,
                             (io.DisplaySize.y - windowSize.y) * 0.5f);
            ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
            ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

            // give the window a title and begin it
            ImGui::Begin("Shortest Path", &showWindow, ImGuiWindowFlags_NoResize);

            // define tabs (one for input and one for visualization
            if (ImGui::BeginTabBar("MainTabs")) {
                if (ImGui::BeginTabItem("Home")) {
                    // increase font scale and draw text, then set scale back down
                    ImGui::SetWindowFontScale(1.8f);
                    ImGui::Text("Find Shortest Path Between Papers");
                    ImGui::SetWindowFontScale(1.0f); // Reset font scale for body

                    // more text, add spacing
                    ImGui::TextWrapped("Discover how two papers are connected through their citations");
                    ImGui::Spacing();

                    // boxes for input, writes to paper1 and paper2, add spacing
                    ImGui::InputText("Paper 1", paper1, IM_ARRAYSIZE(paper1));
                    ImGui::InputText("Paper 2", paper2, IM_ARRAYSIZE(paper2));
                    ImGui::Spacing();

                    // button to find the shortest path and update the visualization
                    if (ImGui::Button("Find Shortest Path")) {
                        // find the shortest path by befs
                        log_messages.emplace_back(std::string("Finding path..."));
                        paperGraph.graph_by_befs(cli, paper1, paper2);

                        // if there are nodes in the graph, output the size
                        if (paperGraph.get_size() != 0) {
                            log_messages.emplace_back(
                                std::string("Shortest path found with size: ") + std::to_string(paperGraph.get_size())
                            );
                        }
                        // if no nodes, output no connection
                        else {
                            log_messages.emplace_back(std::string("No connection found."));
                        }
                    }

                    // this is all for the output log
                    ImGui::Separator();
                    ImGui::Text("Output:");
                    ImGui::BeginChild("OutputLog", ImVec2(0, 200), true);
                    for (const auto& msg : log_messages) {
                        ImGui::TextWrapped("%s", msg.c_str());
                    }
                    ImGui::EndChild();
                    ImGui::EndTabItem();
                }
                // tab for visualization. when open, continuously draw the shortest path
                if (ImGui::BeginTabItem("Visualization")) {
                    drawShortestPath(paperGraph);
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }

            ImGui::End();
        }

        // rendering stuff. draws everything every frame
        ImGui::Render();
        int displayW, displayH;
        glfwGetFramebufferSize(window, &displayW, &displayH);
        glViewport(0, 0, displayW, displayH);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // functions to clean up after shutdown
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
#include "EditorPanels.hpp"
#include <imgui.h>
namespace aether::editor {
void drawEditorDockspace(){ImGui::DockSpaceOverViewport(0,ImGui::GetMainViewport());}
void drawSceneHierarchy(){
 if(!ImGui::Begin("Scene Hierarchy")){ImGui::End();return;}
 ImGui::TextDisabled("WORLD");ImGui::Separator();
 ImGui::BulletText("Procedural Planet");ImGui::BulletText("Key Light");ImGui::BulletText("Showcase Camera");
 ImGui::End();
}
void drawInspector(){
 if(!ImGui::Begin("Inspector")){ImGui::End();return;}
 ImGui::TextDisabled("SELECTED OBJECT");ImGui::Separator();
 ImGui::TextUnformatted("Transform");float position[3]={0.0f,0.0f,0.0f};
 ImGui::DragFloat3("Position",position,0.01f);ImGui::End();
}
void drawAssetBrowser(){
 if(!ImGui::Begin("Asset Browser")){ImGui::End();return;}
 ImGui::TextDisabled("PROJECT / ASSETS");ImGui::Separator();
 ImGui::BulletText("Scenes");ImGui::BulletText("Textures");ImGui::BulletText("Models");
 ImGui::BulletText("Audio");ImGui::BulletText("Extensions");ImGui::End();
}
void drawConsole(){
 if(!ImGui::Begin("Console")){ImGui::End();return;}
 ImGui::TextUnformatted("[AETHER] Editor workspace initialized");ImGui::End();
}
}

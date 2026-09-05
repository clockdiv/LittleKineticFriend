#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "M33.h"

#define MAX_JOINTS 256

typedef enum 
{
    EDITOR_IDLE,
    EDITOR_PRESS_JOINT,
    EDITOR_PRESS_CONNECTION,
    EDITOR_PRESS_CANVAS,
    EDITOR_SELECTION,
    EDITOR_DRAG_JOINT,
} EditorState;

typedef struct
{
    M33 transformation;
    float radius;
    bool isFixed;
    struct Joint* parent;
} Joint;

typedef struct
{
    EditorState state;
    Joint* hoveredJoint;
    Joint* hoveredConnectionToParent;
    Joint* activeJoint;
    Joint* selectedJoints[MAX_JOINTS];
    Vector2 mouseDownPosition;
} Editor;

typedef struct
{
    Joint joints[MAX_JOINTS];
    int jointCount;
} Scene;


void SetJointPosition(Joint* joint, Vector2 position)
{
    M33_SetPosition(joint->transformation, position);
}

void InitJoint(Joint* joint)
{
    joint->radius = 20;
    joint->parent = nullptr;
    joint->isFixed = false;
    M33_SetIdentity(joint->transformation);
    SetJointPosition(joint, GetMousePosition());
}

void AddJoint(Scene* scene)
{
    if(scene->jointCount < MAX_JOINTS - 1)
    {
        InitJoint(&scene->joints[scene->jointCount]);
        scene->jointCount++;
    }
    printf("Added Joint #%d %p to Scene.\n", scene->jointCount, &scene->joints[scene->jointCount-1]);
}

int CountSelectedJoints(const Editor* editor)
{
    int count = 0;
    while(editor->selectedJoints[count] != nullptr)
    {
        count++;
    }
    return count;    
}

void AddJointToSelection(Editor* editor, Joint* joint)
{
    int lastJointIdx = CountSelectedJoints(editor);
    // editor->selectedJointsIdx++;
    if (lastJointIdx < MAX_JOINTS) 
    {
        editor->selectedJoints[lastJointIdx] = joint;
        printf("Added Joint #%d %p to Selection.\n", lastJointIdx, joint);
    }
}

bool IsJointSelected(const Editor* editor, const Joint* joint)
{
    for (int i = 0; i < MAX_JOINTS; ++i)
    {
        if(editor->selectedJoints[i] == joint) 
            return true;
    }
    return false;
}

void ClearJointsSelection(Editor* editor)
{
    for (int i = 0; i < MAX_JOINTS; ++i)
    {
        editor->selectedJoints[i] = nullptr;
    }
    printf("Cleared Joints Selection.\n");
}



bool GetConnectionLineToParent(const Joint* joint, Vector2* start, Vector2* end)
{
    if (joint->parent != nullptr)
    {
        *start = M33_GetPosition(joint->transformation);
        Joint* parent = (Joint*)(joint->parent);
        *end = M33_GetPosition(parent->transformation);
        return true;
    }
    return false;
}

const Joint* GetHoveredJoint(const Scene* scene)
{
    for (int i = 0; i < scene->jointCount; ++i)
    {
        Vector2 jointPos = M33_GetPosition(scene->joints[i].transformation);
        if (CheckCollisionPointCircle(GetMousePosition(), jointPos, scene->joints[i].radius))
            return &scene->joints[i];
    }
    return nullptr;
}

const Joint* GetHoveredConnection(const Scene* scene)
{
    for (int i = 0; i < scene->jointCount; ++i)
    {
        Vector2 conStart, conEnd;
        if (GetConnectionLineToParent(&scene->joints[i], &conStart, &conEnd))
        {
            if (CheckCollisionPointLine(GetMousePosition(), conStart, conEnd, 10))
            {
                return &scene->joints[i];
            }
        }
    }
    return nullptr;
}

void SetJointHierachieFromSelection(Editor* editor, Scene* scene)
{
    printf("Set Joint Hierachie: ");
    int i = 0;
    while(editor->selectedJoints[i + 1] != nullptr)
    {
        editor->selectedJoints[i]->parent = (struct Joint*)(editor->selectedJoints[i + 1]);
        printf("[%d] %p now has parent %p\n", i, editor->selectedJoints[i], editor->selectedJoints[i]->parent);
        i++;
    }
    printf("\n");
}

void PrintJointSelection(Editor* editor)
{
    printf("Print Joint Selection: ");
    int i = 0;
    while(editor->selectedJoints[i] != nullptr)
    {
        printf("[%d] %p\t", i, editor->selectedJoints[i]);
        i++;
    }
    printf("\n");
}


Rectangle RectFromPoints (Vector2 start, Vector2 end)
{
    Rectangle r;
    bool selDirRight = end.x > start.x;
    bool selDirDown  = end.y > start.y;
    r.x      = selDirRight ? start.x : end.x;
    r.width  = selDirRight ? end.x - start.x : start.x - end.x;
    r.y      = selDirDown ? start.y : end.y;
    r.height = selDirDown ? end.y - start.y : start.y - end.y;
    return r;
}

Rectangle GetSelectionRect(const Editor *editor)
{
    Rectangle rect = RectFromPoints(editor->mouseDownPosition, GetMousePosition());
    return rect;
}

void UpdateEditor(Scene* scene, Editor* editor)
{
    switch (editor->state)
    {
        case EDITOR_IDLE:
            editor->hoveredJoint = (Joint*)GetHoveredJoint(scene);
            editor->hoveredConnectionToParent = (Joint*)GetHoveredConnection(scene);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                editor->mouseDownPosition = GetMousePosition();
                if(editor->hoveredJoint != nullptr) 
                {
                    editor->activeJoint = editor->hoveredJoint;
                    editor->state = EDITOR_PRESS_JOINT;
                } 
                else if (editor->hoveredConnectionToParent != nullptr)
                {
                    editor->state = EDITOR_PRESS_CONNECTION;
                }
                else 
                {
                    editor->state = EDITOR_PRESS_CANVAS;
                }
            }
            break;

        case EDITOR_PRESS_CANVAS:
            if (Vector2Distance(editor->mouseDownPosition, GetMousePosition()) > 3.0f)
            {
                editor->state = EDITOR_SELECTION;
            } 
            else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            {
                if(CountSelectedJoints(editor) == 0)
                {
                    AddJoint(scene);
                }
                else 
                {
                    ClearJointsSelection(editor);
                }
                editor->state = EDITOR_IDLE;
            } 
            break;

        case EDITOR_PRESS_JOINT:
            if (Vector2Distance(editor->mouseDownPosition, GetMousePosition()) > 3.0f)
            {
                printf("Dragging Joint Start.\n");
                editor->state = EDITOR_DRAG_JOINT;
            } 
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            {
                if(!IsKeyDown(KEY_LEFT_SHIFT))
                {
                    printf("Toggling Selection of Active Joint.\n");
                    bool selected = IsJointSelected(editor, editor->activeJoint);
                    ClearJointsSelection(editor);
                    if(!selected) 
                        AddJointToSelection(editor, editor->activeJoint);   // toggle joint selection
                    editor->activeJoint = nullptr;
                    editor->state = EDITOR_IDLE;
                } else 
                {
                    printf("Shift Key pressed, add joint to selection and make new joint parent of previous joint\n");
                    AddJointToSelection(editor, editor->activeJoint);
                    SetJointHierachieFromSelection(editor, scene);
                    PrintJointSelection(editor);
                    editor->state = EDITOR_IDLE;
                }
            }
            break;
        
        case EDITOR_PRESS_CONNECTION:
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            {
                // remove connection
                printf("Removing parent %p from %p\n", editor->hoveredConnectionToParent->parent, editor->hoveredConnectionToParent);
                ClearJointsSelection(editor);
                editor->hoveredConnectionToParent->parent = nullptr;
                editor->state = EDITOR_IDLE;
            }
            break;

        case EDITOR_SELECTION:            
            // select joints inside selection
            Rectangle selectionRect = GetSelectionRect(editor);
            for (int i = 0; i < scene->jointCount; ++i)
            {
                Vector2 jointPos = M33_GetPosition(scene->joints[i].transformation);
                if(CheckCollisionCircleRec(jointPos, scene->joints[i].radius, selectionRect))
                {
                    AddJointToSelection(editor, &scene->joints[i]);
                }
            }
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            {
                editor->state = EDITOR_IDLE;
            }
            break;

        case EDITOR_DRAG_JOINT:
            if(!editor->activeJoint->isFixed) 
            {
                SetJointPosition(editor->activeJoint, GetMousePosition());
            }
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            {
                editor->activeJoint = nullptr;
                editor->state = EDITOR_IDLE;
            }
            break;

        default:
            break;
    }
}

void DrawScene(Scene* scene, Editor* editor)
{
    // Draw joints
    for (int i = 0; i < scene->jointCount; ++i)
    {
        Color color = GRAY;
        if (&scene->joints[i] == editor->hoveredJoint) color = RED;
        if (IsJointSelected(editor, &scene->joints[i])) color = GREEN;
        Vector2 jointPos = M33_GetPosition(scene->joints[i].transformation);
        float jointRadius = scene->joints[i].radius;
        DrawCircleV(jointPos, scene->joints[i].radius, color);

        // Draw cross if fixed
        if (scene->joints[i].isFixed)
        {
            DrawLine(
                     jointPos.x,
                     jointPos.y - jointRadius * 2,
                     jointPos.x,
                     jointPos.y + jointRadius * 2,
                     BLACK);
            DrawLine(
                     jointPos.x - jointRadius * 2,
                     jointPos.y,
                     jointPos.x + jointRadius * 2,
                     jointPos.y,
                     BLACK);
        }

        // Draw connection to parent joint
        Vector2 conStart, conEnd;
        if (GetConnectionLineToParent(&scene->joints[i], &conStart, &conEnd))
        {
            Color color = BLACK;
            if (&scene->joints[i] == editor->hoveredConnectionToParent) color = RED;
            DrawLineEx(conStart, conEnd, 3, color);
        }
    }

    // Draw Selection Rectangle
    if (editor->state == EDITOR_SELECTION) 
    {
        DrawRectangleLinesEx(GetSelectionRect(editor), 1, BLACK);
    }
}

void DrawGui(Scene* scene, Editor* editor)
{
    // int buffer[MAX_JOINTS];
    // int cnt = GetSelectedJoints(scene, buffer);
    // if (cnt == 1)
    // {
    //     Joint* joint = &scene->joints[buffer[0]];
    //     bool fixed = joint->isFixed;
    //     GuiCheckBox((Rectangle){ 25, 25, 15, 15 }, "Fixed", &fixed);
    //     joint->isFixed = fixed;
    // }
    // else if (cnt == 2)
    // {
    //     char label[64];
    //     snprintf(label, 64, "Count: %d, joints: %d, %d", cnt, buffer[0], buffer[1]);
    //     GuiLabel((Rectangle){ 25, 45, 150, 15 }, label);

    // }
}

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "LilKin - the little kinetic helper");

    SetTargetFPS(60);

    Rectangle guiBounds = { 0, 0, 100, screenHeight };
        
    Scene scene = { 0 };
    Editor editor = { 
        .state = EDITOR_IDLE,
        .hoveredJoint = nullptr,
        .hoveredConnectionToParent = nullptr,
        .activeJoint = nullptr,
        .selectedJoints = {nullptr}
    };

    
    while (!WindowShouldClose())
    {
        bool mouseOverGui = CheckCollisionPointRec(GetMousePosition(), guiBounds);
        if(!mouseOverGui)
        {
            UpdateEditor(&scene, &editor);
        }
        
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawScene(&scene, &editor);
            DrawRectangleLinesEx(guiBounds, 1, RED);
            DrawGui(&scene, &editor);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}

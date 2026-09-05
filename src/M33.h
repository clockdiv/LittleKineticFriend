#ifndef M33_H
#define M33_H

#include <math.h>
#include "raylib.h"

typedef float M33[3][3];

void M33_SetIdentity(M33 m)
{
    for(int row = 0; row < 3; ++row)
        for(int col = 0; col < 3; ++col)
            m[row][col] = (row == col) ? 1.0f : 0.0f;        
}

// void M33_Transform(M33 m, Vector2 pos, float radians)
// {
// }

// m = (Matrix3x3){
//     {1.0f, 0.0f, 0.0f},
//     {0.0f, 1.0f, 0.0f},
//     {0.0f, 0.0f, 1.0f},
// };

void M33_SetPosition(M33 m, Vector2 pos)
{
    m[0][2] = pos.x;
    m[1][2] = pos.y;
}

Vector2 M33_GetPosition(const M33 m)
{
    Vector2 pos;
    pos.x = m[0][2];
    pos.y = m[1][2];
    return pos;
}

void M33_AddPosition(M33 m, Vector2 pos)
{
    m[0][2] += pos.x;
    m[1][2] += pos.y;
}

void M33_SetRotation(M33 m, float rad)
{
    float c = cosf(rad);
    float s = sinf(rad);
    m[0][0] = c;
    m[0][1] = -s;
    m[1][0] = s;
    m[1][1] = c;
}

float M33_GetRotation(const M33 m)
{
    float s = m[1][0];
    float c = m[0][0];
    return atan2f(s, c);
}

void M33_AddRotation(M33 m, float rad)
{
    float currentRad = M33_GetRotation(m);
    float newRad = currentRad + rad;
    M33_SetRotation(m, newRad);

}

#endif
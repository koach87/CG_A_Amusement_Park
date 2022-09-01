#version 430 core

// 我的位子
layout(location = 0) in vec3 aPos;

// 貼圖
out vec3 TexCoords;


// 必備ㄉ
uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}
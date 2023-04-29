#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 36) out;

uniform mat4 ProjView;
uniform vec3 CamRight, CamUp;

in VS_OUT {
    vec3 gl_Position;
    vec4 inPosAndSize;
} gs_in[];

const float f = 0.1f;
const int elements[] = int[]
(
    0,2,1,
    2,3,1,

    1,3,5,
    3,7,5,

    5,7,4,
    7,6,4,

    4,6,0,
    6,2,0,

    3,2,7,
    2,6,7,

    5,4,1,
    4,0,1
);

void main()
{   
  // Get points of triangle
  vec3 p0 = gs_in[0].gl_Position;
  vec3 p1 = gs_in[1].gl_Position;
  vec3 p2 = gs_in[2].gl_Position;

  // Calulate center point
  vec3 inVert = (p0 + p1 + p2) / 3;
  float sz = (gs_in[0].inPosAndSize.w + gs_in[1].inPosAndSize.w + gs_in[2].inPosAndSize.w) * 3;
  vec3 pos = (gs_in[0].inPosAndSize.xyz + gs_in[1].inPosAndSize.xyz + gs_in[2].inPosAndSize.xyz) / 3;  
  
  vec3 vertices[] = vec3[]
  (
      pos + vec3(-f,-f,-f),
      pos + vec3(-f,-f,+f), 
      pos + vec3(-f,+f,-f), 
      pos + vec3(-f,+f,+f), 
      pos + vec3(+f,-f,-f), 
      pos + vec3(+f,-f,+f), 
      pos + vec3(+f,+f,-f), 
      pos + vec3(+f,+f,+f)
  );
  
  int uiIndex = 0;
  for(int uiTri = 0; uiTri < 12; ++uiTri)
  {
      for(int uiVert = 0; uiVert < 3; ++uiVert)
      {
          float x = inVert.x;
          float y = inVert.y;
          vec4 worldPos = vec4(vertices[elements[uiIndex++]] + CamRight * x * sz + CamUp * y * sz, 1.0);
          gl_Position = ProjView * worldPos;
          
          EmitVertex();
      }

      EndPrimitive();
  }
}
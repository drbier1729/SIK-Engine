#version 330 core

uniform float min_depth;
uniform float max_depth;

in vec4 shadow_pos;

void main()
{
    //Convert depth into relative depth
    float relative_depth = (max_depth - shadow_pos.w) / (max_depth - min_depth);
    gl_FragColor = shadow_pos;
    gl_FragColor.xyz = vec3(relative_depth);
    gl_FragColor.w = shadow_pos.w; 
}

#version 400

in vec3 vertex_position;
in vec3 vertex_colour;
in float scalar;
in float scalar2;

out vec3 colour;

uniform mat4 mvp;
uniform vec4 trans;
uniform float scalarThreshold;
uniform float scalarThreshold2;

void main ()
{
    if(scalar > scalarThreshold && scalar2 < scalarThreshold2 )
        colour = vertex_colour;
    else
        colour = vec3(0.0,0.0,0.0);
    
    gl_Position =  mvp * vec4 (vertex_position, 1.0) + trans;
}
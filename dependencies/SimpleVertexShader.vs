#version 400

in vec3 vertex_position;
in vec3 vertex_colour;
in float scalar0;
in float scalar1;
in float scalar2;
in float scalar3;
in float scalar4;

out vec3 colour;
out float alpha;

uniform mat4 mvp;
uniform vec4 trans;
uniform float scalarThreshold0;
uniform float scalarThreshold1;
uniform float scalarThreshold2;
uniform float scalarThreshold3;
uniform float scalarThreshold4;
uniform int numScalars;

void main ()
{
    bool hide = true;
    
    if(numScalars == 5)
    {
        if( scalar0 > scalarThreshold0 &&
            scalar1 < scalarThreshold1 &&
            scalar2 < scalarThreshold2 &&
            scalar3 < scalarThreshold3 &&
            scalar4 < scalarThreshold4)
            hide = false;
    }
    else if(numScalars == 4)
    {
        if( scalar0 > scalarThreshold0 &&
            scalar1 < scalarThreshold1 &&
            scalar2 < scalarThreshold2 &&
            scalar3 < scalarThreshold3)
            hide = false;
    }
    else if(numScalars == 3)
    {
        if( scalar0 > scalarThreshold0 &&
            scalar1 < scalarThreshold1 &&
            scalar2 > scalarThreshold2)
            hide = false;
    }
    else if(numScalars == 2)
    {
        if( scalar0 > scalarThreshold0 &&
            scalar1 < scalarThreshold1)
            hide = false;
    }
    else if(numScalars == 1)
    {
        if( scalar0 > scalarThreshold0)
            hide = false;
    }

    if(hide)
        alpha = 0.0;
    else
        alpha = 1.0;
    
    colour = vertex_colour;
    
    gl_Position =  mvp * vec4 (vertex_position, 1.0) + trans;
}
#version 400

//uniform vec4 inputColour;

in vec3 colour;
in float alpha;
out vec4 frag_colour;

void main ()
{
    frag_colour = vec4 (colour, alpha);
}
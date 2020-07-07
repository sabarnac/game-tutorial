#version 330 core

// The color to use for coloring the object line fragments.
uniform vec4 lineColor;

// The final color of the fragment.
out vec4 color;

void main()
{
	// We color the line as defined by the debug renderer.
	// This is because we need to use different colors for
	//   different kinds of debug information.
	color = lineColor;
}
#ifdef __APPLE__
#include <GLUT/glut.h>
#  include <OpenGL/gl3.h>
#  include <OpenGL/glext.h>

#else /// windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#define GLEW_STATIC
#define NDEBUG
#include <GL/glew.h>
#include <gl/glext.h>
#include <GL/glut.h>
#endif

// Include OpenGL headers
#include <GLFW/glfw3.h>

// Include TCLAP headers
#include "tclap/CmdLine.h"

// Include AntTweakBar
#include <AntTweakBar.h>

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <string>

// Include GLM
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Include PNG writer library
#include "lodepng.h"

// Definitions
#define GLFW_CDECL
#define HALFPI 1.57079632679

// Global variables
int width, height;
std::string filePath;
float radius;
double savedx;
double savedy;
double savedxm;
double savedym;
double savedyr;
double centx;
double centy;
double transx;
double transy;
float scalarThreshold = 3;
float scalarThreshold2 = 100;
TwBar* bar;
float* scalarThresholds;
int numScalars;
char** scalarNames;
float yaw;
float pitch;
bool insideTool;
bool retinaTrue;

/* CALLBACKS */
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    TwEventMouseButtonGLFW(button, action);
    if(action == GLFW_PRESS)
    {
        int pos[2];
        TwGetParam(bar, NULL, "position", TW_PARAM_INT32, 2, pos);
        int size[2];
        TwGetParam(bar, NULL, "size", TW_PARAM_INT32, 2, size);
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
		if (retinaTrue)
		{

			if (xpos * 2 - pos[0] > 0 && xpos * 2 - pos[0] < size[0] &&
				ypos * 2 - pos[0] > 0 && ypos * 2 - pos[1] < size[1])
				insideTool = true;
		}
		else
		{
			if (xpos - pos[0] > 0 && xpos - pos[0] < size[0] &&
				ypos - pos[0] > 0 && ypos - pos[1] < size[1])
				insideTool = true;
		}
    }
    else if(action == GLFW_RELEASE)
        insideTool = false;
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (retinaTrue)
		TwMouseMotion(xpos*2,ypos*2);
	else
		TwMouseMotion(xpos, ypos);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    TwEventKeyGLFW(key, action);
}

void char_callback(GLFWwindow* window, unsigned int codepoint)
{
    TwEventCharGLFW(codepoint, 1);
}

void window_close_callback(GLFWwindow* window)
{
	glfwGetWindowSize(window, &width, &height);
	int w, h;
	if (retinaTrue)
	{
		w = width * 2;
		h = height * 2;
	}
	else
	{
		w = width;
		h = height;
	}

	// save screenshot on exit
	GLubyte* pixels = new GLubyte[w*h * 4];
	glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	std::string pngpaths = (filePath + "screenshot.png");
	const char* pngpath = pngpaths.c_str();
	printf("%s\n", pngpath);

	//generate some image
	std::vector<unsigned char> image;
	image.resize(w * h * 4);
	for (unsigned x = 0; x < w; x++)
	for (unsigned y = 0; y < h; y++)
	{
		image[4 * w * y + 4 * x + 0] = static_cast<unsigned char>(pixels[4 * w * (h - y - 1) + 4 * x + 0]);
		image[4 * w * y + 4 * x + 1] = static_cast<unsigned char>(pixels[4 * w * (h - y - 1) + 4 * x + 1]);
		image[4 * w * y + 4 * x + 2] = static_cast<unsigned char>(pixels[4 * w * (h - y - 1) + 4 * x + 2]);
		image[4 * w * y + 4 * x + 3] = 255;
	}
	encodeOneStep(pngpath, image, w, h);

	// save settings to text file
	std::ofstream out_file(filePath + "settings_out.txt");
	if (!out_file.is_open())
	{
		printf("File: not found.");
	}
	for (int i = 0; i<numScalars; i++)
	{
		out_file << scalarNames[i] << ":" << scalarThresholds[i] << std::endl;
	}
	out_file << "yaw:" << yaw << std::endl;
	out_file << "pitch:" << pitch << std::endl;
	out_file << "radius:" << radius << std::endl;
	out_file << "tx:" << transx << std::endl;
	out_file << "ty:" << transy << std::endl;
	out_file.close();

	glfwSetWindowShouldClose(window, GL_TRUE);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	int pos[2];
	TwGetParam(bar, NULL, "position", TW_PARAM_INT32, 2, pos);
	int size[2];
	TwGetParam(bar, NULL, "size", TW_PARAM_INT32, 2, size);

	bool withinGUI = false;
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	if (retinaTrue)
	{

		if (xpos * 2 - pos[0] > 0 && xpos * 2 - pos[0] < size[0] &&
			ypos * 2 - pos[0] > 0 && ypos * 2 - pos[1] < size[1])
			withinGUI = true;
	}
	else
	{
		if (xpos - pos[0] > 0 && xpos - pos[0] < size[0] &&
			ypos - pos[0] > 0 && ypos - pos[1] < size[1])
			withinGUI = true;
	}

	if (withinGUI)
		TwMouseWheel(yoffset);
	else
	{

		radius = std::min(30.0, std::max(4.0, radius + yoffset));
		printf("radius: %f\n", radius);
		//    scalarThreshold = std::max(0.0,scalarThreshold + yoffset*0.1);
		//    printf("radius: %f\n",scalarThreshold);
	}

}

void _update_fps_counter(GLFWwindow* window)
{
	static double previous_seconds = glfwGetTime();
	static int frame_count;
	double current_seconds = glfwGetTime();
	double elapsed_seconds = current_seconds - previous_seconds;
	if (elapsed_seconds > 0.25) {
		previous_seconds = current_seconds;
		double fps = (double)frame_count / elapsed_seconds;
		char tmp[128];
		sprintf(tmp, "Fiber Viewer (v1.0) @ fps: %.2f", fps);
		glfwSetWindowTitle(window, tmp);
		frame_count = 0;
	}
	frame_count++;
}

// Rescale the viewport as needed
void Reshape(GLFWwindow* window, int NewWidth, int NewHeight)
{
	// Apply needed glut viewport updates
	glViewport(0, 0, NewWidth, NewHeight);

	// Send the new window size to AntTweakBar
	TwWindowSize(NewWidth, NewHeight);

	glfwSetWindowSize(window, NewWidth, NewHeight);
}

/* END CALLBACKS */

// Perspective camera projection
glm::mat4 camera(float Translate, glm::vec2 const & Rotate)
{
    glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.f);
    glm::mat4 View = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Translate));
    View = glm::rotate(View, Rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f));
    View = glm::rotate(View, Rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
    return Projection * View * Model;
}

// PNG image encoding
void encodeOneStep(const char* filename, std::vector<unsigned char>& image, unsigned width, unsigned height)
{
	//Encode the image
	unsigned error = lodepng::encode(filename, image, width, height);

	//if there's an error, display it
	if (error) std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
}

// Print log for compiled shader
void _print_programme_info_log (GLuint sp)
{
    int max_length = 2048;
    int actual_length = 0;
    char log[2048];
    glGetProgramInfoLog (sp, max_length, &actual_length, log);
    printf ("program info log for GL index %i:\n%s", sp, log);
}


// Check if compiled shader is valid
bool is_valid (GLuint programme)
{
    glValidateProgram (programme);
    int params = -1;
    glGetProgramiv (programme, GL_VALIDATE_STATUS, &params);
    printf ("program %i GL_VALIDATE_STATUS = %i\n", programme, params);
    if (GL_TRUE != params)
    {
        _print_programme_info_log (programme);
        return false;
    }
    return true;
}

/* Main entry point */
int main (int argc, const char * argv[])
{
    try
    {
        // Command line arguments setup
        TCLAP::CmdLine cmd("Command description message", ' ', "0.1");
        TCLAP::ValueArg<std::string> path_files("",
                                              "files",
                                              "Path to the files",
                                              true,
                                              "",
                                              "Path to files");
		TCLAP::ValueArg<float> yawarg("", "yaw", "Yaw", false, -.3,"Yaw");
		TCLAP::ValueArg<float> pitcharg("", "pitch", "Pitch", false, -.3, "Pitch");
		TCLAP::ValueArg<float> radiusarg("", "radius", "Radius", false, 5, "Radius");
		TCLAP::ValueArg<float> txarg("", "tx", "Translate center x", false, 0, "Translate center x");
		TCLAP::ValueArg<float> tyarg("", "ty", "Translate center y", false, 0, "Translate center y");
		TCLAP::ValueArg<int> windowWidtharg("", "windowWidth", "windowWidth", false, 1024, "windowWidth");
		TCLAP::ValueArg<int> windowHeightarg("", "windowHeight", "windowHeight", false, 768, "windowHeight");
		
        cmd.add( path_files );
		cmd.add(yawarg);
		cmd.add(pitcharg);
		cmd.add(radiusarg);
		cmd.add(txarg);
		cmd.add(tyarg);
		cmd.add(windowWidtharg);
		cmd.add(windowHeightarg);

        // Parse the args.
        cmd.parse( argc, argv );
        
		// Get values of the args
        filePath = path_files.getValue();
		yaw = yawarg.getValue();
		pitch = pitcharg.getValue();
		radius = radiusarg.getValue();
		transx = txarg.getValue();
		transy = tyarg.getValue();
        
        // start GL context and O/S window using the GLFW helper library
        if (!glfwInit ()) {
            fprintf (stderr, "ERROR: could not start GLFW3\n");
            return 1;
        }

		// Do we have a retina display?
		retinaTrue = false;
#ifdef __APPLE__
		retinaTrue = true;
#endif
        
        // load data -- vertices
        std::ifstream file(filePath + "vertices.dat", std::ifstream::in | std::ifstream::binary);
        if(!file.is_open())
        {
            printf("File: not found.");
            return -1;
        }
        int numpoints;
        file.read(reinterpret_cast<char*>(&numpoints),sizeof(int));
        float* vertices = (float*)malloc(numpoints*sizeof(float));
        file.read(reinterpret_cast<char*>(vertices),numpoints*sizeof(float));
        file.close();
        
		// read data -- colors
        std::ifstream file2(filePath + "colors.dat", std::ifstream::in | std::ifstream::binary);
        if(!file2.is_open())
        {
            printf("File: not found.");
            return -1;
        }
        float* colors2 = (float*)malloc(numpoints*sizeof(float));
        file2.read(reinterpret_cast<char*>(&numpoints),sizeof(int));
        file2.read(reinterpret_cast<char*>(colors2),numpoints*sizeof(float));
        file2.close();
        
		// read data -- indices
        std::ifstream file3(filePath+"indices.dat", std::ifstream::in | std::ifstream::binary);
        if(!file3.is_open())
        {
            printf("File: not found.");
            return -1;
        }
        int numFibers;
        file3.read(reinterpret_cast<char*>(&numFibers),sizeof(int));
        int* indices = (int*)malloc(numFibers*sizeof(int));
        file3.read(reinterpret_cast<char*>(indices),numFibers*sizeof(int));
        file3.close();

		// read data -- scalars (multiple scalars possible)
        std::ifstream file4(filePath+"scalars.dat", std::ifstream::in | std::ifstream::binary);
        if(!file4.is_open())
        {
            printf("File: not found.");
            return -1;
        }
        file4.read(reinterpret_cast<char*>(&numScalars),sizeof(int));
        int* scalarDataLengths = (int*)malloc(sizeof(int)*numScalars);
        float** scalars = (float**)malloc(sizeof(float*)*numScalars); // multiple scalars hence pointers to pointers
        scalarNames = (char**)malloc(sizeof(char*)*numScalars);
        float* scalarMinRange = (float*)malloc(sizeof(float)*numScalars);
        float* scalarMaxRange = (float*)malloc(sizeof(float)*numScalars);
        scalarThresholds = (float*)malloc(numScalars*sizeof(float));
        int i = 0;
        while( i<numScalars) //!file4.eof() ||
        {
            int namelength;
            file4.read(reinterpret_cast<char*>(&namelength),sizeof(int));
                
            char* scalarName = (char*)malloc(sizeof(char)*namelength+1);
            file4.read(scalarName,sizeof(char)*namelength);
			scalarName[namelength] = NULL;
            scalarNames[i] = scalarName;
            printf("Loaded scalar:%s\n",scalarName);
                
            int datalength;
            file4.read(reinterpret_cast<char*>(&datalength),sizeof(int));
            scalarDataLengths[i] = datalength;
                
            float* scalarValues = (float*)malloc(datalength*sizeof(float));
            file4.read(reinterpret_cast<char*>(scalarValues),datalength*sizeof(float));
            scalars[i] = scalarValues;
                
            float minRange;
            file4.read(reinterpret_cast<char*>(&minRange),sizeof(float));
            scalarMinRange[i] = minRange;
                
            float maxRange;
            file4.read(reinterpret_cast<char*>(&maxRange),sizeof(float));
            scalarMaxRange[i] = maxRange;
                
            float threshold;
            file4.read(reinterpret_cast<char*>(&threshold),sizeof(float));
            scalarThresholds[i] = threshold;
                
            i++;
        }
        file4.close();

		// Set resolution from args
		width = windowWidtharg.getValue();
        height = windowHeightarg.getValue();
        
		// Window hints
        glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 4); // OpenGL version -- we need 4
        glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 4);
        glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); // Window not resizable, troubles with aspect ratio
        glfwWindowHint(GLFW_SAMPLES, 16); // Makes multisampling possible (antialiasing)
        glfwWindowHint(GLFW_DEPTH_BITS, 16); // Alpha bits
        
        // Create the actual window
        GLFWwindow* window = glfwCreateWindow (width, height, "Fiber Viewer (v1.0)", NULL, NULL);
        if (!window)
        {
            fprintf (stderr, "ERROR: could not open window with GLFW3\n");
            glfwTerminate();
            return 1;
        }
        glfwMakeContextCurrent (window);
        
        // start GLEW extension handler
#ifndef __APPLE__
        glewExperimental = GL_TRUE;
        glewInit();
#endif
        
        // Initialize AntTweakBar
        TwInit(TW_OPENGL_CORE, 0);
        glfwGetWindowSize(window, &width, &height);  // Send the new window size to AntTweakBar
		if (retinaTrue)
			TwWindowSize(width*2, height*2);
		else
			TwWindowSize(width, height);
        
        // get version info
        const GLubyte* renderer = glGetString (GL_RENDERER); // get renderer string
        const GLubyte* version = glGetString (GL_VERSION); // version as a string
        printf ("Renderer: %s\n", renderer);
        printf ("OpenGL version supported %s\n", version);
        
        // tell GL to only draw onto a pixel if the shape is closer to the viewer
        glEnable (GL_DEPTH_TEST); // enable depth-testing
        glDepthFunc (GL_LESS); // depth-testing interprets a smaller value as "closer"
        
        // create 3 VBO buffers
        int numberOfBuffers = 2 + numScalars; // vertex buffer, color buffer, scalar buffers
		GLuint* vbo = (GLuint*)malloc(sizeof(GLuint)*numberOfBuffers);
        glGenBuffers (numberOfBuffers, &vbo[0]);
        
        // fill the vertex buffer
        glBindBuffer (GL_ARRAY_BUFFER, vbo[0]);
        glBufferData (GL_ARRAY_BUFFER, numpoints*sizeof(float), vertices, GL_STATIC_DRAW);
        
        // fill the color buffer
        glBindBuffer (GL_ARRAY_BUFFER, vbo[1]);
        glBufferData (GL_ARRAY_BUFFER, numpoints*sizeof(float), colors2, GL_STATIC_DRAW);
        for(int i = 0; i<numScalars; i++)
        {
            glBindBuffer (GL_ARRAY_BUFFER, vbo[2+i]);
            glBufferData (GL_ARRAY_BUFFER, scalarDataLengths[i]*sizeof(float), scalars[i], GL_STATIC_DRAW);
        }
        
        
        GLuint vao = 0;
        glGenVertexArrays (1, &vao);
        glBindVertexArray (vao);
        
        glBindBuffer (GL_ARRAY_BUFFER, vbo[0]);
        glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL); // index, size, type, normalized, byte
        glBindBuffer (GL_ARRAY_BUFFER, vbo[1]);
        glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray (0);
        glEnableVertexAttribArray (1);
        
        for(int i = 0; i<numScalars; i++)
        {
            glBindBuffer (GL_ARRAY_BUFFER, vbo[2+i]);
            glVertexAttribPointer (2+i, 1, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray (2+i);
        }
        
        // fill index buffer
        GLuint ibo;
        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numFibers*sizeof(int), indices, GL_STATIC_DRAW);
        
        // Read the Vertex Shader code from the file
		std::string VertexShaderCode = "";
		std::ifstream VertexShaderStream(filePath + "SimpleVertexShader.vs", std::ifstream::in);
        if(VertexShaderStream.is_open())
        {
            std::string Line = "";
            while(getline(VertexShaderStream, Line))
                VertexShaderCode += "\n" + Line;
            VertexShaderStream.close();
        }
        else
        {
            printf("Vertex shader file not found.\n");
            exit(0);
        }
        const char* vertex_shader = VertexShaderCode.c_str();
        
        // Read the Fragment Shader code from the file
		std::string FragmentShaderCode = "";
		std::ifstream FragmentShaderStream(filePath + "SimpleFragmentShader.fs", std::ios::in);
        if(FragmentShaderStream.is_open())
        {
            std::string Line = "";
            while(getline(FragmentShaderStream, Line))
                FragmentShaderCode += "\n" + Line;
            FragmentShaderStream.close();
        }
        else
        {
            printf("Fragment shader file not found.\n");
            exit(0);
        }
        const char* fragment_shader = FragmentShaderCode.c_str();
        
        // Compile the vertex and fragment shaders
        GLuint vs = glCreateShader (GL_VERTEX_SHADER);
        glShaderSource (vs, 1, &vertex_shader, NULL);
        glCompileShader (vs);
        GLuint fs = glCreateShader (GL_FRAGMENT_SHADER);
        glShaderSource (fs, 1, &fragment_shader, NULL);
        glCompileShader (fs);
        
        // Save the shaders in a program
        GLuint shader_programme = glCreateProgram ();
        glAttachShader (shader_programme, fs);
        glAttachShader (shader_programme, vs);
        
		// Bind positions to variable names inside shader
        glBindAttribLocation (shader_programme, 0, "vertex_position");
        glBindAttribLocation (shader_programme, 1, "vertex_colour");
        for(int i = 0; i<numScalars; i++)
        {
            char scalarName[8];
            sprintf(scalarName, "scalar%d", i);
            glBindAttribLocation (shader_programme, 2+i, scalarName);
        }
        
		// Link shader and check validity
        glLinkProgram (shader_programme);
        is_valid(shader_programme);
        
        // Delete reference to shaders
        glDeleteShader(vs);
        glDeleteShader(fs);
        
        // Create a tweak bar
        bar = TwNewBar("Settings");
        TwDefine(" GLOBAL help='This example shows how to integrate AntTweakBar with GLFW and OpenGL.' "); // Message added to the help bar.
       
		// Create controls for each scalar
        for(int i = 0; i<numScalars; i++)
        {
            char settings[100];
            sprintf(settings, "label='%s' min=%f max=%f step=%f", scalarNames[i],scalarMinRange[i],scalarMaxRange[i],(scalarMaxRange[i]-scalarMinRange[i])/1000.0);
            TwAddVarRW(bar, scalarNames[i], TW_TYPE_FLOAT, scalarThresholds+i,
                       settings);
        }
        
		// Separator bar
        TwAddSeparator(bar, "", "");
        
		// Anti-aliasing toggle
        bool antialias = false;
        glDisable(GL_MULTISAMPLE_ARB);
        TwAddVarRW(bar, "Anti-aliasing", TW_TYPE_BOOLCPP, &antialias,"");        
        TwSetCurrentWindow(0);
        
        // Set primitive restart index, such that we can draw multiple lines in a single GL call
        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(65535);
        
		// use default light diffuse and position
        glEnable(GL_LIGHT0);    
        glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
        
		// Set callback functions
        glfwSetWindowCloseCallback(window, window_close_callback);
        glfwSetScrollCallback(window, scroll_callback);
        glfwSetCursorPosCallback(window, cursor_pos_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetKeyCallback(window, key_callback);
        glfwSetCharCallback(window, char_callback);
		glfwSetWindowSizeCallback(window, Reshape);
                
        // Some default vars
        centx = centy = 0.0;
        insideTool = false;
		savedx = savedy = -1;
        
        // GL options
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // enable alpha blending
        glEnable( GL_BLEND );
		glClearColor(0,0,0, 1); // background color = black
        
		glm::mat4 cam;
        while (!glfwWindowShouldClose (window))
        {
            _update_fps_counter (window);
            // wipe the drawing surface clear
            glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
			// enable multisampling if toggled
            if(antialias)
                glEnable(GL_MULTISAMPLE_ARB);
            else
                glDisable(GL_MULTISAMPLE_ARB);
            
            // get the current position of a bar
            int pos[2];
            TwGetParam(bar, NULL, "position", TW_PARAM_INT32, 2, pos);
            int size[2];
            TwGetParam(bar, NULL, "size", TW_PARAM_INT32, 2, size);
            
			// Check if the cursor is inside the GUI to enable interaction
            bool withinGUI = false;
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
			if (retinaTrue)
			{
				if(xpos*2 - pos[0] > 0 && xpos*2 - pos[0] < size[0] &&
				   ypos*2 - pos[0] > 0 && ypos*2 - pos[1] < size[1] )
					withinGUI = true;
			}
			else
			{
				if (xpos - pos[0] > 0 && xpos - pos[0] < size[0] &&
					ypos - pos[0] > 0 && ypos - pos[1] < size[1])
					withinGUI = true;
			}
            
            glUseProgram (shader_programme);
            glBindVertexArray (vao);
            
			// Let the shader know how many scalars weve got
            GLint loc10 = glGetUniformLocation(shader_programme, "numScalars");
            if (loc10 != -1)
            {
                glUniform1i(loc10, numScalars);
            }
            
			// Set scalar thresholds inside the shader
            for(int i = 0; i<numScalars; i++)
            {
                char scalarThresholdName[17];
                sprintf(scalarThresholdName, "scalarThreshold%d", i);
                
                GLint loc = glGetUniformLocation(shader_programme, scalarThresholdName);
                if (loc != -1)
                {
                    glUniform1f(loc, (float)scalarThresholds[i]);
                }
            }
            
            // Do some GUI interaction stuff
            if(!withinGUI && !insideTool)
            {
				// Left mouse button
                int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
                if (state == GLFW_PRESS)
                {
                
                    double xpos, ypos;
                    glfwGetCursorPos(window, &xpos, &ypos);
                    
                    if(savedx != -1)
                    {
                        pitch += (xpos - savedx)*0.01;
                        //printf("%d %f\n",xpos,savedx);
                    }
                    savedx = xpos;
                    
                    if(savedy != -1)
                    {
                        yaw -= (ypos - savedy)*0.01;
                        yaw = fmin(yaw,HALFPI-0.1);
                        yaw = fmax(yaw,-HALFPI+0.1);
                    }
                    savedy = ypos;
                    
                }
                else
                    savedx = savedy = -1;
                
				// Middle mouse button
                int statem = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);
                if (statem == GLFW_PRESS)
                {
                    double xpos, ypos;
                    glfwGetCursorPos(window, &xpos, &ypos);
                    
                    if(savedxm != -1)
                    {
                        transx += (xpos - savedxm)*0.01;
                    }
                    savedxm = xpos;
                    
                    if(savedym != -1)
                    {
                        transy -= (ypos - savedym)*0.01;
                    }
                    savedym = ypos;
                }
                else
                    savedxm = savedym = -1;                
            }
            
			// Camera perspective calculation stuff
            glm::vec3 target(centx,centy,0);
            glm::vec3 position(centx+radius,centy,0);
            glm::vec3 up(0,0,-1);
            
            glm::vec3 target_to_camera = position - target;
            glm::vec3 camera_right = glm::normalize(glm::cross(target_to_camera, up));
            glm::vec3 camera_up = glm::normalize(glm::cross(camera_right, target_to_camera));
            
            glm::mat4 rotate_yaw_matrix = glm::mat4(1.f);
            glm::mat4 rotate_pitch_matrix = glm::mat4(1.f);
            
            rotate_yaw_matrix = glm::rotate(rotate_yaw_matrix, yaw, camera_right);
            rotate_pitch_matrix = glm::rotate(rotate_pitch_matrix, pitch, camera_up);
            
            target_to_camera = glm::vec3(rotate_pitch_matrix * rotate_yaw_matrix * glm::vec4(target_to_camera,1.0f));
            target_to_camera += target;
            
            cam = glm::perspective(150.0f, (float)width/height, 1.0f, 1000.f) *
              glm::lookAt(target_to_camera, target, up);
            
            glm::vec4 trans(transx,transy,0,0);
			
            // Give the computed matrix to the shader
            GLint loc2 = glGetUniformLocation(shader_programme, "mvp");
            if (loc2 != -1)
            {
                glUniformMatrix4fv(loc2, 1, GL_FALSE, glm::value_ptr(cam));
            }
            
			// Set the translation vector
            GLint loc3 = glGetUniformLocation(shader_programme, "trans");
            if (loc3 != -1)
            {
                glUniform4fv(loc3, 1, glm::value_ptr(trans));
            }
            
            
            // draw points 0-3 from the currently bound VAO with current in-use shader
            //glDrawArrays (GL_LINE_STRIP, 0, numpoints/3);
            glDrawElements(GL_LINE_STRIP, numFibers, GL_UNSIGNED_INT, 0);
            
            // update other events like input handling
            glfwPollEvents ();
            
            // Draw tweak bars
            TwDraw();
            
            // put the stuff we've been drawing onto the display
            glfwSwapBuffers (window);
            
            // close on escape key
            if (GLFW_PRESS == glfwGetKey (window, GLFW_KEY_ESCAPE))
                glfwSetWindowShouldClose (window, 1);
        }
        
        // close GL context and any other GLFW resources
        TwTerminate();
        glfwTerminate();
    }
    catch (TCLAP::ArgException &e)  // catch any exceptions
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return EXIT_FAILURE;
    }
    
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>

#include "glew/glew.h"
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "GL/glfw.h"
#include "stb/stb_image.h"

#include "glm/glm.hpp"
#include "glm/vec3.hpp" // glm::vec3
#include "glm/vec4.hpp" // glm::vec4, glm::ivec4
#include "glm/mat4x4.hpp" // glm::mat4
#include "glm/gtc/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "glm/gtc/type_ptr.hpp" // glm::value_ptr

#ifndef DEBUG_PRINT
#define DEBUG_PRINT 1
#endif

#if DEBUG_PRINT == 0
#define debug_print(FORMAT, ...) ((void)0)
#else
#ifdef _MSC_VER
#define debug_print(FORMAT, ...) \
    fprintf(stderr, "%s() in %s, line %i: " FORMAT "\n", \
        __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)
#else
#define debug_print(FORMAT, ...) \
    fprintf(stderr, "%s() in %s, line %i: " FORMAT "\n", \
        __func__, __FILE__, __LINE__, __VA_ARGS__)
#endif
#endif

// Font buffers
extern const unsigned char DroidSans_ttf[];
extern const unsigned int DroidSans_ttf_len;    

struct ShaderGLSL
{
    enum ShaderType
    {
        VERTEX_SHADER = 1,
        FRAGMENT_SHADER = 2,
        GEOMETRY_SHADER = 4
    };
    GLuint program;
};

int compile_and_link_shader(ShaderGLSL & shader, int typeMask, const char * sourceBuffer, int bufferSize);
int destroy_shader(ShaderGLSL & shader);
int load_shader_from_file(ShaderGLSL & shader, const char * path, int typemask);
    
struct Camera
{
    float radius;
    float theta;
    float phi;
    glm::vec3 o;
    glm::vec3 eye;
    glm::vec3 up;
};

void camera_defaults(Camera & c);
void camera_zoom(Camera & c, float factor);
void camera_turn(Camera & c, float phi, float theta);
void camera_pan(Camera & c, float x, float y);

struct GUIStates
{
    bool panLock;
    bool turnLock;
    bool zoomLock;
    int lockPositionX;
    int lockPositionY;
    int camera;
    double time;
    bool playing;
    static const float MOUSE_PAN_SPEED;
    static const float MOUSE_ZOOM_SPEED;
    static const float MOUSE_TURN_SPEED;
};
const float GUIStates::MOUSE_PAN_SPEED = 0.001f;
const float GUIStates::MOUSE_ZOOM_SPEED = 0.5;
const float GUIStates::MOUSE_TURN_SPEED = 0.003f;


void init_gui_states(GUIStates & guiStates)
{
    guiStates.panLock = false;
    guiStates.turnLock = false;
    guiStates.zoomLock = false;
    guiStates.lockPositionX = 0;
    guiStates.lockPositionY = 0;
    guiStates.camera = 0;
    guiStates.time = 0.0;
    guiStates.playing = false;
}

void camera_compute(Camera & c)
{
    c.eye.x = cos(c.theta) * sin(c.phi) * c.radius + c.o.x;   
    c.eye.y = cos(c.phi) * c.radius + c.o.y ;
    c.eye.z = sin(c.theta) * sin(c.phi) * c.radius + c.o.z;   
    c.up = glm::vec3(0.f, c.phi < M_PI ?1.f:-1.f, 0.f);
}

void camera_defaults(Camera & c)
{
    c.phi = 3.14/2.f;
    c.theta = 3.14/2.f;
    c.radius = 10.f;
    camera_compute(c);
}

void camera_zoom(Camera & c, float factor)
{
    c.radius += factor ;//* c.radius ;
    if (c.radius < 0.1)
    {
        c.radius = 10.f;
        c.o = c.eye + glm::normalize(c.o - c.eye) * c.radius;
    }
    camera_compute(c);
}

void camera_turn(Camera & c, float phi, float theta)
{
    c.theta += 1.f * theta;
    c.phi   -= 1.f * phi;
    if (c.phi >= (2 * M_PI) - 0.1 )
        c.phi = 0.00001;
    else if (c.phi <= 0 )
        c.phi = 2 * M_PI - 0.1;
    camera_compute(c);
}

void camera_pan(Camera & c, float x, float y)
{
    glm::vec3 up(0.f, c.phi < M_PI ?1.f:-1.f, 0.f);
    glm::vec3 fwd = glm::normalize(c.o - c.eye);
    glm::vec3 side = glm::normalize(glm::cross(fwd, up));
    c.up = glm::normalize(glm::cross(side, fwd));
    c.o[0] += up[0] * y * c.radius * 2;
    c.o[1] += up[1] * y * c.radius * 2;
    c.o[2] += up[2] * y * c.radius * 2;
    c.o[0] -= side[0] * x * c.radius * 2;
    c.o[1] -= side[1] * x * c.radius * 2;
    c.o[2] -= side[2] * x * c.radius * 2;       
    camera_compute(c);
}



int main( int argc, char **argv )
{
    int width = 1300, height=768;
    float widthf = (float) width, heightf = (float) height;
    double t;

    int VORTEX = 1;
    int UNIVERSE = 0;

    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        exit( EXIT_FAILURE );
    }

    // Force core profile on Mac OSX
#ifdef __APPLE__
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
    glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
    // Open a window and create its OpenGL context
    if( !glfwOpenWindow( width, height, 0,0,0,0, 24, 0, GLFW_WINDOW ) )
    {
        fprintf( stderr, "Failed to open GLFW window\n" );

        glfwTerminate();
        exit( EXIT_FAILURE );
    }

    glfwSetWindowTitle( "Travel beyond Space and Time" );


    // Core profile is flagged as experimental in glew
#ifdef __APPLE__
    glewExperimental = GL_TRUE;
#endif
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
          /* Problem: glewInit failed, something is seriously wrong. */
          fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
          exit( EXIT_FAILURE );
    }

    // Ensure we can capture the escape key being pressed below
    glfwEnable( GLFW_STICKY_KEYS );

    // Enable vertical sync (on cards that support it)
    glfwSwapInterval( 1 );

    // Init viewer structures
    Camera camera;
    camera_defaults(camera);
    GUIStates guiStates;
    init_gui_states(guiStates);

    // GUI
    float numLights = 10.f;

    // Load images and upload textures
    GLuint textures[3];
    glGenTextures(3, textures);
    int x;
    int y;
    int comp; 
    unsigned char * diffuse = stbi_load("textures/spnza_bricks_a_diff.tga", &x, &y, &comp, 3);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, diffuse);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    fprintf(stderr, "Diffuse %dx%d:%d\n", x, y, comp);
    unsigned char * spec = stbi_load("textures/spnza_bricks_a_spec.tga", &x, &y, &comp, 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, x, y, 0, GL_RED, GL_UNSIGNED_BYTE, spec);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    fprintf(stderr, "Spec %dx%d:%d\n", x, y, comp);




    //LOAD SPACEBOX TEXTURE
    GLuint spaceboxTextures[7];
    glGenTextures(7, spaceboxTextures);
    //front
    unsigned char * back = stbi_load("textures/spacebox/skybox_BK.tga", &x, &y, &comp, 3);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, spaceboxTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, back);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    fprintf(stderr, "Back %dx%d:%d\n", x, y, comp);
    //right
    unsigned char * up = stbi_load("textures/spacebox/skybox_UP.tga", &x, &y, &comp, 3);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, spaceboxTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, up);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    fprintf(stderr, "Up %dx%d:%d\n", x, y, comp);
    //back
    unsigned char * front = stbi_load("textures/spacebox/skybox_FR.tga", &x, &y, &comp, 3);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, spaceboxTextures[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, front);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    fprintf(stderr, "Front %dx%d:%d\n", x, y, comp);
    //left
    unsigned char * down = stbi_load("textures/spacebox/skybox_DN.tga", &x, &y, &comp, 3);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, spaceboxTextures[3]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, down);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    fprintf(stderr, "Down %dx%d:%d\n", x, y, comp);
    //g
     unsigned char * right = stbi_load("textures/spacebox/skybox_RT.tga", &x, &y, &comp, 3);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, spaceboxTextures[4]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, right);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    fprintf(stderr, "Right %dx%d:%d\n", x, y, comp);
    //down
    unsigned char * left = stbi_load("textures/spacebox/skybox_LF.tga", &x, &y, &comp, 3);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, spaceboxTextures[5]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, left);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    fprintf(stderr, "Left %dx%d:%d\n", x, y, comp);




    //LOAD VORTEX TEXTURE
    GLuint timeVortexTextures[7];
    glGenTextures(7, timeVortexTextures);
    //front
    unsigned char * fractal = stbi_load("textures/timeVortex/fractal.jpg", &x, &y, &comp, 3);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, timeVortexTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, fractal );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // fprintf(stderr, "Back %dx%d:%d\n", x, y, comp);
    //right
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, timeVortexTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, fractal);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //fprintf(stderr, "Up %dx%d:%d\n", x, y, comp);
    //back
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, timeVortexTextures[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, fractal);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // fprintf(stderr, "Front %dx%d:%d\n", x, y, comp);
    //left
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, timeVortexTextures[3]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, fractal);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // fprintf(stderr, "Down %dx%d:%d\n", x, y, comp);
    //up
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, timeVortexTextures[4]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, fractal);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // fprintf(stderr, "Right %dx%d:%d\n", x, y, comp);
    //down
    left = stbi_load("textures/timeVortex/fractal.jpg", &x, &y, &comp, 3);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, timeVortexTextures[5]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, fractal);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // fprintf(stderr, "Left %dx%d:%d\n", x, y, comp);





    // Try to load and compile shader
    int status;
    ShaderGLSL starsVortex_shader;
    const char * shaderFileStarsVortex = "src/starsVortex.glsl";
    //int status = load_shader_from_file(gbuffer_shader, shaderFileGBuffer, ShaderGLSL::VERTEX_SHADER | ShaderGLSL::FRAGMENT_SHADER | ShaderGLSL::GEOMETRY_SHADER);
    status = load_shader_from_file(starsVortex_shader, shaderFileStarsVortex, ShaderGLSL::VERTEX_SHADER | ShaderGLSL::FRAGMENT_SHADER);
    if ( status == -1 )
    {
        fprintf(stderr, "Error on loading  %s\n", shaderFileStarsVortex);
        exit( EXIT_FAILURE );
    }    

    // Compute locations for gbuffer_shader
    GLuint starsVortex_projectionLocation = glGetUniformLocation(starsVortex_shader.program, "Projection");
    GLuint starsVortex_viewLocation = glGetUniformLocation(starsVortex_shader.program, "View");
    GLuint starsVortex_objectLocation = glGetUniformLocation(starsVortex_shader.program, "Object");
    GLuint starsVortex_timeLocation = glGetUniformLocation(starsVortex_shader.program, "Time");
    GLuint starsVortex_diffuseLocation = glGetUniformLocation(starsVortex_shader.program, "Diffuse");
    GLuint starsVortex_specLocation = glGetUniformLocation(starsVortex_shader.program, "Spec");


    ShaderGLSL starsSphere_shader;
    const char * shaderFileStarsSphere = "src/starsSphere.glsl";
    //int status = load_shader_from_file(gbuffer_shader, shaderFileGBuffer, ShaderGLSL::VERTEX_SHADER | ShaderGLSL::FRAGMENT_SHADER | ShaderGLSL::GEOMETRY_SHADER);
    status = load_shader_from_file(starsSphere_shader, shaderFileStarsSphere, ShaderGLSL::VERTEX_SHADER | ShaderGLSL::FRAGMENT_SHADER);
    if ( status == -1 )
    {
        fprintf(stderr, "Error on loading  %s\n", shaderFileStarsSphere);
        exit( EXIT_FAILURE );
    }    

    // Compute locations for gbuffer_shader
    GLuint starsSphere_projectionLocation = glGetUniformLocation(starsSphere_shader.program, "Projection");
    GLuint starsSphere_viewLocation = glGetUniformLocation(starsSphere_shader.program, "View");
    GLuint starsSphere_objectLocation = glGetUniformLocation(starsSphere_shader.program, "Object");
    GLuint starsSphere_timeLocation = glGetUniformLocation(starsSphere_shader.program, "Time");
    GLuint starsSphere_diffuseLocation = glGetUniformLocation(starsSphere_shader.program, "Diffuse");
    GLuint starsSphere_specLocation = glGetUniformLocation(starsSphere_shader.program, "Spec");



    // Load gamma shader
    ShaderGLSL gamma_shader;
    const char * shaderFilegamma = "src/gamma.glsl";
    status = load_shader_from_file(gamma_shader, shaderFilegamma, ShaderGLSL::VERTEX_SHADER | ShaderGLSL::FRAGMENT_SHADER);
    if ( status == -1 )
    {
        fprintf(stderr, "Error on loading  %s\n", shaderFilegamma);
        exit( EXIT_FAILURE );
    }    
    // Compute locations for gamma_shader
    GLuint gamma_tex1Location = glGetUniformLocation(gamma_shader.program, "Texture1");
    GLuint gamma_timeLocation = glGetUniformLocation(gamma_shader.program, "Time");



    // Load blur shader
    ShaderGLSL blur_shader;
    const char * shaderFileblur = "src/blur.glsl";
    status = load_shader_from_file(blur_shader, shaderFileblur, ShaderGLSL::VERTEX_SHADER | ShaderGLSL::FRAGMENT_SHADER);
    if ( status == -1 )
    {
        fprintf(stderr, "Error on loading  %s\n", shaderFileblur);
        exit( EXIT_FAILURE );
    }    
    // Compute locations for blur_shader
    GLuint blur_tex1Location = glGetUniformLocation(blur_shader.program, "Texture1");
    GLuint blur_timeLocation = glGetUniformLocation(blur_shader.program, "Time");





    ShaderGLSL cube_shader;
    const char * shaderFileCube= "src/cube.glsl";
    //int status = load_shader_from_file(gbuffer_shader, shaderFileGBuffer, ShaderGLSL::VERTEX_SHADER | ShaderGLSL::FRAGMENT_SHADER | ShaderGLSL::GEOMETRY_SHADER);
    status = load_shader_from_file(cube_shader, shaderFileCube, ShaderGLSL::VERTEX_SHADER | ShaderGLSL::FRAGMENT_SHADER);
    if ( status == -1 )
    {
        fprintf(stderr, "Error on loading  %s\n", shaderFileCube);
        exit( EXIT_FAILURE );
    }    

    // Compute locations for cube_shader
    GLuint cube_projectionLocation = glGetUniformLocation(cube_shader.program, "Projection");
    GLuint cube_viewLocation = glGetUniformLocation(cube_shader.program, "View");
    GLuint cube_objectLocation = glGetUniformLocation(cube_shader.program, "Object");
    GLuint cube_timeLocation = glGetUniformLocation(cube_shader.program, "Time");
    GLuint cube_diffuseLocation = glGetUniformLocation(cube_shader.program, "Diffuse");
    GLuint cube_specLocation = glGetUniformLocation(cube_shader.program, "Spec");




    ShaderGLSL fractalCube_shader;
    const char * shaderFileFractalCube= "src/fractalCube.glsl";
    //int status = load_shader_from_file(gbuffer_shader, shaderFileGBuffer, ShaderGLSL::VERTEX_SHADER | ShaderGLSL::FRAGMENT_SHADER | ShaderGLSL::GEOMETRY_SHADER);
    status = load_shader_from_file(fractalCube_shader, shaderFileFractalCube, ShaderGLSL::VERTEX_SHADER | ShaderGLSL::FRAGMENT_SHADER);
    if ( status == -1 )
    {
        fprintf(stderr, "Error on loading  %s\n", shaderFileFractalCube);
        exit( EXIT_FAILURE );
    }    

    // Compute locations for cube_shader
    GLuint fractalCube_projectionLocation = glGetUniformLocation(fractalCube_shader.program, "Projection");
    GLuint fractalCube_viewLocation = glGetUniformLocation(fractalCube_shader.program, "View");
    GLuint fractalCube_objectLocation = glGetUniformLocation(fractalCube_shader.program, "Object");
    GLuint fractalCube_timeLocation = glGetUniformLocation(fractalCube_shader.program, "Time");
    GLuint fractalCube_diffuseLocation = glGetUniformLocation(fractalCube_shader.program, "Diffuse");
    GLuint fractalCube_specLocation = glGetUniformLocation(fractalCube_shader.program, "Spec");

    // SKYBOX SHADER
    ShaderGLSL skybox_shader;
    const char * shaderFileSkybox = "src/skybox.glsl";
    //int status = load_shader_from_file(gbuffer_shader, shaderFileGBuffer, ShaderGLSL::VERTEX_SHADER | ShaderGLSL::FRAGMENT_SHADER | ShaderGLSL::GEOMETRY_SHADER);
    status = load_shader_from_file(skybox_shader, shaderFileSkybox, ShaderGLSL::VERTEX_SHADER | ShaderGLSL::FRAGMENT_SHADER);
    if ( status == -1 )
    {
        fprintf(stderr, "Error on loading  %s\n", shaderFileSkybox);
        exit( EXIT_FAILURE );
    }    

    // Compute locations for skybox_shader
    GLuint skybox_projectionLocation = glGetUniformLocation(skybox_shader.program, "Projection");
    GLuint skybox_viewLocation = glGetUniformLocation(skybox_shader.program, "View");
    GLuint skybox_objectLocation = glGetUniformLocation(skybox_shader.program, "Object");
    GLuint skybox_timeLocation = glGetUniformLocation(skybox_shader.program, "Time");
    GLuint skybox_cameraPositionLocation = glGetUniformLocation(skybox_shader.program, "CameraPosition");
    GLuint camera_inverseViewProjectionLocation = glGetUniformLocation(skybox_shader.program, "InverseViewProjection");


    // Load light accumulation shader
    ShaderGLSL vortexLighting_shader;
    const char * shaderFileVortexLighting = "src/vortexLight.glsl";
    //int status = load_shader_from_file(lighting_shader, shaderFileLighting, ShaderGLSL::VERTEX_SHADER | ShaderGLSL::FRAGMENT_SHADER | ShaderGLSL::GEOMETRY_SHADER);
    status = load_shader_from_file(vortexLighting_shader, shaderFileVortexLighting, ShaderGLSL::VERTEX_SHADER | ShaderGLSL::FRAGMENT_SHADER);
    if ( status == -1 )
    {
        fprintf(stderr, "Error on loading  %s\n", shaderFileVortexLighting);
        exit( EXIT_FAILURE );
    }    
    // Compute locations for lighting_shader
    GLuint vortexLighting_materialLocation = glGetUniformLocation(vortexLighting_shader.program, "Material");
    GLuint vortexLighting_normalLocation = glGetUniformLocation(vortexLighting_shader.program, "Normal");
    GLuint vortexLighting_depthLocation = glGetUniformLocation(vortexLighting_shader.program, "Depth");
    GLuint vortexLighting_inverseViewProjectionLocation = glGetUniformLocation(vortexLighting_shader.program, "InverseViewProjection");
    GLuint vortexLighting_cameraPositionLocation = glGetUniformLocation(vortexLighting_shader.program, "CameraPosition");
    GLuint vortexLighting_lightPositionLocation = glGetUniformLocation(vortexLighting_shader.program, "LightPosition");
    GLuint vortexLighting_lightColorLocation = glGetUniformLocation(vortexLighting_shader.program, "LightColor");
    GLuint vortexLighting_lightIntensityLocation = glGetUniformLocation(vortexLighting_shader.program, "LightIntensity");
    GLuint vortexLighting_skyboxLocation = glGetUniformLocation(vortexLighting_shader.program, "Skybox");
    GLuint vortexLighting_numLightLocation = glGetUniformLocation(vortexLighting_shader.program, "NumLight");



    // Load light accumulation shader
    ShaderGLSL universeLighting_shader;
    const char * shaderFileUniverseLighting = "src/universeLight.glsl";
    //int status = load_shader_from_file(lighting_shader, shaderFileLighting, ShaderGLSL::VERTEX_SHADER | ShaderGLSL::FRAGMENT_SHADER | ShaderGLSL::GEOMETRY_SHADER);
    status = load_shader_from_file(universeLighting_shader, shaderFileUniverseLighting, ShaderGLSL::VERTEX_SHADER | ShaderGLSL::FRAGMENT_SHADER);
    if ( status == -1 )
    {
        fprintf(stderr, "Error on loading  %s\n", shaderFileUniverseLighting);
        exit( EXIT_FAILURE );
    }    
    // Compute locations for lighting_shader
    GLuint universeLighting_materialLocation = glGetUniformLocation(universeLighting_shader.program, "Material");
    GLuint universeLighting_normalLocation = glGetUniformLocation(universeLighting_shader.program, "Normal");
    GLuint universeLighting_depthLocation = glGetUniformLocation(universeLighting_shader.program, "Depth");
    GLuint universeLighting_inverseViewProjectionLocation = glGetUniformLocation(universeLighting_shader.program, "InverseViewProjection");
    GLuint universeLighting_cameraPositionLocation = glGetUniformLocation(universeLighting_shader.program, "CameraPosition");
    GLuint universeLighting_lightPositionLocation = glGetUniformLocation(universeLighting_shader.program, "LightPosition");
    GLuint universeLighting_lightColorLocation = glGetUniformLocation(universeLighting_shader.program, "LightColor");
    GLuint universeLighting_lightIntensityLocation = glGetUniformLocation(universeLighting_shader.program, "LightIntensity");
    GLuint universeLighting_skyboxLocation = glGetUniformLocation(universeLighting_shader.program, "Skybox");
    GLuint universeLighting_numLightLocation = glGetUniformLocation(universeLighting_shader.program, "NumLight");
    GLuint universeLighting_timeLocation = glGetUniformLocation(universeLighting_shader.program, "Time");


    // Load geometry
    int   cube_triangleCount = 12;
    int   cube_triangleList[] = {0, 1, 2, 2, 1, 3,
                                 4, 5, 6, 6, 5, 7,
                                 8, 9, 10, 10, 9, 11,
                                 12, 13, 14, 14, 13, 15,
                                 16, 17, 18, 19, 17, 20,
                                 21, 22, 23, 24, 25, 26};

    float cube_uvs[] = {0.f, 0.f, 0.f, 1.f, 1.f, 0.f, 1.f, 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f, 1.f, 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f, 1.f, 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f, 1.f, 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f,  1.f, 0.f,  1.f, 1.f,  0.f, 1.f,  1.f, 1.f,  0.f, 0.f, 0.f, 0.f, 1.f, 1.f,  1.f, 0.f,  };
    float cube_vertices[] = {-0.5, -0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0.5, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0.5 };
    float cube_normals[] = {0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, };

    int   quad_triangleCount = 2;
    int   quad_triangleList[] = {0, 1, 2, 2, 1, 3}; 
    float quad_vertices[] =  {-1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0};


    int   star_triangleCount = 8;
    int   star_triangleList[] = {0, 1, 2, 2, 3, 4, 4, 5, 6, 6, 7, 8, 8, 9, 1, 1, 8, 6, 6, 1, 2, 2, 4, 6}; 
   float  littleStar_vertices[] = {
                             0.0,     0.01,
                             0.0025, 0.0025,
                            -0.0025, 0.0025,
                            -0.01,   0.0025,
                            -0.004,  -0.002,
                            -0.006,  -0.01,
                             0.00,   -0.005,
                             0.006,  -0.01,
                             0.004,  -0.002,
                             0.01,   0.0025
                            };

                    float star_vertices[] = {
                             0.0,     0.05,
                             0.0125, 0.0125,
                            -0.0125, 0.0125,
                            -0.05,   0.0125,
                            -0.02,  -0.01,
                            -0.03,  -0.05,
                             0.00,   -0.025,
                             0.03,  -0.05,
                             0.020,  -0.01,
                             0.05,   0.0125
                            };

float star_normals[] = {0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1};
float spacebox_vertices[] = {-0.5, -0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0.5, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0.5 };

    for(unsigned int i = 0; i<sizeof(cube_vertices); ++i){
        spacebox_vertices[i] = cube_vertices[i]*100000;
        cube_vertices[i] = cube_vertices[i]*0.05;
    }




    // Vertex Array Object
    GLuint vao[5];
    glGenVertexArrays(5, vao);

    // Vertex Buffer Objects
    GLuint vbo[16];
    glGenBuffers(16, vbo);

   glBindVertexArray(vao[0]);
    // Bind indices and upload data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_triangleList), cube_triangleList, GL_STATIC_DRAW);
    // Bind vertices and upload data
     glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*3, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(spacebox_vertices), spacebox_vertices, GL_STATIC_DRAW);
    // Bind normals and upload data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*3, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_normals), cube_normals, GL_STATIC_DRAW);
    // Bind uv coords and upload data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_uvs), cube_uvs, GL_STATIC_DRAW);


    // Cube
    glBindVertexArray(vao[1]);
    // Bind indices and upload data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[4]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_triangleList), cube_triangleList, GL_STATIC_DRAW);
    // Bind vertices and upload data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*3, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
    // Bind normals and upload data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[6]);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*3, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_normals), cube_normals, GL_STATIC_DRAW);
    // Bind uv coords and upload data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_uvs), cube_uvs, GL_STATIC_DRAW);



    //little starts
    glBindVertexArray(vao[2]);
    // Bind indices and upload data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[8]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(star_triangleList), star_triangleList, GL_STATIC_DRAW);
    // Bind vertices and upload data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[9]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(littleStar_vertices), littleStar_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[10]);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*3, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(star_normals), star_normals, GL_STATIC_DRAW);

    // Quad
    glBindVertexArray(vao[3]);
    // Bind indices and upload data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[11]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_triangleList), quad_triangleList, GL_STATIC_DRAW);
    // Bind vertices and upload data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[12]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);


     // Stars
    glBindVertexArray(vao[4]);
    // Bind indices and upload data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[13]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(star_triangleList), star_triangleList, GL_STATIC_DRAW);
    // Bind vertices and upload data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[14]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(star_vertices), star_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[15]);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*3, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(star_normals), star_normals, GL_STATIC_DRAW);


    // Unbind everything. Potentially illegal on some implementations
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);




    // Init frame buffers
    GLuint gbufferFbo;
    GLuint gbufferTextures[3];
    GLuint gbufferDrawBuffers[2];
    glGenTextures(3, gbufferTextures);

    // Create color texture
    glBindTexture(GL_TEXTURE_2D, gbufferTextures[0]);
   // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Create normal texture
    glBindTexture(GL_TEXTURE_2D, gbufferTextures[1]);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Create depth texture
    glBindTexture(GL_TEXTURE_2D, gbufferTextures[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Create Framebuffer Object
    glGenFramebuffers(1, &gbufferFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, gbufferFbo);

    // Attach textures to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D, gbufferTextures[0], 0);
    gbufferDrawBuffers[0] = GL_COLOR_ATTACHMENT0;
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1 , GL_TEXTURE_2D, gbufferTextures[1], 0);
    gbufferDrawBuffers[1] = GL_COLOR_ATTACHMENT1;
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gbufferTextures[2], 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        fprintf(stderr, "Error on building framebuffer\n");
        exit( EXIT_FAILURE );
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);










    GLuint effectsFbo;
    GLuint effectsTextures[2];
    glGenTextures(2, effectsTextures);
    // Create color texture
    glBindTexture(GL_TEXTURE_2D, effectsTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Create normal texture
    glBindTexture(GL_TEXTURE_2D, effectsTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &effectsFbo);


    int numLight=0;

    int mousex = 0;
    int mousey = 0;
    int diffLockPositionX = 0;
    int diffLockPositionY = 0;

    do
    {
        t = glfwGetTime();

        // Mouse states
        int leftButton = glfwGetMouseButton( GLFW_MOUSE_BUTTON_LEFT );
        int rightButton = glfwGetMouseButton( GLFW_MOUSE_BUTTON_RIGHT );
        int middleButton = glfwGetMouseButton( GLFW_MOUSE_BUTTON_MIDDLE );

        int up = glfwGetKey(GLFW_KEY_UP);
        int down = glfwGetKey(GLFW_KEY_DOWN);
        int right = glfwGetKey(GLFW_KEY_RIGHT);
        int left = glfwGetKey(GLFW_KEY_LEFT);
    
        if( leftButton == GLFW_PRESS )
            guiStates.turnLock = true;
        else
            guiStates.turnLock = false;

        if( rightButton == GLFW_PRESS )
            guiStates.zoomLock = true;
        else
            guiStates.zoomLock = false;

        if( middleButton == GLFW_PRESS )
            guiStates.panLock = true;
        else
            guiStates.panLock = false;




        if( up == GLFW_PRESS ){
               camera.radius -= 0.1 ;
                if (camera.radius < 0.1)
                {
                    camera.radius = 10.f;
                    camera.o = camera.eye + glm::normalize(camera.o - camera.eye) * camera.radius;
                }
                camera_compute(camera);
        }

        if( down == GLFW_PRESS ){
               camera.radius += 0.1 ;
                if (camera.radius < 0.1)
                {
                    camera.radius = 10.f;
                    camera.o = camera.eye + glm::normalize(camera.o - camera.eye) * camera.radius;
                }
                camera_compute(camera);
        }

        if( left == GLFW_PRESS ){
            glm::vec3 up(0.f, camera.phi < M_PI ?1.f:-1.f, 0.f);
            glm::vec3 fwd = glm::normalize(camera.o - camera.eye);
            glm::vec3 side = glm::normalize(glm::cross(fwd, up));
            camera.up = glm::normalize(glm::cross(side, fwd));
            camera.o[0] -= side[0] * 0.002 * camera.radius * 2;
            camera.o[1] -= side[1] * 0.002 * camera.radius * 2;
            camera.o[2] -= side[2] * 0.002 * camera.radius * 2;       
            camera_compute(camera);
        }

        if( right == GLFW_PRESS ){
            glm::vec3 up(0.f, camera.phi < M_PI ?1.f:-1.f, 0.f);
            glm::vec3 fwd = glm::normalize(camera.o - camera.eye);
            glm::vec3 side = glm::normalize(glm::cross(fwd, up));
            camera.up = glm::normalize(glm::cross(side, fwd));
            camera.o[0] -= side[0] * -0.002 * camera.radius * 2;
            camera.o[1] -= side[1] * -0.002 * camera.radius * 2;
            camera.o[2] -= side[2] * -0.002 * camera.radius * 2;       
           camera_compute(camera);
        }

        // Camera movements
        int altPressed = glfwGetKey(GLFW_KEY_LSHIFT);
     //  if (!altPressed && (leftButton == GLFW_PRESS || rightButton == GLFW_PRESS || middleButton == GLFW_PRESS))
      //  {
           /* int x; int y;
            glfwGetMousePos(&x, &y);
            guiStates.lockPositionX = x;
            guiStates.lockPositionY = y;*/
       // }
        //if (altPressed = GLFW_PRESS)
        //{
            glfwGetMousePos(&mousex, &mousey);
            diffLockPositionX = mousex - guiStates.lockPositionX;
            diffLockPositionY = mousey - guiStates.lockPositionY;
            if (guiStates.zoomLock)
            {
                float zoomDir = 0.0;
                if (diffLockPositionX > 0)
                    zoomDir = -1.f;
                else if (diffLockPositionX < 0 )
                    zoomDir = 1.f;
                camera_zoom(camera, zoomDir * GUIStates::MOUSE_ZOOM_SPEED);
            }
            else if (guiStates.turnLock)
            {
                camera_turn(camera, diffLockPositionY * GUIStates::MOUSE_TURN_SPEED,
                            diffLockPositionX * GUIStates::MOUSE_TURN_SPEED);

            }
            else if (guiStates.panLock)
            {
                camera_pan(camera, diffLockPositionX * GUIStates::MOUSE_PAN_SPEED,
                            diffLockPositionY * GUIStates::MOUSE_PAN_SPEED);
            }
            guiStates.lockPositionX = mousex;
            guiStates.lockPositionY = mousey;
  //      }
  
        // Get camera matrices
        glm::mat4 projection = glm::perspective(45.0f, widthf / heightf, 0.1f, 1000.f); 
        glm::mat4 worldToView = glm::lookAt(camera.eye, camera.o, camera.up);
        glm::mat4 objectToWorld;
        glm::mat4 worldToScreen = projection * worldToView;
        glm::mat4 screenToWorld = glm::transpose(glm::inverse(worldToScreen));


        //SPACEBOX
        glBindFramebuffer(GL_FRAMEBUFFER, effectsFbo);
        //Attach textures to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D, effectsTextures[0], 0);
        glViewport( 0, 0, width, height );
        // Clear the front buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Bind gbuffer shader
        glUseProgram(skybox_shader.program);
        // Upload uniforms
        glUniformMatrix4fv(skybox_projectionLocation, 1, 0, glm::value_ptr(projection));
        glUniformMatrix4fv(skybox_viewLocation, 1, 0, glm::value_ptr(worldToView));
        glUniformMatrix4fv(skybox_objectLocation, 1, 0, glm::value_ptr(objectToWorld));
        glUniform1f(skybox_timeLocation, t);
        glUniform3fv(skybox_cameraPositionLocation, 1, glm::value_ptr(camera.eye));
        glUniformMatrix4fv(camera_inverseViewProjectionLocation, 1, 0, glm::value_ptr(screenToWorld));
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(vao[0]);
        for(int i=0; i<6; ++i){
              if(VORTEX == 1)
                 glBindTexture(GL_TEXTURE_2D, timeVortexTextures[i]);
             else
                 glBindTexture(GL_TEXTURE_2D, spaceboxTextures[i]);
             glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(i*cube_triangleCount*2));
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);



        glBindFramebuffer(GL_FRAMEBUFFER, gbufferFbo);
        glDrawBuffers(2, gbufferDrawBuffers);

            // Viewport 
           glViewport( 0, 0, width, height  );
            // Clear the front buffer
           glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Default states
            glEnable(GL_DEPTH_TEST);

            // Bind gbuffer shader
            glUseProgram(starsVortex_shader.program);
            // Upload uniforms
            glUniformMatrix4fv(starsVortex_projectionLocation, 1, 0, glm::value_ptr(projection));
            glUniformMatrix4fv(starsVortex_viewLocation, 1, 0, glm::value_ptr(worldToView));
            glUniformMatrix4fv(starsVortex_objectLocation, 1, 0, glm::value_ptr(objectToWorld));
            glUniform1f(starsVortex_timeLocation, t);
            glUniform1i(starsVortex_diffuseLocation, 0);
            glUniform1i(starsVortex_specLocation, 1);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textures[0]);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, textures[1]); 

            if(VORTEX == 1){
                //Vortex Etoile
                glBindVertexArray(vao[4]);
                glDrawElementsInstanced(GL_TRIANGLES, star_triangleCount * 3, GL_UNSIGNED_INT, (void*)0, 2000);
            }
            else if(UNIVERSE == 1){
                //Vortex Etoile
                glBindVertexArray(vao[4]);
                glDrawElementsInstanced(GL_TRIANGLES, star_triangleCount * 3, GL_UNSIGNED_INT, (void*)0, 2000);

                // Bind gbuffer shader
                glUseProgram(starsSphere_shader.program);
                // Upload uniforms
                glUniformMatrix4fv(starsSphere_projectionLocation, 1, 0, glm::value_ptr(projection));
                glUniformMatrix4fv(starsSphere_viewLocation, 1, 0, glm::value_ptr(worldToView));
                glUniformMatrix4fv(starsSphere_objectLocation, 1, 0, glm::value_ptr(objectToWorld));
                glUniform1f(starsSphere_timeLocation, t);
                glUniform1i(starsSphere_diffuseLocation, 0);
                glUniform1i(starsSphere_specLocation, 1);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, textures[0]);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, textures[1]); 

                //Sphere etoile
                glBindVertexArray(vao[2]);
                glDrawElementsInstanced(GL_TRIANGLES, star_triangleCount * 3, GL_UNSIGNED_INT, (void*)0, 10000);



                //Sphere Anneaux Cubique
                // Bind cube shader
                glUseProgram(cube_shader.program);
                // Upload uniforms
                glUniformMatrix4fv(cube_projectionLocation, 1, 0, glm::value_ptr(projection));
                glUniformMatrix4fv(cube_viewLocation, 1, 0, glm::value_ptr(worldToView));
                glUniformMatrix4fv(cube_objectLocation, 1, 0, glm::value_ptr(objectToWorld));
                glUniform1f(cube_timeLocation, t);
                glUniform1i(cube_diffuseLocation, 0);
                glUniform1i(cube_specLocation, 1);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, textures[0]);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, textures[1]); 

                glBindVertexArray(vao[1]);
                glDrawElementsInstanced(GL_TRIANGLES, cube_triangleCount * 3, GL_UNSIGNED_INT, (void*)0, 900);

                // Bind cube shader
                glUseProgram(fractalCube_shader.program);
                // Upload uniforms
                glUniformMatrix4fv(fractalCube_projectionLocation, 1, 0, glm::value_ptr(projection));
                glUniformMatrix4fv(fractalCube_viewLocation, 1, 0, glm::value_ptr(worldToView));
                glUniformMatrix4fv(fractalCube_objectLocation, 1, 0, glm::value_ptr(objectToWorld));
                glUniform1f(fractalCube_timeLocation, t);
                glUniform1i(fractalCube_diffuseLocation, 0);
                glUniform1i(fractalCube_specLocation, 1);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, textures[0]);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, textures[1]); 

                glBindVertexArray(vao[1]);
                glDrawElementsInstanced(GL_TRIANGLES, cube_triangleCount * 3, GL_UNSIGNED_INT, (void*)0, 2000);

            }
            // Unbind framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, 0);




           if(VORTEX == 1){
                glBindFramebuffer(GL_FRAMEBUFFER, effectsFbo);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D, effectsTextures[1], 0);

                // Viewport 
                glViewport( 0, 0, width, height );
                // Clear the front buffer
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                // Bind lighting shader
                glUseProgram(vortexLighting_shader.program);
                // Upload uniforms
                glUniform1i(vortexLighting_materialLocation, 0);
                glUniform1i(vortexLighting_normalLocation, 1);
                glUniform1i(vortexLighting_depthLocation, 2);
                glUniform1i(vortexLighting_skyboxLocation, 3);
                glUniform3fv(vortexLighting_cameraPositionLocation, 1, glm::value_ptr(camera.eye));
                glUniformMatrix4fv(vortexLighting_inverseViewProjectionLocation, 1, 0, glm::value_ptr(screenToWorld));

                // Bind color to unit 0
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, gbufferTextures[0]);        
                // Bind normal to unit 1
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, gbufferTextures[1]);    
                // Bind depth to unit 2
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, gbufferTextures[2]);       

                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, effectsTextures[0]);    

                // Blit above the rest
                glDisable(GL_DEPTH_TEST);

               glEnable(GL_BLEND);
               glBlendFunc(GL_ONE, GL_ONE);

                for (int i = 0; i < numLights; ++i)
                {
                     glUniform1i(vortexLighting_numLightLocation, numLight);
                    ++numLight;

                    float tl = t*0.5 * i;
                    float lightIntensity = 10.;
                    float lightPosition[3] = { 0., 0., -i*3+13};
                    float lightColor[3] = { sinf(tl) *  1.1, 0.7 - cosf(tl), 1 -sinf(tl)};

                    glUniform3fv(vortexLighting_lightPositionLocation, 1, lightPosition);
                    glUniform3fv(vortexLighting_lightColorLocation, 1, lightColor);
                    glUniform1f(vortexLighting_lightIntensityLocation, lightIntensity);

                    // Draw quad
                    glBindVertexArray(vao[3]);
                    glDrawElements(GL_TRIANGLES, quad_triangleCount * 3, GL_UNSIGNED_INT, (void*)0);
                }

                glDisable(GL_BLEND);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                numLight=0;
           }






            if(UNIVERSE == 1){
                glBindFramebuffer(GL_FRAMEBUFFER, effectsFbo);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D, effectsTextures[1], 0);

                // Viewport 
               glViewport( 0, 0, width, height );
                // Clear the front buffer
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


                // Bind lighting shader
                glUseProgram(universeLighting_shader.program);
                // Upload uniforms
                glUniform1i(universeLighting_materialLocation, 0);
                glUniform1i(universeLighting_normalLocation, 1);
                glUniform1i(universeLighting_depthLocation, 2);
                glUniform1i(universeLighting_skyboxLocation, 3);
                glUniform3fv(universeLighting_cameraPositionLocation, 1, glm::value_ptr(camera.eye));
                glUniformMatrix4fv(universeLighting_inverseViewProjectionLocation, 1, 0, glm::value_ptr(screenToWorld));
                glUniform1i(universeLighting_timeLocation, t);
                // Bind color to unit 0
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, gbufferTextures[0]);        
                // Bind normal to unit 1
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, gbufferTextures[1]);    
                // Bind depth to unit 2
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, gbufferTextures[2]);       

                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, effectsTextures[0]);    

                // Blit above the rest
                glDisable(GL_DEPTH_TEST);

               glEnable(GL_BLEND);
               glBlendFunc(GL_ONE, GL_ONE);

                for (int i = 0; i < numLights; ++i)
                {
                     glUniform1i(universeLighting_numLightLocation, numLight);
                    ++numLight;

                    float lightIntensity = 6;
                    float lightPosition[3] = {0., 0., 0.};
                    float lightColor[3] = {0., 0., 0.};


                    //CUBE FRACTAL
                    if(i==0){
                        lightPosition[0] = 19.3;
                        lightPosition[1] = 22.9;
                        lightPosition[2] = 2.1;
                        lightColor[0] = 0.1;
                        lightColor[1] = 0.5;
                        lightColor[2] = 0.7;
                        lightIntensity = 4;
                    }

                    //SPHERE ETOILE
                    if(i==1){
                        lightPosition[0] = 0.;
                        lightPosition[1] = 0.;
                        lightPosition[2] = 0.;
                        lightColor[0] = 0.4;
                        lightColor[1] = 0.6;
                        lightColor[2] = 0.88;
                    }
                    else if(i==2){
                        lightPosition[0] = 0.;
                        lightPosition[1] = 0.;
                        lightPosition[2] = 4.7;
                        lightColor[0] = 0.61;
                        lightColor[1] = 0.29;
                        lightColor[2] = 0.42;
                    }

                    // VORTEX TEMPOREL
                    if(i==3){
                        lightPosition[0] = 0.;
                        lightPosition[1] = 0.;
                        lightPosition[2] = 40+cos(t)*20;
                        lightColor[0] = 0.2;
                        lightColor[1] = 0.9*cos(t);
                        lightColor[2] = 0.5;
                    }
                    else if(i==4){
                     lightPosition[0] = 0.;
                        lightPosition[1] = 0.;
                        lightPosition[2] = 40+cos(t*2)*20;
                        lightColor[0] = 0.8*cos(t);
                        lightColor[1] = 0.4;
                        lightColor[2] = 0.3;
                    }
                    else if(i==5){
                        lightPosition[0] = 0.;
                        lightPosition[1] = 0.;
                        lightPosition[2] = 40+cos(t*3)*20;
                        lightColor[0] = 0.5;
                        lightColor[1] = 0.1;
                        lightColor[2] = 0.7*cos(t);
                    }

                    else if(i==6){
                        lightPosition[0] = 0.;
                        lightPosition[1] = 0.;
                        lightPosition[2] = 40+cos(t*4)*20;
                        lightColor[0] = 0.6;
                        lightColor[1] = 0.3;
                        lightColor[2] = 0.4;
                    }


                    else if(i==7){
                        lightPosition[0] = 0.;
                        lightPosition[1] = 0.;
                        lightPosition[2] = 40+cos(t*5)*20;
                        lightColor[0] = 0.2;
                        lightColor[1] = 0.3;
                        lightColor[2] = 1*cos(t);
                    }



                    glUniform3fv(universeLighting_lightPositionLocation, 1, lightPosition);
                    glUniform3fv(universeLighting_lightColorLocation, 1, lightColor);
                    glUniform1f(universeLighting_lightIntensityLocation, lightIntensity);

                    // Draw quad
                   glBindVertexArray(vao[3]);
                   glDrawElements(GL_TRIANGLES, quad_triangleCount * 3, GL_UNSIGNED_INT, (void*)0);
                }
  

                glDisable(GL_BLEND);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                numLight=0;
           }



        glBindFramebuffer(GL_FRAMEBUFFER, effectsFbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D, effectsTextures[0], 0);

        //Blur
        // Clear the front buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport( 0, 0, width, height );

        // Bind lighting shader
        glUseProgram(blur_shader.program);
        // Upload uniforms
        glUniform1i(blur_tex1Location, 0);
        glUniform1f(blur_timeLocation, t);

        // Bind color to unit 0
        glActiveTexture(GL_TEXTURE0); // pas besoin de le refaire car on l'a fait au niveau de sobel
        glBindTexture(GL_TEXTURE_2D, effectsTextures[1]);           
        // Draw quad
        glBindVertexArray(vao[3]);
        glDrawElements(GL_TRIANGLES, quad_triangleCount * 3, GL_UNSIGNED_INT, (void*)0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //GAMMA
        // Clear the front buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport( 0, 0, width, height );

        // Bind lighting shader
        glUseProgram(gamma_shader.program);
        // Upload uniforms
        glUniform1i(gamma_tex1Location, 0);
        glUniform1f(gamma_timeLocation, t);

        // Bind color to unit 0
        glActiveTexture(GL_TEXTURE0); // pas besoin de le refaire car on l'a fait au niveau de sobel
        glBindTexture(GL_TEXTURE_2D, effectsTextures[0]);           
        // Draw quad
        glBindVertexArray(vao[3]);
        glDrawElements(GL_TRIANGLES, quad_triangleCount * 3, GL_UNSIGNED_INT, (void*)0);



        if(t>22.8 && UNIVERSE==0){
                VORTEX = 0;
                UNIVERSE = 1;
                numLights = 8;
                glDeleteTextures(6, timeVortexTextures);
                    for(unsigned int i = 0; i<sizeof(cube_vertices); ++i){
                      spacebox_vertices[i] = cube_vertices[i]*1000;//00;//pour VORTEX !
                     }
                            glBindVertexArray(vao[0]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*3, (void*)0);
        glBufferData(GL_ARRAY_BUFFER, sizeof(spacebox_vertices), spacebox_vertices, GL_STATIC_DRAW);
         glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        }
        
        // Check for errors
        GLenum err = glGetError();
        if(err != GL_NO_ERROR)
        {
            fprintf(stderr, "OpenGL Error : %s\n", gluErrorString(err));
            
        }

        // Swap buffers
        glfwSwapBuffers();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS &&
           glfwGetWindowParam( GLFW_OPENED ) );

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    exit( EXIT_SUCCESS );
}




int  compile_and_link_shader(ShaderGLSL & shader, int typeMask, const char * sourceBuffer, int bufferSize)
{
    // Create program object
    shader.program = glCreateProgram();
    
    //Handle Vertex Shader
    GLuint vertexShaderObject ;
    if (typeMask & ShaderGLSL::VERTEX_SHADER)
    {
        // Create shader object for vertex shader
        vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
        // Add #define VERTEX to buffer
        const char * sc[3] = { "#version 150\n", "#define VERTEX\n", sourceBuffer};
        glShaderSource(vertexShaderObject, 
                       3, 
                       sc,
                       NULL);
        // Compile shader
        glCompileShader(vertexShaderObject);

        // Get error log size and print it eventually
        int logLength;
        glGetShaderiv(vertexShaderObject, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 1)
        {
            char * log = new char[logLength];
            glGetShaderInfoLog(vertexShaderObject, logLength, &logLength, log);
            fprintf(stderr, "Error in compiling vertex shader : %s", log);
            fprintf(stderr, "%s\n%s\n%s", sc[0], sc[1], sc[2]);
            delete[] log;
        }
        // If an error happend quit
        int status;
        glGetShaderiv(vertexShaderObject, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE)
            return -1;          

        //Attach shader to program
        glAttachShader(shader.program, vertexShaderObject);
    }

    // Handle Geometry shader
    GLuint geometryShaderObject ;
    if (typeMask & ShaderGLSL::GEOMETRY_SHADER)
    {
        // Create shader object for Geometry shader
        geometryShaderObject = glCreateShader(GL_GEOMETRY_SHADER);
        // Add #define Geometry to buffer
        const char * sc[3] = { "#version 150\n", "#define GEOMETRY\n", sourceBuffer};
        glShaderSource(geometryShaderObject, 
                       3, 
                       sc,
                       NULL);
        // Compile shader
        glCompileShader(geometryShaderObject);

        // Get error log size and print it eventually
        int logLength;
        glGetShaderiv(geometryShaderObject, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 1)
        {
            char * log = new char[logLength];
            glGetShaderInfoLog(geometryShaderObject, logLength, &logLength, log);
            fprintf(stderr, "Error in compiling Geometry shader : %s \n", log);
            fprintf(stderr, "%s\n%s\n%s", sc[0], sc[1], sc[2]);
            delete[] log;
        }
        // If an error happend quit
        int status;
        glGetShaderiv(geometryShaderObject, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE)
            return -1;          

        //Attach shader to program
        glAttachShader(shader.program, geometryShaderObject);
    }


    // Handle Fragment shader
    GLuint fragmentShaderObject ;
    if (typeMask && ShaderGLSL::FRAGMENT_SHADER)
    {
        // Create shader object for fragment shader
        fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
        // Add #define fragment to buffer
        const char * sc[3] = { "#version 150\n", "#define FRAGMENT\n", sourceBuffer};
        glShaderSource(fragmentShaderObject, 
                       3, 
                       sc,
                       NULL);
        // Compile shader
        glCompileShader(fragmentShaderObject);

        // Get error log size and print it eventually
        int logLength;
        glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 1)
        {
            char * log = new char[logLength];
            glGetShaderInfoLog(fragmentShaderObject, logLength, &logLength, log);
            fprintf(stderr, "Error in compiling fragment shader : %s \n", log);
            fprintf(stderr, "%s\n%s\n%s", sc[0], sc[1], sc[2]);
            delete[] log;
        }
        // If an error happend quit
        int status;
        glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE)
            return -1;          

        //Attach shader to program
        glAttachShader(shader.program, fragmentShaderObject);
    }


    // Bind attribute location
    glBindAttribLocation(shader.program,  0,  "VertexPosition");
    glBindAttribLocation(shader.program,  1,  "VertexNormal");
    glBindAttribLocation(shader.program,  2,  "VertexTexCoord");
    glBindFragDataLocation(shader.program, 0, "Color");
    glBindFragDataLocation(shader.program, 1, "Normal");

    // Link attached shaders
    glLinkProgram(shader.program);

    // Clean
    if (typeMask & ShaderGLSL::VERTEX_SHADER)
    {
        glDeleteShader(vertexShaderObject);
    }
    if (typeMask && ShaderGLSL::GEOMETRY_SHADER)
    {
        glDeleteShader(fragmentShaderObject);
    }
    if (typeMask && ShaderGLSL::FRAGMENT_SHADER)
    {
        glDeleteShader(fragmentShaderObject);
    }

    // Get link error log size and print it eventually
    int logLength;
    glGetProgramiv(shader.program, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 1)
    {
        char * log = new char[logLength];
        glGetProgramInfoLog(shader.program, logLength, &logLength, log);
        fprintf(stderr, "Error in linking shaders : %s \n", log);
        delete[] log;
    }
    int status;
    glGetProgramiv(shader.program, GL_LINK_STATUS, &status);        
    if (status == GL_FALSE)
        return -1;


    return 0;
}

int  destroy_shader(ShaderGLSL & shader)
{
    glDeleteProgram(shader.program);
    shader.program = 0;
    return 0;
}

int load_shader_from_file(ShaderGLSL & shader, const char * path, int typemask)
{
    int status;
    FILE * shaderFileDesc = fopen( path, "rb" );
    if (!shaderFileDesc)
        return -1;
    fseek ( shaderFileDesc , 0 , SEEK_END );
    long fileSize = ftell ( shaderFileDesc );
    rewind ( shaderFileDesc );
    char * buffer = new char[fileSize + 1];
    fread( buffer, 1, fileSize, shaderFileDesc );
    buffer[fileSize] = '\0';
    status = compile_and_link_shader( shader, typemask, buffer, fileSize );
    delete[] buffer;
    return status;
}



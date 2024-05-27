#include "render.h"
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned int lade_shader() {

  /**Hier ist der Code für den Vertex Shader.
   Dieser Shader sorgt dafür, dass die übergebenen Punkte und Kanten
   zusammen gesetzt werden.**/
  const char* vertex_shader_code =
                "#version 460 core\n"
                "layout (location = 0) in vec2 aPos;\n"
                "layout (location = 1) in vec4 farbe;\n"
                "layout (location = 2) in vec2 texpos;\n"
                "layout (location = 3) in int i_texindex;\n"
                "out vec2 f_texpos;\n"
                "out vec4 vertexColor;\n"
                "out int texindex;\n"
                "uniform vec2 groesze;\n"
                "void main()\n"
                "{\n"
                "    vertexColor = farbe;\n"
                "    f_texpos = texpos;\n"
                "    texindex = i_texindex;\n"
                "    gl_Position = vec4(aPos * groesze, 0.0, 1.0);\n"
                "}\n";
  /**Hier ist der Code für den Fragment Shader.
    Der Fragment Shader wandelt die Informationen des Vertex Shaders in die Pixel um,
    die auf dem Bildschirm angezeigt werden.**/
  const char* fragment_shader_code =
                "#version 460 core\n"
                "in vec2 f_texpos;\n"
                "in vec4 vertexColor;\n"
                "flat in int texindex;\n"
                "out vec4 FragColor;\n"
                "uniform sampler2D texturen[5];\n"
                "void main()\n"
                "{\n"
                    "//FragColor = vec4(0.5 * texindex, 1.0, 1.0, 1.0);\n"
                    "FragColor = texture(texturen[texindex], f_texpos) * vertexColor;\n"
                "}\n";


    /* FILE *vertexShaderFile; */
    /* char* vertexShaderSource; */

    /* Read File to get size */
    /* vertexShaderFile = fopen("/home/ole/CLionProjects/test/vertex.glsl", "rb"); */
    /* long size = 0; */
    /* if (vertexShaderFile == NULL) { */
    /*     printf("error\n"); */
    /* } */
    /* fseek(vertexShaderFile, 0L, SEEK_END); */
    /* size = ftell(vertexShaderFile)+1; */
    /* fclose(vertexShaderFile); */

    /* Read File for Content */
    /* vertexShaderFile = fopen("/home/ole/CLionProjects/test/vertex.glsl", "r"); */
    /* vertexShaderSource = memset(malloc(size), '\0', size); */
    /* fread(vertexShaderSource, 1, size-1, vertexShaderFile); */
    /* fclose(vertexShaderFile); */

    //kreire ein neues leeres shader programm
    unsigned int shader_program = glCreateProgram();
    //leerer vertex shader
    unsigned int vertex_shader;
    //shader typ
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    //code für den vertex shader
    glShaderSource(vertex_shader, 1, &vertex_shader_code, NULL);
    //shader kompilieren
    glCompileShader(vertex_shader);
    //vertex shader dem shader programm zuweisen
    glAttachShader(shader_program, vertex_shader);

    //das gleiche wie beim vertex shader
    unsigned int fragment_shader;
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_code, NULL);
    glCompileShader(fragment_shader);
    glAttachShader(shader_program, fragment_shader);

    //shader programm aktivieren
    glLinkProgram(shader_program);

    //glDeleteShader(vertex_shader);
    //glDeleteShader(fragment_shader);


    GLint compileStatus;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE) {
        GLint infoLogLength;
        glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &infoLogLength);
        GLchar *infoLog = (GLchar *)malloc(infoLogLength * sizeof(GLchar));
        glGetShaderInfoLog(vertex_shader, infoLogLength, NULL, infoLog);
        printf("Shader compilation error:\n%s\n", infoLog);
        free(infoLog);
    }

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE) {
        GLint infoLogLength;
        glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &infoLogLength);
        GLchar *infoLog = (GLchar *)malloc(infoLogLength * sizeof(GLchar));
        glGetShaderInfoLog(fragment_shader, infoLogLength, NULL, infoLog);
        printf("Shader compilation error:\n%s\n", infoLog);
        free(infoLog);
    }

    return shader_program;
}

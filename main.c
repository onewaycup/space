#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include "glad/include/glad/glad.h"
#define GLFW_INCLUDE_NONE
#include "glfw-3.4/include/GLFW/glfw3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN_ZOOM 0.000005f
#define G 10000000.0f

void readKeyboard(GLFWwindow *window);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

typedef struct {
    int64_t x;
    int64_t y;
    float zoom;
} Kamera;

Kamera kamera = {
    .x = 0,
    .y = 0,
    .zoom = 1.0f,
};

typedef struct {
  float position[2];
  float texcoord[2];
  float farbe[4];
} Knoten;

typedef struct {
  float cols[4][4];
} Matrix;
/*
x, y, durchmesser =^ 1 km
masse =^ 10^24 kg
*/
typedef struct {
    int64_t x;
    int64_t y;
    float durchmesser;
    unsigned int masse;
    float farbe[4];
} Planet;



static unsigned int lade_shader() {

  /**Hier ist der Code für den Vertex Shader.
   Dieser Shader sorgt dafür, dass die übergebenen Punkte und Kanten
   zusammen gesetzt werden.**/
  const char* vertex_shader_code =
                "#version 460 core\n"
                "layout (location = 0) in vec2 aPos; // the position variable has attribute position 0\n"
                "layout (location = 1) in vec4 farbe;\n"
                "out vec4 vertexColor; // specify a color output to the fragment shader\n"
                "uniform float groesze;"
                "void main()\n"
                "{\n"
                "    gl_Position = vec4(aPos * groesze, 0.0, 1.0); // see how we directly give a vec3 to vec4's constructor\n"
                "    vertexColor = farbe;\n"
                "}\n";
  /**Hier ist der Code für den Fragment Shader.
    Der Fragment Shader wandelt die Informationen des Vertex Shaders in die Pixel um,
    die auf dem Bildschirm angezeigt werden.**/
  const char* fragment_shader_code =
                "#version 460 core\n"
                "out vec4 FragColor;\n"
                "in vec4 vertexColor; // the input variable from the vertex shader (same name and same type)\n"
                "void main()\n"
                "{\n"
                    "FragColor = vertexColor;\n"
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

    return shader_program;
}


// void r_flush(BatchRenderer *renderer) {
//   if (renderer->vertex_count == 0) {
//     return;
//   }

//   glUseProgram(renderer->shader);

//   glActiveTexture(GL_TEXTURE0);
//   glBindTexture(GL_TEXTURE_2D, renderer->texture);

//   glUniform1i(glGetUniformLocation(renderer->shader, "u_texture"), 0);
//   glUniformMatrix4fv(glGetUniformLocation(renderer->shader, "u_mvp"), 1,
//                      GL_FALSE, renderer->mvp.cols[0]);

//   glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
//   glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Knoten) * renderer->vertex_count,
//                   renderer->vertices);

//   glBindVertexArray(renderer->vao);
//   glDrawArrays(GL_TRIANGLES, 0, renderer->vertex_count);

//   renderer->vertex_count = 0;
// }


// void r_push_vertex(BatchRenderer *renderer, float x, float y, float u,
//                    float v) {
//   if (renderer->vertex_count == renderer->objekte_anzahl) {
//     r_flush(renderer);
//   }

//   renderer->vertices[renderer->vertex_count++] = (Knoten){
//     .position = {x, y},
//     .texcoord = {u, v},
//   };
// }


// void r_texture(BatchRenderer *renderer, GLuint id) {
//   if (renderer->texture != id) {
//     r_flush(renderer);
//     renderer->texture = id;
//   }
// }

// void r_mvp(BatchRenderer *renderer, Matrix mat) {
//   if (memcmp(&renderer->mvp.cols, &mat.cols, sizeof(Matrix)) != 0) {
//     r_flush(renderer);
//     renderer->mvp = mat;
//   }
// }



int main(){
    printf("Start\n");

    //initialsiere glfw
    if (!glfwInit()) {
        printf("no glfw3 :(\n");
        return 1;
    }

    //setze die opengl version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    //spezifiziere opengl als grafik api
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    //glfwWindowHint(GLFW_POSITION_X, 100);

    //öffne das fenster: größe x; größe y; titel
    GLFWwindow  *window = glfwCreateWindow(1000, 1000, "space", NULL, NULL);
    if (!window) {
        printf("no window :(\n");
        return 1;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        printf("no glad :(\n");
        glfwTerminate();
        return 1;
    }

    glfwSetScrollCallback(window, scroll_callback);

    //wo soll opengl rendern
    glViewport(0, 0, 1000, 1000);

    //im vertex array objekt können mehrere Buffer gespeichert werden
    unsigned int vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    //Das Vertex Buffer Objekt ist eine Speicherplatzregion
    //auf der Grafikkarte, speziell im VRAM.
    //Buffer ID
    unsigned int vbo;
    //erstelle den Buffer und seine ID. Deshalb ist der Buffer ein Integer: anzahl der buffer; ort der buffer ID
    glGenBuffers(1, &vbo);
    //spezifiziere Buffer Typ: Buffer Typ; Buffer ID
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    //weise dem Buffer Daten zu: Buffer Typ; Buffer größe; Daten die von anfang an im Buffer sind; wie der Buffer genutzt wird
    glBufferData(GL_ARRAY_BUFFER, sizeof(Knoten) * 32 * 4, NULL, GL_DYNAMIC_DRAW);
    //aktiviere folgendes bei position 0
    glEnableVertexAttribArray(0);
    //Definiere wo die Position des Knotens im Speicher liegt:
    //Stelle Eingschaft; wie viele Variablen; Variablen Typ; soll die Variable umformatiert werden; Abstand zwischen den Knoten; Abstande vom Knoten Beginn bis zur Position im Speicher
    //glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Knoten), (const void*)offsetof(Knoten, position));
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Knoten), (const void*)offsetof(Knoten, position));
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Knoten), (const void*)offsetof(Knoten, farbe));

    unsigned int ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 6 * 32 * 4, NULL, GL_DYNAMIC_DRAW);

    //glEnableVertexAttribArray(1);
    //glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Knoten), (void *)offsetof(Knoten, texcoord));

    unsigned int shader = lade_shader();

    Planet planeten[] = {
        {.x = 0, .y = 0, .durchmesser = 12000, .masse = 100000, .farbe = {0.863, 0.925, 0.102, 1}},
        {.x = 100000, .y = 100000, .durchmesser = 5000, .masse = 1, .farbe = {0.965, 0.569, 0.867, 1}},
        {.x = -100000, .y = -100000, .durchmesser = 8000, .masse = 1, .farbe = {0.965, 0.569, 0.867, 1}},
        {.x = -100000, .y = 100000, .durchmesser = 20000, .masse = 1, .farbe = {0.965, 0.569, 0.867, 1}}
    };

    double deltaTime;
    double zeit_bild_beginn;
    double zeit_bild_ende = 0.0;
    

    //schleife in der auf den bildschirm gezeichnet wird
    while (!glfwWindowShouldClose(window)) {
        zeit_bild_beginn = glfwGetTime();
        glClearColor(0.2f, 0.2f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //spezifiziere den zu nutzenden shader
        glUseProgram(shader);
        readKeyboard(window);

        int planten_anzahl = sizeof(planeten) / sizeof(Planet);
        int knoten_anzahl = planten_anzahl * 4;
        int kanten_anzahl = planten_anzahl * 6;

        Knoten knoten[knoten_anzahl];
        int kanten[kanten_anzahl];

        //printf("%d\n", planten_anzahl);
        //printf("%d\n", knoten_anzahl);
        //printf("%d\n", kanten_anzahl);


        deltaTime = zeit_bild_ende - zeit_bild_beginn;
        //printf("%lf\n", deltaTime);

        float radius_q;
        float radius = 0;
        float delta_x;
        float delta_y;
        float beschleunigung = 0;
        float beschleunigung_x = 0;
        float beschleunigung_y = 0;

/*          for (int i = 0; i < planten_anzahl; i++) {
            for (int anderer_planet = 0; anderer_planet < planten_anzahl; anderer_planet++) {
                if (anderer_planet != i) {
                    delta_x = planeten[anderer_planet].x - planeten[i].x;
                    delta_y = planeten[anderer_planet].y - planeten[i].x;

                    radius_q = delta_x * delta_x + delta_y * delta_y;
                    radius = sqrtf(radius);

                    beschleunigung = G * (planeten[anderer_planet].masse / radius_q);

                    beschleunigung_x += beschleunigung * (delta_x / radius);
                    beschleunigung_y += beschleunigung * (delta_y / radius);
                }
            }

            printf("%f%s%f\n", beschleunigung_x, " ", beschleunigung_y);

            beschleunigung_x /= planten_anzahl;
            beschleunigung_y /= planten_anzahl;

            planeten[i].x += beschleunigung_x * deltaTime * deltaTime;
            planeten[i].y += beschleunigung_y * deltaTime * deltaTime;
        }*/

        for (int i = 1; i < planten_anzahl; i++) {
            delta_x = planeten[0].x - planeten[i].x;
            delta_y = planeten[0].y - planeten[i].x;
            //printf("%s%f%s%f\n", "delta x: ", delta_x, " delta y: ", delta_y);

            radius_q = delta_x * delta_x + delta_y * delta_y;
            //printf("%f\n", radius_q);
            radius = sqrtf(radius_q);
            //printf("%s%f%s%f\n", "radius_q: ", radius_q, " radius: ", radius);

            beschleunigung = G * planeten[0].masse / radius_q;

            beschleunigung_x += beschleunigung * tan(delta_x / radius);
            beschleunigung_y += beschleunigung * tan(delta_y / radius);
            //printf("%s%f%s%f%s%f\n", "a: ", beschleunigung, " ax: ", beschleunigung_x, " ay: ", beschleunigung_y);

            planeten[i].x += beschleunigung_x * deltaTime * deltaTime;
            planeten[i].y += beschleunigung_y * deltaTime * deltaTime;
        }

        for (int anderer_planet = 0; anderer_planet < planten_anzahl; anderer_planet++) {
            printf("%s%d%s%ld%s%ld\n", "planet ", anderer_planet, ": ", planeten[anderer_planet].x, " ", planeten[anderer_planet].y);
        }

        for (int i = 0; i < planten_anzahl; i++) {
            knoten[i * 4 + 0].position[0] = (planeten[i].x - kamera.x - planeten[i].durchmesser);
            knoten[i * 4 + 0].position[1] = (planeten[i].y - kamera.y - planeten[i].durchmesser);
            memcpy(knoten[i * 4 + 0].farbe, planeten[i].farbe, sizeof(float) * 4);
            // knoten[i * 4 + 0].farbe[0] = planeten[i].farbe[0];
            // knoten[i * 4 + 0].farbe[1] = planeten[i].farbe[1];
            // knoten[i * 4 + 0].farbe[2] = planeten[i].farbe[2];
            // knoten[i * 4 + 0].farbe[3] = planeten[i].farbe[3];

            knoten[i * 4 + 1].position[0] = (planeten[i].x - kamera.x + planeten[i].durchmesser);
            knoten[i * 4 + 1].position[1] = (planeten[i].y - kamera.y - planeten[i].durchmesser);
            memcpy(knoten[i * 4 + 1].farbe, planeten[i].farbe, sizeof(float) * 4);
            // knoten[i * 4 + 1].farbe[0] = planeten[i].farbe[0];
            // knoten[i * 4 + 1].farbe[1] = planeten[i].farbe[1];
            // knoten[i * 4 + 1].farbe[2] = planeten[i].farbe[2];
            // knoten[i * 4 + 1].farbe[3] = planeten[i].farbe[3];

            knoten[i * 4 + 2].position[0] = (planeten[i].x - kamera.x + planeten[i].durchmesser);
            knoten[i * 4 + 2].position[1] = (planeten[i].y - kamera.y + planeten[i].durchmesser);
            memcpy(knoten[i * 4 + 2].farbe, planeten[i].farbe, sizeof(float) * 4);

            // knoten[i * 4 + 2].farbe[0] = planeten[i].farbe[0];
            // knoten[i * 4 + 2].farbe[1] = planeten[i].farbe[1];
            // knoten[i * 4 + 2].farbe[2] = planeten[i].farbe[2];
            // knoten[i * 4 + 2].farbe[3] = planeten[i].farbe[3];

            knoten[i * 4 + 3].position[0] = (planeten[i].x - kamera.x - planeten[i].durchmesser);
            knoten[i * 4 + 3].position[1] = (planeten[i].y - kamera.y + planeten[i].durchmesser);
            memcpy(knoten[i * 4 + 3].farbe, planeten[i].farbe, sizeof(float) * 4);
            // knoten[i * 4 + 3].farbe[0] = planeten[i].farbe[0];
            // knoten[i * 4 + 3].farbe[1] = planeten[i].farbe[1];
            // knoten[i * 4 + 3].farbe[2] = planeten[i].farbe[2];
            // knoten[i * 4 + 3].farbe[3] = planeten[i].farbe[3];
        }
        //0, 1, 2, 2, 3, 0
        kanten[0] = 0;
        kanten[1] = 1;
        kanten[2] = 2;
        kanten[3] = 2;
        kanten[4] = 3;
        kanten[5] = 0;

        for (int i = 6; i < kanten_anzahl; i++) {
            kanten[i] = kanten[i - 6] + 4;
        }

        //Die größe der Planeten soll auf der Grafikkarte berechnet werden, 
        //da es effizienter ist, Berechnungen, die oft wiederholt werden besser auf der Grafikkarte gemacht werden.
        //Dafür hat der Vertex Shader eine Varibale die von der CPU gesetzt werden kann.
        //Diese Varibalen heißen "uniforms".
        //Im Vertex Shader Code kann die uniform "groesze" vom Typ float gefunden werden.
        //Die CPU muss nur noch wissen wo die sich diese Variable auf der Grafikkarte befinet
        unsigned int uniform_id = glGetUniformLocation(shader, "groesze");
        glUniform1f(uniform_id, kamera.zoom * MIN_ZOOM);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(knoten), knoten);

        glBindVertexArray(ebo);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(kanten), kanten);
        glDrawElements(GL_TRIANGLES, kanten_anzahl, GL_UNSIGNED_INT, NULL);

        /*Der Buffer enthält das, was gerade auf dem Bildschirm zu sehen ist.
        Wenn man einen Buffer verwenden würde könnte man sehen, wie das nächste Bild
        das gezeigt wird zusammen gesetzt wird, was zu einem flickndem Bild führen würde.
        Deshalb verwendet man zwei Buffer. Einen der gezeigt wird und einen, der versteckt wird.
        Der nachste Buffer wird erst gezeigt, sobald das Bild fertig zusammengesetzt wurde.
        Das flicken verchwindet so.*/
        glfwSwapBuffers(window);
        //rückmeldung an das OS, dass das programm noch läuft
        glfwPollEvents();
        zeit_bild_ende = glfwGetTime();
        //return 1; //das muss noch weg
    }

    glfwTerminate();
    return 0;
}

void readKeyboard(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        kamera.y += 1000;
    }
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        kamera.y -= 1000;
    }
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        kamera.x -= 1000;
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        kamera.x += 1000;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    float tmp = kamera.zoom + yoffset * kamera.zoom / 10;
    if (tmp  > 0.0f) {
        kamera.zoom = tmp;
    } else {
        kamera.zoom = 1;
        printf("%s%f\n", "t: ", tmp);
    }
    printf("%s%f\n", "m: ", kamera.zoom);
}

#include "render.h"
#include "texture.h"
#include <math.h>
#include <stddef.h>
#include <glad/glad.h>
#include <stdint.h>
#include <time.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <omp.h>

#define MIN_ZOOM 0.000005f
#define G 1000000
#define PLANETEN 1024

void readKeyboard(GLFWwindow *window);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void window_size_callback(GLFWwindow *window, int breite, int hoehe);
void window_focus_callback(GLFWwindow *window, int focus);

int pixel_breite = 1000;
double deltaTime = 0;
int fokussiert = 1;

typedef struct {
    int64_t x;
    int64_t y;
    float zoom;
    float x_zoom;
    float y_zoom;

    float radius;
    float masse;
    float x_geschwindigkeit;
    float y_geschwindigkeit;
    float x_beschleunigung;
    float y_beschleunigung;

    float rotation;

    int antrieb_level;
} Rakete;

Rakete rakete = {
    .x = 40000,
    .y = 0,
    .zoom = 1.0f,
    .x_zoom = 1,
    .y_zoom = 1,
    .radius = 50,
    .masse = 0,
    .x_geschwindigkeit = 0,
    .y_geschwindigkeit = 0,
    .x_beschleunigung = 0,
    .y_beschleunigung = 0,

    .rotation = 0.79f,

    .antrieb_level = 0
};

typedef struct {
  float position[2];
  float farbe[4];
  float texpos[2];
  int texid;
} Knoten;

typedef struct {
    int64_t x;
    int64_t y;
    float radius;
    float masse;
    float farbe[4];
    float x_geschwindigkeit;
    float y_geschwindigkeit;
    float x_beschleunigung;
    float y_beschleunigung;
    int texid;
} Planet;

void initrandom() {
    struct timespec ts;
    //falls unter Windows compiliert wird muss die ältere clock_gettime Prozedur genutzt werden
    #ifdef __linux__
        timespec_get(&ts, TIME_UTC);
    #elif _WIN32
        clock_gettime(CLOCK_REALTIME, &ts);
    #endif

    srand(ts.tv_nsec);
}

int main() {
    printf("Start\n");

    initrandom();

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

    //öffne das fenster: größe x; größe y; titel; unwichtig; unwichtig
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
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetWindowFocusCallback(window, window_focus_callback);

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
    glBufferData(GL_ARRAY_BUFFER, sizeof(Knoten) * 4 * PLANETEN, NULL, GL_DYNAMIC_DRAW);
    //aktiviere folgendes bei position 0
    glEnableVertexAttribArray(0);
    //Definiere wo die Position des Knotens im Speicher liegt:
    //Stelle Eingschaft; wie viele Variablen; Variablen Typ; soll die Variable umformatiert werden; Abstand zwischen den Knoten; Abstande vom Knoten Beginn bis zur Position im Speicher
    //glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Knoten), (const void*)offsetof(Knoten, position));
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Knoten), (const void*)offsetof(Knoten, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Knoten), (const void*)offsetof(Knoten, farbe));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Knoten), (const void*)offsetof(Knoten, texpos));
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 1, GL_INT, sizeof(Knoten), (const void*)offsetof(Knoten, texid));


    unsigned int ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 6 * PLANETEN, NULL, GL_DYNAMIC_DRAW);

    unsigned int rakete_vao;
    glGenVertexArrays(1, &rakete_vao);
    glBindVertexArray(rakete_vao);
    unsigned int rakete_vbo;
    glGenBuffers(1, &rakete_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, rakete_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Knoten) * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Knoten), (const void*)offsetof(Knoten, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Knoten), (const void*)offsetof(Knoten, farbe));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Knoten), (const void*)offsetof(Knoten, texpos));
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 1, GL_INT, sizeof(Knoten), (const void*)offsetof(Knoten, texid));

    unsigned int rakete_ebo;
    glGenBuffers(1, &rakete_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rakete_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 6, NULL, GL_DYNAMIC_DRAW);

    unsigned int shader = lade_shader();

    glUseProgram(shader);

    unsigned int id;
    glGenTextures(1, &id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    unsigned char *textur = lade_textur("./textures/planet32.raw", 32 * 32);
    //weise textur zu; typ, 0 = original auflösung;breite; höhe; format(rot grün blau transparenz 8bit); wie soll die texture gelesen werden; textur 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, textur);
    //die Textur ist jetzt im VRAM und der Speicherplatz im RAM kann wieder freigegeben werden
    free(textur);

    unsigned int id_pixel;
    glGenTextures(1, &id_pixel);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, id_pixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    unsigned char *pixel_textur = lade_textur("./textures/planet1.raw", 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_textur);
    free(pixel_textur);

    unsigned int id_rakete;
    glGenTextures(1, &id_rakete);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, id_rakete);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    unsigned char *rakete_textur = lade_textur("./textures/rakete.raw", 27 * 27);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 27, 27, 0, GL_RGBA, GL_UNSIGNED_BYTE, rakete_textur);
    free(rakete_textur);

    unsigned int id_punkt;
    glGenTextures(1, &id_punkt);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, id_punkt);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    unsigned char *punkt_textur = lade_textur("./textures/punkt.raw", 21 * 21);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 21, 21, 0, GL_RGBA, GL_UNSIGNED_BYTE, punkt_textur);
    free(punkt_textur);

    unsigned int id_rakete_antrieb1;
    glGenTextures(1, &id_rakete_antrieb1);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, id_rakete_antrieb1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    unsigned char *rakete_antrieb1_textur = lade_textur("./textures/rakete_antrieb1.raw", 27 * 27);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 27, 27, 0, GL_RGBA, GL_UNSIGNED_BYTE, rakete_antrieb1_textur);
    free(rakete_antrieb1_textur);


    //macht, dass der alpha kanal funktioniert
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Planet planeten[] = {
    //     {.x = 1000.0f, .y = 0.0f, .radius = 200.0f, .masse = 3000.0f, .farbe = {0.863, 0.925, 0.102, 1}, .texid = 0},
    //     {.x = 0.0f, .y = 0.0f, .radius = 100.0f, .masse = 0.0f, .farbe = {0.965, 0.569, 0.867, 1}, .texid = 0},
    // };

    float start_farbe1[] = {0.91f, 0.541f, 0.055f};
    float end_farbe1[] = {0.776f, 0.29f, 0.165f};

    float start_farbe2[] = {0.631, 0.722, 0.753};
    float end_farbe2[] = {0.137, 0.549f, 0.635f};

    Planet planeten[PLANETEN];
    planeten[0].x = 0; 
    planeten[0].y = 0;
    planeten[0].masse = 500000;
    planeten[0].x_geschwindigkeit = 0;
    planeten[0].y_geschwindigkeit = 0;
    planeten[0].x_beschleunigung = 0;
    planeten[0].y_beschleunigung = 0;
    planeten[0].radius = 3000;
    planeten[0].farbe[0] = 1.0f;
    planeten[0].farbe[1] = 1.0f;
    planeten[0].farbe[2] = 1.0f;
    planeten[0].farbe[3] = 1.0f;
    planeten[0].texid = 0;
    for (int i = 1; i < PLANETEN; i++) {
        float zufall_distanz = (rand() % 30000) + 20000;
        float zufall_winkel = (rand() % (int)(2 * M_PI * 100000)) / 100000.0f;
        planeten[i].x = zufall_distanz * sinf(zufall_winkel);
        planeten[i].y = zufall_distanz * cosf(zufall_winkel);

        float zufall_farbe = (float)(rand() % 1000000) / 1000000;
        if (rand() % 10 == 1) {
            planeten[i].masse = 5000;
            // planeten[i].x_geschwindigkeit = (rand() % 300) - 150;
            // planeten[i].y_geschwindigkeit = (rand() % 300) - 150;
            planeten[i].radius = 400;
            planeten[i].farbe[0] = (1.0f - zufall_farbe) * start_farbe2[0] + zufall_farbe * end_farbe2[0];
            planeten[i].farbe[1] = (1.0f - zufall_farbe) * start_farbe2[1] + zufall_farbe * end_farbe2[1];
            planeten[i].farbe[2] = (1.0f - zufall_farbe) * start_farbe2[2] + zufall_farbe * end_farbe2[2];
        } else {
            planeten[i].masse = 1000;
            // planeten[i].x_geschwindigkeit = (rand() % 3000) - 1500;
            // planeten[i].y_geschwindigkeit = (rand() % 3000) - 1500;
            planeten[i].radius = 200;
            planeten[i].farbe[0] = (1.0f - zufall_farbe) * start_farbe1[0] + zufall_farbe * end_farbe1[0];
            planeten[i].farbe[1] = (1.0f - zufall_farbe) * start_farbe1[1] + zufall_farbe * end_farbe1[1];
            planeten[i].farbe[2] = (1.0f - zufall_farbe) * start_farbe1[2] + zufall_farbe * end_farbe1[2];
        }
        planeten[i].x_geschwindigkeit = 0.3f * planeten[i].y;
        planeten[i].y_geschwindigkeit = 0.3f * -planeten[i].x;

        planeten[i].farbe[3] = 1.0f;
        planeten[i].texid = 0;
        planeten[i].x_beschleunigung = 0.0f;
        planeten[i].y_beschleunigung = 0.0f;
    }

    // for (int i = 1; i < PLANETEN; i++) {
    //     printf("%s%d%s%ld%s%ld%s%f%s%f%s%f%s%f\n", "planet ", i, ": ", planeten[i].x, " ", planeten[i].y, " vx=", planeten[i].x_geschwindigkeit, " vy=", planeten[i].y_geschwindigkeit, " ax=", planeten[i].x_beschleunigung, " ay=", planeten[i].y_beschleunigung);
    // }

    float zeit_bild_beginn = 0;
    float zeit_bild_ende = 0.0;

    //Die größe der Planeten soll auf der Grafikkarte berechnet werden, 
    //da es effizienter ist, Berechnungen, die oft wiederholt werden besser auf der Grafikkarte gemacht werden.
    //Dafür hat der Vertex Shader eine Varibale die von der CPU gesetzt werden kann.
    //Diese Varibalen heißen "uniforms".
    //Im Vertex Shader Code kann die uniform "groesze" vom Typ float gefunden werden.
    //Die CPU muss nur noch wissen wo die sich diese Variable auf der Grafikkarte befinet
    unsigned int zoom_id = glGetUniformLocation(shader, "groesze");
    unsigned int texturen_id = glGetUniformLocation(shader, "texturen");
    int texturen[] = {0, 1, 2, 3, 4};
    glUniform1iv(texturen_id, 5, texturen);

    //schleife in der auf den bildschirm gezeichnet wird
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //spezifiziere den zu nutzenden shader
        glUseProgram(shader);
        readKeyboard(window);

        int planeten_anzahl = sizeof(planeten) / sizeof(Planet);
        int knoten_anzahl = planeten_anzahl * 4;
        int kanten_anzahl = planeten_anzahl * 6;

        Knoten knoten[knoten_anzahl];
        int kanten[kanten_anzahl];

        //printf("%d\n", planeten_anzahl);
        //printf("%d\n", knoten_anzahl);
        //printf("%d\n", kanten_anzahl);


        zeit_bild_ende = glfwGetTime();
        float zeit_bild = zeit_bild_ende - zeit_bild_beginn;
        if (zeit_bild < 0.016666) {
            //printf("%s%f\n", "zeit korrektur: ", 0.016666 - zeit_bild);
            usleep(16666 - zeit_bild * 1000000);
        } else {
            printf("%lf\n", deltaTime);
        }
        deltaTime = glfwGetTime() - zeit_bild_beginn;
        //deltaTime = 0;
        zeit_bild_beginn = glfwGetTime();

        // for (int i = 1; i < planeten_anzahl; i++) {
        //     printf("%s%d%s%ld%s%ld%s%f%s%f%s%f%s%f\n", "planet ", i, ": ", planeten[i].x, " ", planeten[i].y, " vx=", planeten[i].x_geschwindigkeit, " vy=", planeten[i].y_geschwindigkeit, " ax=", planeten[i].x_beschleunigung, " ay=", planeten[i].y_beschleunigung);
        // }

        #pragma omp parallel for shared(planeten, planeten_anzahl, deltaTime)
        for (int i = 0; i < planeten_anzahl; i++) {
            planeten[i].x = planeten[i].x + planeten[i].x_geschwindigkeit * deltaTime + 0.5f * planeten[i].x_beschleunigung * deltaTime * deltaTime;
            planeten[i].y = planeten[i].y + planeten[i].y_geschwindigkeit * deltaTime + 0.5f * planeten[i].y_beschleunigung * deltaTime * deltaTime;

            float x_beschleunigung_alt = planeten[i].x_beschleunigung;
            float y_beschleunigung_alt = planeten[i].y_beschleunigung;

            planeten[i].x_beschleunigung = 0.0f;
            planeten[i].y_beschleunigung = 0.0f;

            for (int a = 0; a < planeten_anzahl; a++) {
                if (a != i) {
                    int64_t delta_x = planeten[a].x - planeten[i].x;
                    int64_t delta_y = planeten[a].y - planeten[i].y;
                    int64_t distanz_q = delta_x * delta_x + delta_y * delta_y;
                    float distanz = sqrtf(distanz_q);

                    if (distanz > 0 && distanz > planeten[a].radius + planeten[i].radius) {
                        float beschleunigung = G * planeten[a].masse / distanz_q;
                        planeten[i].x_beschleunigung += beschleunigung * delta_x / distanz;
                        planeten[i].y_beschleunigung += beschleunigung * delta_y / distanz;
                    }
                }
            }

            planeten[i].x_geschwindigkeit = planeten[i].x_geschwindigkeit + 0.5f * (x_beschleunigung_alt + planeten[i].x_beschleunigung) * deltaTime;
            planeten[i].y_geschwindigkeit = planeten[i].y_geschwindigkeit + 0.5f * (y_beschleunigung_alt + planeten[i].y_beschleunigung) * deltaTime;
        }

        //printf("%ld%s%ld%s%f%s%f%s%f%s%f\n", rakete.x, " ", rakete.y, " ", rakete.x_beschleunigung, " ", rakete.y_beschleunigung, " ", rakete.x_geschwindigkeit, " ", rakete.y_geschwindigkeit);

        int staerkste_beschleunigung_index;
        float staerkste_beschleunigung = 0;

        rakete.x = rakete.x + rakete.x_geschwindigkeit * deltaTime + 0.5f * rakete.x_beschleunigung * deltaTime * deltaTime;
        rakete.y = rakete.y + rakete.y_geschwindigkeit * deltaTime + 0.5f * rakete.y_beschleunigung * deltaTime * deltaTime;

        float x_beschleunigung_alt = rakete.x_beschleunigung;
        float y_beschleunigung_alt = rakete.y_beschleunigung;

        rakete.x_beschleunigung = 0.0f;
        rakete.y_beschleunigung = 0.0f;

        for (int a = 0; a < planeten_anzahl; a++) {
            int64_t delta_x = planeten[a].x - rakete.x;
            int64_t delta_y = planeten[a].y - rakete.y;
            int64_t distanz_q = delta_x * delta_x + delta_y * delta_y;
            float distanz = sqrtf(distanz_q);

            if (distanz > 0 && distanz > planeten[a].radius + rakete.radius) {
                float beschleunigung = G * planeten[a].masse / distanz_q;
                rakete.x_beschleunigung += beschleunigung * delta_x / distanz;
                rakete.y_beschleunigung += beschleunigung * delta_y / distanz;
                
                if (beschleunigung > staerkste_beschleunigung) {
                    staerkste_beschleunigung_index = a;
                }
            }
        }

        rakete.x_geschwindigkeit = rakete.x_geschwindigkeit + 0.5f * (x_beschleunigung_alt + rakete.x_beschleunigung) * deltaTime;
        rakete.y_geschwindigkeit = rakete.y_geschwindigkeit + 0.5f * (y_beschleunigung_alt + rakete.y_beschleunigung) * deltaTime;
        
        //wenn das Fenster minimiert wird muss nicht gezeichnet werden und die Simulation kann weiter berechnet werden
        if (fokussiert) 
        { //<--Das ist eine Außnahme um die Lesbarkeit zu verbessern
        float radius_anpassung;
        int textur_anpassung;

        for (int i = 0; i < planeten_anzahl; i++) {
            if ((planeten[i].radius * rakete.zoom * rakete.x_zoom * MIN_ZOOM) * pixel_breite <= 1.0f) {
                radius_anpassung = (1.0f / pixel_breite) / (rakete.zoom * rakete.x_zoom * MIN_ZOOM);
                textur_anpassung = 1;
            } else {
                radius_anpassung = planeten[i].radius;
                textur_anpassung = planeten[i].texid;
            }

            knoten[i * 4 + 0].position[0] = (planeten[i].x - rakete.x - radius_anpassung);
            knoten[i * 4 + 0].position[1] = (planeten[i].y - rakete.y - radius_anpassung);
            //kopiere die frabe nach hier von hier und das ist die zu kopierende größe
            memcpy(knoten[i * 4 + 0].farbe, planeten[i].farbe, sizeof(float) * 4);
            knoten[i * 4 + 0].texpos[0] = 0.0f;
            knoten[i * 4 + 0].texpos[1] = 1.0f;
            knoten[i * 4 + 0].texid = textur_anpassung;

            knoten[i * 4 + 1].position[0] = (planeten[i].x - rakete.x + radius_anpassung);
            knoten[i * 4 + 1].position[1] = (planeten[i].y - rakete.y - radius_anpassung);
            memcpy(knoten[i * 4 + 1].farbe, planeten[i].farbe, sizeof(float) * 4);
            knoten[i * 4 + 1].texpos[0] = 1.0f;
            knoten[i * 4 + 1].texpos[1] = 1.0f;
            knoten[i * 4 + 1].texid = textur_anpassung;

            knoten[i * 4 + 2].position[0] = (planeten[i].x - rakete.x + radius_anpassung);
            knoten[i * 4 + 2].position[1] = (planeten[i].y - rakete.y + radius_anpassung);
            memcpy(knoten[i * 4 + 2].farbe, planeten[i].farbe, sizeof(float) * 4);
            knoten[i * 4 + 2].texpos[0] = 1.0f;
            knoten[i * 4 + 2].texpos[1] = 0.0f;
            knoten[i * 4 + 2].texid = textur_anpassung;

            knoten[i * 4 + 3].position[0] = (planeten[i].x - rakete.x - radius_anpassung);
            knoten[i * 4 + 3].position[1] = (planeten[i].y - rakete.y + radius_anpassung);
            memcpy(knoten[i * 4 + 3].farbe, planeten[i].farbe, sizeof(float) * 4);
            knoten[i * 4 + 3].texpos[0] = 0.0f;
            knoten[i * 4 + 3].texpos[1] = 0.0f;
            knoten[i * 4 + 3].texid = textur_anpassung;
        }

        if ((rakete.radius * rakete.zoom * rakete.x_zoom * MIN_ZOOM) * pixel_breite <= 15.0f) {
            rakete.radius = (15.0f / pixel_breite) / (rakete.zoom * rakete.x_zoom * MIN_ZOOM);
            textur_anpassung = 3;
        } else {
            rakete.radius = 50;
            if (rakete.antrieb_level > 0) {
                textur_anpassung = 4;
            } else {
                textur_anpassung = 2;
            }
        }

        Knoten rakete_knoten[] = {
            {.position = {-rakete.radius * cosf(rakete.rotation) - -rakete.radius * sinf(rakete.rotation), -rakete.radius * cosf(rakete.rotation) + -rakete.radius * sinf(rakete.rotation)}, .farbe = {1.0f, 1.0f, 1.0f, 1.0f}, .texpos = {0.0f, 1.0f}, .texid = textur_anpassung},
            {.position = {+rakete.radius * cosf(rakete.rotation) - -rakete.radius * sinf(rakete.rotation), -rakete.radius * cosf(rakete.rotation) + +rakete.radius * sinf(rakete.rotation)}, .farbe = {1.0f, 1.0f, 1.0f, 1.0f}, .texpos = {1.0f, 1.0f}, .texid = textur_anpassung},
            {.position = {+rakete.radius * cosf(rakete.rotation) - +rakete.radius * sinf(rakete.rotation), +rakete.radius * cosf(rakete.rotation) + +rakete.radius * sinf(rakete.rotation)}, .farbe = {1.0f, 1.0f, 1.0f, 1.0f}, .texpos = {1.0f, 0.0f}, .texid = textur_anpassung},
            {.position = {-rakete.radius * cosf(rakete.rotation) - +rakete.radius * sinf(rakete.rotation), +rakete.radius * cosf(rakete.rotation) + -rakete.radius * sinf(rakete.rotation)}, .farbe = {1.0f, 1.0f, 1.0f, 1.0f}, .texpos = {0.0f, 0.0f}, .texid = textur_anpassung}
        };

        //kanten start werte / verbinde knoten 0 mit 1 und 1 mit 2 usw.
        kanten[0] = 0;
        kanten[1] = 1;
        kanten[2] = 2;
        kanten[3] = 2;
        kanten[4] = 3;
        kanten[5] = 0;

        for (int i = 6; i < kanten_anzahl; i++) {
            kanten[i] = kanten[i - 6] + 4;
        }

        int rakete_kanten[] = {0, 1, 2, 2, 3, 0};

        glUniform2f(zoom_id, rakete.zoom * rakete.x_zoom * MIN_ZOOM, rakete.zoom * rakete.y_zoom * MIN_ZOOM);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(knoten), knoten);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(kanten), kanten);
        glDrawElements(GL_TRIANGLES, kanten_anzahl, GL_UNSIGNED_INT, NULL);

        glBindVertexArray(rakete_vao);
        
        glBindBuffer(GL_ARRAY_BUFFER, rakete_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(rakete_knoten), rakete_knoten);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rakete_ebo);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(rakete_kanten), rakete_kanten);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

        /*Der Buffer enthält das, was gerade auf dem Bildschirm zu sehen ist.
        Wenn man einen Buffer verwenden würde könnte man sehen, wie das nächste Bild
        das gezeigt wird zusammen gesetzt wird, was zu einem flickndem Bild führen würde.
        Deshalb verwendet man zwei Buffer. Einen der gezeigt wird und einen, der versteckt wird.
        Der nachste Buffer wird erst gezeigt, sobald das Bild fertig zusammengesetzt wurde.
        Das flicken verchwindet so.*/
        glfwSwapBuffers(window);
        }
        //rückmeldung an das OS, dass das programm noch läuft
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void readKeyboard(GLFWwindow *window) {
    // if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    //     rakete.x_geschwindigkeit -= 100 * sinf(rakete.rotation);
    //     rakete.y_geschwindigkeit += 100 * cosf(rakete.rotation);
    // }
    // if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    //     rakete.x_geschwindigkeit += 100 * sinf(rakete.rotation);
    //     rakete.y_geschwindigkeit -= 100 * cosf(rakete.rotation);
    // }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        //rakete.x -= 1000;
        rakete.rotation += 5.0 * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        //rakete.x += 1000;
        rakete.rotation -= 5.0 * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            rakete.x_beschleunigung -= 10000 * sinf(rakete.rotation);
            rakete.y_beschleunigung += 10000 * cosf(rakete.rotation);
            rakete.antrieb_level = 2;
        } else {
            rakete.x_beschleunigung -= 5000 * sinf(rakete.rotation);
            rakete.y_beschleunigung += 5000 * cosf(rakete.rotation);
            rakete.antrieb_level = 1;
        }
    } else {
        rakete.antrieb_level = 0;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    float tmp = rakete.zoom + yoffset * rakete.zoom / 10;
    if (tmp  > 0.0f) {
        rakete.zoom = tmp;
    } else {
        rakete.zoom = 1;
        printf("%s%f\n", "t: ", tmp);
    }
    //printf("%s%f\n", "m: ", rakete.zoom);
}
void window_size_callback(GLFWwindow* window, int breite, int hoehe) {
    glViewport(0, 0, breite, hoehe);
    rakete.x_zoom = 1000.0f / breite;
    rakete.y_zoom = 1000.0f / hoehe;
    pixel_breite = breite;
}
void window_focus_callback(GLFWwindow * window, int focus) {
    fokussiert = focus;
}
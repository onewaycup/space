# space
### zum Kompileren:
#### linux:
1. gcc instalieren `sudo <packagemanager> install gcc`
2. glfw instalieren `sudo <packagemanager> install glfw`
4. `make clean space run`
#### windows:
2. [gcc](https://jmeubank.github.io/tdm-gcc/) herunterladen und beim instalieren "Alle Pakete" Option auswählen
3. `gcc -o space.exe -Wall -ggdb -fno-omit-frame-pointer -O2 main.c glad.c render.c texture.c -lglfw3 -lm -fopenmp -Iinclude -Lbin -lopengl32 -lgdi32`
#### glad und glfw müssen evtl. noch eingefügt werden
### oder fertiges Build herunterladen
#### linux: als ausfühbar makieren `./space`
#### windows: `space.exe`

### spielen:
S D zum drehen

Leertaste zum beschleunigen

Leertaste + Shift zum schnellen beschleunigen

Scrollen zum zoomen

Compilation du projet depuis Msys2 minGW64 sur windows avec problèmes d'inclusion des bibliothèques SDL

gcc -o prog main.c -DSDL_MAIN_HANDLED -lSDLmain -lSDL -lSDL_gfx "-IC:\msys64\mingw64\include" "-LC:\msys64\mingw64\lib"

#include <stdio.h>
#include <unistd.h>
#include <wchar.h>
#include <locale.h>

#include "../include/eInkTools.h"

int main(void) {
    init_display();
    clear_display();
    setlocale(LC_ALL, "");

    //display_grid(8);
    write_string(UNIFONT, 32, 32, 16, "こんにちは、世界");
    write_string(UNIFONT, 32, 64, 16, "Hello, World!");

//    int width, height;
//    write_char(UNIFONT, 32, 32, 32, &width, &height, 0x306F);

//    display_cross(32, 32);

    
    //write_string(UNIFONT, 16, 64, 32, "Σ੧(❛□❛✿)");

    activate_display();
    printf("character displayed\n");

    sleep_display();
    return 0;
}
#include <stdio.h>
#include <unistd.h>

#include "../include/eInkTools.h"

// Font paths
#define MONACO "/home/frasercrumpler/projects/prod/fonts/Monaco.ttf"
#define BLOCK "/home/frasercrumpler/projects/prod/fonts/BlockMerged-RnjW.ttf"
#define QABEXEL "/home/frasercrumpler/projects/prod/fonts/Qabaxel-2v3el.ttf"

int main(void) {
    init_display();
    clear_display();

    write_string(QABEXEL, 64, 64, 32, "Hello");
        
    display_grid();

    activate_display();
    printf("character displayed\n");

    sleep_display();
    return 0;
}
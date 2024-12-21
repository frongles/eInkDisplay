#include <stdio.h>
#include <unistd.h>
#include "../include/eInkTools.h"

int main(void) {
    printf("initialising display\n");
    init_display();
    printf("display initialised\n");

    printf("clearing display\n");
    clear_display();
    printf("display cleared\n");
    sleep(3);
    printf("about to pattern display\n");
    pattern_display();
    printf("pattern displayed\n");
    sleep(3);
    printf("clearing display\n");
    clear_display();
    sleep(3);
    printf("display cleared\n");
    cleanup();
    return 0;
}
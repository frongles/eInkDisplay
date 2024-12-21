
#include "../include/eInkTools.h"

int main(void) {
    init_display();
    clear_display();
    sleep(3);
    pattern_display();
    sleep(3);
    clear_display();

    cleanup();
    return 0;
}
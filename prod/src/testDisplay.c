#include <stdio.h>
#include <unistd.h>
#include <wchar.h>

#include "../include/eInkTools.h"

int main(void) {
    init_display();
    clear_display();

    wchar_t ch = L'は';
    printf("%d\n", ch);

    int array[] = {0x3A3, 0xA67, 0x28, 0x275B, ' ', 0x25A1, 0x275B, ' ', 0x273F, 0x29, '\0'};
    write_unicode(UNIFONT, 32, 122/2, 32, array);


    
    //write_string(UNIFONT, 16, 64, 32, "Σ੧(❛□❛✿)");

    activate_display();
    printf("character displayed\n");

    sleep_display();
    return 0;
}
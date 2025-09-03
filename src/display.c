#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wchar.h>
#include <locale.h>

#include "../include/stb_truetype.h"
#include "../include/eInkTools.h"

int main(void) {
    init_display();
    clear_display();
    setlocale(LC_ALL, "");

    //display_grid(8);
    printf("initialising font\n");
    stbtt_fontinfo* fontinfo = init_font(UNIFONT, 32);
    printf("Writing string\n");
    write_string(fontinfo, 30, 64, 16, "ヤッホー、ヒナ");
    write_string(fontinfo, 32, 32, 16, "直した！😁");
    printf("String written\n");
//    int width, height;
//    write_char(UNIFONT, 32, 32, 32, &width, &height, 0x306F);

//    display_cross(32, 32);

    //write_string(UNIFONT, 16, 64, 32, "Σ੧(❛□❛✿)");

    activate_display();

    sleep_display();
    free(fontinfo->data);
    free(fontinfo);
    return 0;
}
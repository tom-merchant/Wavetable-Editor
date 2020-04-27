//
// Created by tmerchant on 04/04/2020.
//

#ifndef WAVETABLE_EDITOR_UTIL_H
#define WAVETABLE_EDITOR_UTIL_H

typedef struct button_hitbox
{
    int* highlight;
    int rect[4];
} button_hitbox;

bool is_in_rect(int x, int y, int rectx, int recty, int rectw, int recth);

#endif //WAVETABLE_EDITOR_UTIL_H

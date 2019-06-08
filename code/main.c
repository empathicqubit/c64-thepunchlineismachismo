#include <stdio.h>
#include <stdlib.h>
#include <tgi.h>
#include <joystick.h>
#include <conio.h>

extern const char text[];

void joy_wait(char mask) {
    while(1) {
        char val = joy_read(0x01);
        if(val & mask) {
            break;
        }
    }
}

void (*joy_hooks[])(void) = { 0x00, 0x00, 0x00, 0x00, 0x00 };

void maybe_call_joy_hook(int idx) {
    void (*hook)(void) = joy_hooks[idx];
    if(hook) {
        hook();
        joy_hooks[idx] = 0x00;
    }
}

void set_joy_hook(int idx, void (*ptr)(void)) {
    joy_hooks[idx] = ptr;
}

#define JOY_HOOK_UP 0
#define JOY_HOOK_DOWN 1
#define JOY_HOOK_LEFT 2
#define JOY_HOOK_RIGHT 3
#define JOY_HOOK_FIRE 4

void step_two(void) {
    tgi_setcolor(TGI_COLOR_GREEN);
    tgi_bar(45 + 45, 45, 90 + 45, 90);
}

void step_one(void) {
    tgi_setcolor(TGI_COLOR_GREEN);
    tgi_bar(45, 45, 90, 90);

    set_joy_hook(JOY_HOOK_RIGHT, step_two);
}

void input_loop(void) {
    while(1) {
        char val = joy_read(0x01);

        if(val & JOY_UP_MASK) {
            maybe_call_joy_hook(JOY_HOOK_UP);
        }
        if(val & JOY_DOWN_MASK) {
            maybe_call_joy_hook(JOY_HOOK_DOWN);
        }
        if(val & JOY_LEFT_MASK) {
            maybe_call_joy_hook(JOY_HOOK_LEFT);
        }
        if(val & JOY_RIGHT_MASK) {
            maybe_call_joy_hook(JOY_HOOK_RIGHT);
        }
    }
}

int main (void) {
    printf("%s\n", text);

    joy_install(&joy_static_stddrv);

    tgi_install(tgi_static_stddrv);
    tgi_init();
    tgi_clear();

    set_joy_hook(JOY_HOOK_UP, step_one);

    input_loop();

    return EXIT_SUCCESS;
}

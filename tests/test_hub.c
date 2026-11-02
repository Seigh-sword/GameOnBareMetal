#include "hub.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    hub_state s = { HUB_HOME, 0 };
    hub_step(&s, HUB_UP); assert(s.selected == 2);
    hub_step(&s, HUB_DOWN); assert(s.selected == 0);
    hub_step(&s, HUB_ENTER); assert(s.page == HUB_PLAY);
    hub_step(&s, HUB_ENTER); assert(s.page == HUB_DOOM);
    hub_step(&s, HUB_BACK); assert(s.page == HUB_PLAY);
    hub_step(&s, HUB_BACK); assert(s.page == HUB_HOME);
    hub_step(&s, HUB_DOWN); hub_step(&s, HUB_ENTER); assert(s.page == HUB_SETTINGS);
    hub_step(&s, HUB_BACK); hub_step(&s, HUB_UP); hub_step(&s, HUB_ENTER); assert(s.page == HUB_MODE);
    hub_step(&s, HUB_BACK); assert(s.page == HUB_HOME);
    puts("Hub navigation OK");
}

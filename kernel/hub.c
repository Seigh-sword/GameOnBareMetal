#include "hub.h"
void hub_step(hub_state *s, hub_action action) {
    if (action == HUB_BACK) {
        if (s->page == HUB_DOOM) s->page = HUB_PLAY;
        else if (s->page != HUB_HOME) s->page = HUB_HOME;
        s->selected = 0;
        return;
    }
    if (s->page == HUB_HOME) {
        if (action == HUB_UP) s->selected = (s->selected + 2) % 3;
        if (action == HUB_DOWN) s->selected = (s->selected + 1) % 3;
        if (action == HUB_ENTER) {
            s->page = s->selected == 0 ? HUB_PLAY : s->selected == 1 ? HUB_SETTINGS : HUB_MODE;
            s->selected = 0;
        }
    } else if (s->page == HUB_PLAY && action == HUB_ENTER) {
        s->page = HUB_DOOM;
        s->selected = 0;
    }
}

#ifndef HUB_H
#define HUB_H
/* Pure menu state for the boot hub, independent of the display and keyboard. */
typedef enum { HUB_HOME, HUB_PLAY, HUB_SETTINGS, HUB_MODE, HUB_DOOM, HUB_CXG } hub_page;
typedef enum { HUB_UP, HUB_DOWN, HUB_ENTER, HUB_BACK } hub_action;
typedef struct { hub_page page; unsigned selected; } hub_state;
void hub_step(hub_state *state, hub_action action);
#endif

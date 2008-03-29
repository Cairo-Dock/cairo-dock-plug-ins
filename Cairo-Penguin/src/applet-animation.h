
#ifndef __APPLET_ANIMATION__
#define  __APPLET_ANIMATION__

#include <cairo-dock.h>


#define penguin_get_current_animation() (myData.iCurrentAnimation >= 0 ? &myData.pAnimations[myData.iCurrentAnimation] : NULL);


gboolean penguin_move_in_dock (gpointer data);


gboolean penguin_draw_on_dock (GtkWidget *pWidget, GdkEventExpose *pExpose, gpointer data);


gboolean penguin_move_in_icon (gpointer data);


void penguin_calculate_new_position (PenguinAnimation *pAnimation, int iXMin, int iXMax, int iHeight);

void penguin_advance_to_next_frame (PenguinAnimation *pAnimation);

int penguin_choose_movement_animation (void);
int penguin_choose_go_up_animation (void);
int penguin_choose_beginning_animation (void);
int penguin_choose_ending_animation (void);
int penguin_choose_resting_animation (void);
int penguin_choose_next_animation (PenguinAnimation *pAnimation);

void penguin_set_new_animation (int iNewAnimation);

void penguin_start_animating (void);
void penguin_start_animating_with_delay (gboolean bInit);


#endif

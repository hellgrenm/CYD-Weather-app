#include <lvgl.h>
#include <stdio.h>
#include "ui.h"
#include <stdbool.h>


bool hide_popup = true;


bool get_var_hide_popup() {
  return hide_popup;
}

void set_var_hide_popup(bool value) {
  hide_popup = value;
}


void action_show_about(lv_event_t * e) {
  hide_popup = false;
  lv_label_set_text(objects.active_program, "Information" );
  lv_obj_invalidate(lv_scr_act()); 
}

void action_close_about(lv_event_t * e) {
  hide_popup = true;  
  lv_label_set_text(objects.active_program, "Local Weather" );
  lv_obj_invalidate(lv_scr_act()); 
}
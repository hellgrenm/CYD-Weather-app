#ifndef EEZ_LVGL_UI_IMAGES_H
#define EEZ_LVGL_UI_IMAGES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_img_dsc_t img_btn_close;
extern const lv_img_dsc_t img_btn_close_pressed;
extern const lv_img_dsc_t img_pc_icon;
extern const lv_img_dsc_t img_recycle_icon;
extern const lv_img_dsc_t img_start_button;
extern const lv_img_dsc_t img_start_button_pressed;
extern const lv_img_dsc_t img_qmark;
extern const lv_img_dsc_t img_qmark_pressed;

#ifndef EXT_IMG_DESC_T
#define EXT_IMG_DESC_T
typedef struct _ext_img_desc_t {
    const char *name;
    const lv_img_dsc_t *img_dsc;
} ext_img_desc_t;
#endif

extern const ext_img_desc_t images[8];

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_IMAGES_H*/
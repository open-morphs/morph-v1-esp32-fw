#include "LVGLImage.hpp"

LVGLImage::LVGLImage(const lv_img_dsc_t* img,  LVGLBase* const parent)
    :   LVGLBase(lv_img_create(parent? parent->innerData() : lv_disp_get_scr_act(NULL), NULL), parent) {

    updateImage(img);
}

void LVGLImage::updateImage(const lv_img_dsc_t* img) {
    if (img) {
        lv_img_set_src(_obj, img);
    }
}

// void LVGLImage::zoomOut(const size_t percent) {
//     lv_img_set_pivot(_obj, 100, 45);  //To zoom from the left top corner
//     lv_img_set_zoom(_obj, 64);
//     // lv_img_set_offset_x(_obj, 45); //Select the second image
//     // lv_img_set_pivot(_obj, 100, 45);  //To zoom from the left top corner
//     lv_obj_set_size(_obj, 150, 80);
// }
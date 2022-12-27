#pragma once
#include "LVGLBase.hpp"

class LVGLContainer : public LVGLBase {

    public:
        explicit LVGLContainer(LVGLBase* const parent = NULL);
        explicit LVGLContainer(lv_obj_t* const parent);
        explicit LVGLContainer(lv_obj_t* const src, LVGLBase* const parent);

        void setLayout(const lv_layout_t layout);
        void setFit(const lv_fit_t fit);
        void setFit(const lv_fit_t hor, const lv_fit_t ver);
        void setFit(const lv_fit_t left, const lv_fit_t right, const lv_fit_t top, const lv_fit_t bottom);

        void addObject(LVGLBase* const obj);
};
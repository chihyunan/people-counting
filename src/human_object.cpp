#include "human_object.hpp"

int HumanObject::serial_num_ = 0;

HumanObject::HumanObject(int blob_label, int pos_x, int pos_y, int size)
{
    serial_num_ += 1;
    label_ = serial_num_;
    DBG_PRINT("create object %d from Blob %d at %d %d\n", label_, blob_label, pos_x, pos_y);
    now_.pos_x = pos_x;
    now_.pos_y = pos_y;
    now_.vel_x = 0;
    now_.vel_y = 0;
    now_.size = size;
    last_ = now_;
    vrtl_age_ = 0;
    first_x_ = pos_x;
    first_y_ = pos_y;
}

void HumanObject::ab_filter(int pos_x, int pos_y)
{
    float alpha = 0.75;
    float beta = 0.8;
    int res_x = pos_x - now_.pos_x;
    int res_y = pos_y - now_.pos_y;
    now_.pos_x += res_x * alpha;
    now_.pos_y += res_y * alpha;
    now_.vel_x = (1 - beta) * now_.vel_x + beta * res_x;
    now_.vel_y = (1 - beta) * now_.vel_y + beta * res_y;
}

void HumanObject::update(int pos_x, int pos_y, int size)
{
    last_ = now_;
    ab_filter(pos_x, pos_y);
    now_.size = size;
    DBG_PRINT("pos (%d %d)->(%d %d) vel (%d %d)->(%d %d) sz %d->%d\n", last_.pos_x, last_.pos_y, now_.pos_x, now_.pos_y, last_.vel_x, last_.vel_y, now_.vel_x, now_.vel_y, last_.size, now_.size);
}

void HumanObject::predict(int *ppos_x, int *ppos_y)
{
    *ppos_x = now_.pos_x + now_.vel_x;
    *ppos_y = now_.pos_y + now_.vel_y;
}

void HumanObject::get_last_predict(int *lppos_x, int *lppos_y)
{
    *lppos_x = last_.pos_x + last_.vel_x;
    *lppos_y = last_.pos_y + last_.vel_y;
}

int HumanObject::counting()
{
    static const int bondary = (IM_H - 1) / 2;
    DBG_PRINT("finish count of ob %d, %d->%d\n", label_, first_y_, now_.pos_y);
    if ((first_y_ < bondary) && (now_.pos_y > bondary))
    {
        return 1;
    }
    else if ((first_y_ > bondary) && (now_.pos_y < bondary))
    {
        return -1;
    }
    return 0;
}

HumanObject::~HumanObject()
{
    DBG_PRINT("delete object %d\n", label_);
}

int HumanObject::get_index()
{
    return label_;
}

void HumanObject::get_size(int *size, int *last_sz)
{
    *size = now_.size;
    *last_sz = last_.size;
}

void HumanObject::get_last_pos(int *lpos_x, int *lpos_y)
{
    *lpos_x = last_.pos_x;
    *lpos_y = last_.pos_y;
}

void HumanObject::get_now_pos(int *npos_x, int *npos_y)
{
    *npos_x = now_.pos_x;
    *npos_y = now_.pos_y;
}

int HumanObject::get_virtual_age()
{
    return vrtl_age_;
}

void HumanObject::virtual_update(int pos_x, int pos_y, int size)
{
    vrtl_age_ = 0;
    now_.pos_x = pos_x;
    now_.pos_y = pos_y;
    now_.size = size;
    now_.vel_x += pos_x - vrtl_.pos_x;
    now_.vel_y += pos_y - vrtl_.pos_y;
    last_ = now_;
    DBG_PRINT("virtual updated at pos (%d, %d) vel (%d, %d)\n", pos_x, pos_y, now_.vel_x, now_.vel_y);
}

void HumanObject::virtual_propagation()
{
    vrtl_age_ += 1;
    vrtl_.pos_x = now_.pos_x + now_.vel_x * (vrtl_age_ + 1);
    vrtl_.pos_y = now_.pos_y + now_.vel_y * (vrtl_age_ + 1);
    DBG_PRINT("object %d virtual propagated, age: %d, virtual pos: (%d, %d)\n", label_, vrtl_age_, vrtl_.pos_x, vrtl_.pos_y);
}

void HumanObject::get_virtual_pos(int *vppos_x, int *vppos_y)
{
    *vppos_x = vrtl_.pos_x;
    *vppos_y = vrtl_.pos_y;
}
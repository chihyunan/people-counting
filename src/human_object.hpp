#pragma once
extern "C"
{
#include "stdbool.h"
#include "stdio.h"
#include <string.h>
}
#include "macro.h"

class HumanObject
{
private:
    static int serial_num_;
    int label_;
    struct state_
    {
        int size;
        int pos_x;
        int pos_y;
        int vel_x;
        int vel_y;
    } last_, now_,vrtl_;
    int first_x_;
    int first_y_;
    int vrtl_age_;

public:
    HumanObject(int label, int pos_x, int pos_y, int size);
    ~HumanObject();
    void update(int pos_x, int pos_y, int size);
    void ab_filter(int pos_x, int pos_y);
    void predict(int *ppos_x, int *ppos_y);
    void get_last_predict(int *last_pred_pos_x, int *last_pred_pos_y);
    int get_index();
    void get_size(int *size, int *last_size);
    void get_last_pos(int *lpos_x, int *lpos_y);
    void get_now_pos(int *npos_x, int *npos_y);
    int counting();

    void virtual_update(int pos_x, int pos_y, int size);
    void virtual_propagation();
    void get_virtual_pos(int *vrtl_predict_pos_x, int *vrtl_predict_pos_y);
    int get_virtual_age();
};

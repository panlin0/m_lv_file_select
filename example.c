#include "lvgl/lvgl.h"
#include "m_lv_file_select.h"


static void event(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    if(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
    {
        printf("OpenDir:%s\n",m_lv_file_select_get_dir(obj));
        //m_lv_file_select_set_srot_flag(obj,false); /*call this interface to set the sort*/
        //m_lv_file_select_set_srot(obj,fn);         /*call this interface to customize the sort comparison function*/
    }else if (lv_event_get_code(e) == LV_EVENT_READY)
    {
        printf("\nSelected:%s\n",m_lv_file_select_read(obj));
    }
}


void example(void)
{
    lv_obj_t *file_view = m_lv_file_select_create(lv_scr_act());
    m_lv_file_select_open(file_view,"/",FILE_SELECT_MODE_ALL|FILE_SELECT_MODE_MULTI,NULL); /*select multiple files*/
    // m_lv_file_select_open(file_view,"/",FILE_SELECT_MODE_DIR|FILE_SELECT_MODE_MULTI,NULL); /*select multiple files directory*/

    // m_lv_file_select_open(file_view,"/",FILE_SELECT_MODE_DIR|FILE_SELECT_MODE_ONE,NULL);   /*Select only one file*/
    // m_lv_file_select_open(file_view,"/",FILE_SELECT_MODE_ALL|FILE_SELECT_MODE_ONE,NULL);   /*Select only one directory*/

    //m_lv_file_select_open(file_view,"/",FILE_SELECT_MODE_ALL|FILE_SELECT_MODE_MULTI,".jpg .bmp .png"); /*filter files and select multiple*/

    //m_lv_file_select_open(file_view,"/",FILE_SELECT_MODE_ALL,NULL); /*file browser mode*/
    lv_obj_add_event_cb(file_view,event,LV_EVENT_VALUE_CHANGED,NULL);
    lv_obj_add_event_cb(file_view,event,LV_EVENT_READY,NULL);
}

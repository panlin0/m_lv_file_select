/**
 * @file m_lv_file_select.c
 * @brief
 * @author PanLin (786005498@qq.com)
 * @version 1.0
 * @date 2023-10-09
 *
 * @copyright Copyright (c) 2023
 */
/*********************
 *      INCLUDES
 *********************/
#include "m_lv_file_select.h"
#include "lvgl/lvgl.h"
#include  <stdio.h>
#include  <string.h>
#include  <stdlib.h>
#include  <stdint.h>
#include  <dirent.h>

#if (defined(M_FILE_SELECT_SUPPORT_LVGL_FS) || defined(M_FILE_SELECT_SUPPORT_STDIO_FS))
/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &m_lv_file_select_class

/**********************
 *      TYPEDEFS
 **********************/


/**********************
 *  STATIC PROTOTYPES
 **********************/
static void file_select_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void file_select_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static lv_fs_res_t file_select_open(m_lv_file_select_t *f);

#ifdef M_FILE_SELECT_SUPPORT_STDIO_FS
static inline lv_fs_res_t  stdio_fs_open_dir(m_lv_file_select_t *f);
#endif

#ifdef M_FILE_SELECT_SUPPORT_LVGL_FS
static inline lv_fs_res_t lvgl_fs_open_dir(m_lv_file_select_t *f);
#endif

static bool is_image(const char* fn);
static bool is_video(const char* fn);
static bool is_audio(const char* fn);
static selct_list_t * alloc_node(const char *fn);
static void free_node(m_lv_file_select_t *f);
static bool file_filter(const char *filter,const char *fn);
static void table_show(m_lv_file_select_t *f);
static void one_select(m_lv_file_select_t *f,selct_list_t *node);
static void event_all_cb(lv_event_t *e);
static void event_lab_ok_cb(lv_event_t *e);
static void event_lab_back_cb(lv_event_t *e);

static bool  sort_name_fn(const char *str1,const char *str2,bool f);
static void _quicksort(selct_list_t **list,uint16_t  left,uint16_t  right,bool(*fn)(const char*,const char*,bool),bool f);
static void  dir_file_sort(m_lv_file_select_t *f);
/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t m_lv_file_select_class =
{
    .constructor_cb = file_select_constructor,
    .destructor_cb = file_select_destructor,
    .width_def =  LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(m_lv_file_select_t),
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .base_class = &lv_obj_class
};

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t *m_lv_file_select_create(lv_obj_t *parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

lv_fs_res_t m_lv_file_select_open(lv_obj_t *obj, const char *dir, m_lv_file_select_mode_t mode, const char *filter)
{
    lv_fs_res_t res;
    char  *tmp;
    LV_ASSERT_OBJ(obj, MY_CLASS);
    LV_ASSERT_NULL(dir);
    m_lv_file_select_t *f = (m_lv_file_select_t*)obj;
    strcpy(f->cur_dir,dir);

    tmp = f->cur_dir + strlen(f->cur_dir);

    if(*(tmp-1) != '/')
    {
        *(tmp) = '/';
    }

    if(filter != NULL)
    {
        strcpy(f->filter,filter);
    }
    else
    {
        f->filter[0] = 0;
    }

    f->mode = (uint8_t)mode;
    res = file_select_open(f);

    if(res != LV_FS_RES_OK)
    {
        return res;
    }

    lv_obj_clear_flag(obj,LV_OBJ_FLAG_HIDDEN);
    return res;
}

const char *m_lv_file_select_get_dir(lv_obj_t *obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    m_lv_file_select_t *f = (m_lv_file_select_t*)obj;
    return f->cur_dir;
}

const char *m_lv_file_select_read(lv_obj_t *obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    m_lv_file_select_t *f = (m_lv_file_select_t*)obj;
    return f->selct_str;
}

void m_lv_file_select_set_srot_flag(lv_obj_t *obj,bool flag)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    m_lv_file_select_t *f = (m_lv_file_select_t*)obj;
    f->srot_flag = flag;
}

void m_lv_file_select_set_srot(lv_obj_t *obj,bool(*fn)(const char*,const char*,bool))
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    m_lv_file_select_t *f = (m_lv_file_select_t*)obj;
    f->srot = fn;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
/**
 * @brief  constructor
 * @param  class_p
 * @param  obj
 */
static void file_select_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");
    lv_obj_t *lab;
    m_lv_file_select_t *f = (m_lv_file_select_t*)obj;
    lv_obj_add_flag(obj,LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *view = lv_obj_create(obj);
    lv_obj_set_size(view,LV_PCT(100),LV_PCT(100));
    lv_obj_set_flex_flow(view, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_flex_main_place(view,LV_FLEX_ALIGN_START,0);
    lv_obj_set_style_pad_right(view,0,0);
    lv_obj_set_style_pad_left(view,0,0);
    lv_obj_set_style_pad_top(view,10,0);
    lv_obj_set_style_pad_bottom(view,5,0);
    lv_obj_center(view);

    lv_obj_t *lab_box = lv_obj_create(view);
    lv_obj_set_size(lab_box,LV_PCT(100),LV_PCT(8));
    lv_obj_set_style_border_width(lab_box,0,0);
    lv_obj_set_flex_flow(lab_box, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(lab_box,LV_FLEX_ALIGN_SPACE_BETWEEN,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_radius(lab_box,0,0);
    lv_obj_set_style_pad_right(lab_box,35,0);
    lv_obj_set_style_pad_left(lab_box,35,0);
    lv_obj_set_scroll_dir(lab_box,LV_DIR_NONE);

    lab = lv_label_create(lab_box);
    lv_obj_add_flag(lab,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(lab,event_lab_back_cb,LV_EVENT_CLICKED,f);
    lv_label_set_text(lab,M_FILE_SELECT_ICON_BACK);
    lv_obj_center(lab);

    f->cur_dir_lab = lv_label_create(lab_box);

    f->ok_lab = lv_label_create(lab_box);
    lv_obj_add_flag(f->ok_lab,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(f->ok_lab,event_lab_ok_cb,LV_EVENT_CLICKED,f);
    lv_obj_center(f->ok_lab);


    f->table = lv_table_create(view);
    lv_obj_set_scroll_dir(f->table,LV_DIR_VER);
    lv_obj_set_size(f->table,LV_PCT(100),LV_PCT(90));
    //lv_obj_set_style_border_width(f->table,0,LV_PART_ITEMS);
    lv_obj_set_style_border_width(f->table,0,0);
    lv_obj_add_event_cb(f->table,event_all_cb,LV_EVENT_ALL,f);
    f->srot = sort_name_fn; /*Set the default sorting function*/
    f->srot_flag = true;    /*Default ascending order*/
}
/**
 * @brief  destructor
 * @param  class_p
 * @param  obj
 */
static void file_select_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    m_lv_file_select_t *f = (m_lv_file_select_t*)obj;
    free_node(f);

    if(f->selct_str != NULL)
        lv_mem_free(f->selct_str);
}


static void event_lab_ok_cb(lv_event_t *e)
{
    uint16_t i;
    uint16_t len;
    char  *tmp;
    m_lv_file_select_t *f = (m_lv_file_select_t*)lv_event_get_user_data(e);

    if(!f->select_flag)
    {
        lv_obj_add_flag((lv_obj_t*)f,LV_OBJ_FLAG_HIDDEN);
        free_node(f);

        if(f->selct_str != NULL)
            lv_mem_free(f->selct_str);

        return;
    }

    len = 0;

    for(i = 0; i< f->open_list_len; i++)
    {
        if(f->open_list[i]->selct)
        {
            len += strlen(f->open_list[i]->text) + 1;
        }
    }

    if(f->selct_str != NULL)
        lv_mem_free(f->selct_str);

    f->selct_str = lv_mem_alloc(len);

    if(f->selct_str == NULL)
    {
        LV_LOG_WARN("f->selct_str alloc failed!");
        return;
    }

    tmp = f->selct_str;

    for(i = 0; i< f->open_list_len; i++)
    {
        if(f->open_list[i]->selct)
        {
            strcpy(tmp,f->open_list[i]->text);
            tmp += strlen(tmp);
            *tmp = '\n';
            tmp ++;
        }
    }

    *(tmp-1) = '\0';
    lv_event_send((lv_obj_t*)f,LV_EVENT_READY,NULL);
    lv_obj_add_flag((lv_obj_t*)f,LV_OBJ_FLAG_HIDDEN);
}

static void event_lab_back_cb(lv_event_t *e)
{
    char str[M_FILE_SELECT_DIR_MAX_LEN];
    m_lv_file_select_t *f = (m_lv_file_select_t*)lv_event_get_user_data(e);

    stpcpy(str,f->cur_dir);
    str[strlen(str)-1] = 0;
    char *c = strrchr(str,'/');

    if(c != NULL)
    {
        c++;
        *c = 0;
        stpcpy(f->cur_dir,str);
        file_select_open(f);
    }
}

static void one_select(m_lv_file_select_t *f,selct_list_t *node)
{
    node->selct = true;
    f->selct_count = 1;
    lv_obj_add_flag((lv_obj_t*)f,LV_OBJ_FLAG_HIDDEN);

    if(f->selct_str != NULL)
        lv_mem_free(f->selct_str);

    f->selct_str = lv_mem_alloc(strlen(node->text)+1);

    if(f->selct_str== NULL)
    {
        LV_LOG_WARN("f->selct_str null!");
        return;
    }

    memcpy(f->selct_str,node->text,strlen(node->text)+1);
    lv_event_send((lv_obj_t*)f,LV_EVENT_READY,NULL);
}


static void  event_all_cb(lv_event_t *e)
{
    char *temp;
    uint16_t row;
    uint16_t col;
    selct_list_t *node;
    lv_obj_t *obj = lv_event_get_target(e);
    m_lv_file_select_t *f = (m_lv_file_select_t*)lv_event_get_user_data(e);

    if(lv_event_get_code(e) == LV_EVENT_DRAW_PART_BEGIN)
    {
        lv_obj_draw_part_dsc_t *draw = lv_event_get_draw_part_dsc(e);

        if(draw == NULL || LV_PART_ITEMS !=  draw->part || draw->id >= f->open_list_len)
        {
            return;
        }

        if(f->open_list[draw->id] != NULL && f->open_list[draw->id]->selct)
        {
            if( draw->rect_dsc != NULL)
                draw->rect_dsc->bg_color = lv_color_hex(M_FILE_SELECT_COLOR);

            if(draw->label_dsc != NULL)
                draw->label_dsc->color= lv_color_hex(M_FILE_SELECT_LABLE_COLOR);
        }
    }
    else if(lv_event_get_code(e) == LV_EVENT_LONG_PRESSED && f->selct_count == 0)
    {
        lv_table_get_selected_cell(obj,&row,&col);

        if(row >= f->open_list_len) return;

        node = f->open_list[row];

        if(f->mode & FILE_SELECT_MODE_DIR)/*View directory mode*/
        {
            if(f->mode & FILE_SELECT_MODE_MULTI)
            {
                f->select_flag  = true;
                lv_label_set_text(f->ok_lab,M_FILE_SELECT_ICON_OK);
                lv_obj_set_style_text_color(f->ok_lab,lv_color_hex(M_FILE_SELECT_COLOR),0);
            }
            else if(f->mode & FILE_SELECT_MODE_ONE) 
            {
                if(node) one_select(f,node);
            }
        }
        else if(!node->is_dir)
        {
            if(f->mode & FILE_SELECT_MODE_MULTI )
            {
                f->select_flag  = true;
                lv_label_set_text(f->ok_lab,M_FILE_SELECT_ICON_OK);
                lv_obj_set_style_text_color(f->ok_lab,lv_color_hex(M_FILE_SELECT_COLOR),0);
            }
            else if(f->mode & FILE_SELECT_MODE_ONE)
            {
                if(node) one_select(f,node);
            }
        }
    }
    else if(lv_event_get_code(e) == LV_EVENT_PRESSED)
    {
        lv_table_get_selected_cell(obj,&row,&col);
        f->row = row;
    }
    else if(lv_event_get_code(e) == LV_EVENT_CLICKED)
    {
        row= f->row;

        if(row >= f->open_list_len) return;

        node = f->open_list[row];

        if(f->select_flag)
        {
            if(f->mode & FILE_SELECT_MODE_DIR)
            {
                if(node->selct)
                {
                    f->selct_count --;
                    node->selct = false;
                }
                else
                {
                    node->selct = true;
                    f->selct_count ++;
                }
            }
            else if(!node->is_dir)
            {
                if(node->selct)
                {
                    f->selct_count --;
                    node->selct = false;
                }
                else
                {
                    node->selct = true;
                    f->selct_count ++;
                }
            }

            if(f->selct_count == 0)
            {
                lv_label_set_text(f->ok_lab,M_FILE_SELECT_ICON_CLOSE);
                lv_obj_set_style_text_color(f->ok_lab,lv_obj_get_style_text_color(f->table,0),0);
                f->select_flag = false;
            }
        }
        else if((f->mode &0xF0) == 0 && !node->is_dir)
        {
            if(f->selct_str != NULL)
            {
                if(strlen(f->selct_str) < strlen(node->text))
                {
                    lv_mem_free(f->selct_str);
                    f->selct_str = lv_mem_alloc(strlen(node->text) + 1);

                    if( f->selct_str == NULL)
                    {
                        LV_LOG_WARN("f->selct_str null!");
                        return;
                    }
                }
            }
            else
            {
                f->selct_str =  lv_mem_alloc(strlen(node->text) + 1);

                if( f->selct_str == NULL)
                {
                    LV_LOG_WARN("f->selct_str null!");
                    return;
                }
            }
            memcpy(f->selct_str,node->text,strlen(node->text)+1);
            lv_event_send((lv_obj_t*)f,LV_EVENT_READY,NULL);
        }
        else if(node->is_dir)
        {
            if(lv_obj_has_flag((lv_obj_t*)f,LV_OBJ_FLAG_HIDDEN))return;

            temp = f->cur_dir+strlen(f->cur_dir);
            lv_snprintf(temp,M_FILE_SELECT_DIR_MAX_LEN,"%s/",node->text);
            file_select_open(f);
        }
    }
}

static bool is_image(const char* fn)
{
    if(strcmp("jpg",fn) == 0 || strcmp("JPG",fn) == 0 ||
       strcmp("png",fn) == 0 || strcmp("PNG",fn) == 0 ||
       strcmp("gif",fn) == 0 || strcmp("GIF",fn) == 0 ||
       strcmp("bmp",fn) == 0 || strcmp("BMP",fn) == 0
      )
    {
        return true;
    }

    return false;
}


static bool is_video(const char *fn)
{
    if(strcmp("mp4",fn) == 0 || strcmp("MP4",fn) == 0 ||
       strcmp("3gp",fn) == 0 || strcmp("3GP",fn) == 0 ||
       strcmp("avi",fn) == 0 || strcmp("AVI",fn) == 0 ||
       strcmp("wmv",fn) == 0 || strcmp("WMV",fn) == 0
      )
    {
        return true;
    }

    return false;
}


static bool is_audio(const char *fn)
{
    if(strcmp("mp3",fn) == 0 || strcmp("MP3",fn) == 0 ||
       strcmp("wav",fn) == 0 || strcmp("WAV",fn) == 0
      )
    {
        return true;
    }

    return false;
}


static void table_show(m_lv_file_select_t *f)
{
    char *c;
    selct_list_t *node;

    for(uint16_t i = 0; i < f->open_list_len; i++)
    {
        node = f->open_list[i];
        node->id = i;

        if(node->is_dir)
        {
            lv_table_set_cell_value_fmt(f->table,i,0,"%s    %s",M_FILE_SELECT_ICON_DIR,node->text);
        }
        else
        {
            c = strrchr(node->text,'.');

            if(c == NULL)
            {
                lv_table_set_cell_value_fmt(f->table,i,0,"%s    %s",M_FILE_SELECT_ICON_FILE,node->text);
            }
            else
            {
                c++;

                if(is_image(c))
                {
                    lv_table_set_cell_value_fmt(f->table,i,0,"%s    %s",M_FILE_SELECT_ICON_IMAGE,node->text);
                }
                else if(is_audio(c))
                {
                    lv_table_set_cell_value_fmt(f->table,i,0,"%s    %s",M_FILE_SELECT_ICON_AUDIO,node->text);
                }
                else if(is_video(c))
                {
                    lv_table_set_cell_value_fmt(f->table,i,0,"%s    %s",M_FILE_SELECT_ICON_VIDEO,node->text);
                }
                else
                {
                    lv_table_set_cell_value_fmt(f->table,i,0,"%s    %s",M_FILE_SELECT_ICON_FILE,node->text);
                }
            }
        }
    }
    lv_obj_invalidate(f->table);
}


static  bool  file_filter(const char *filter,const char *fn)
{
    char *c;

    if(strlen(filter) == 0)return true;

    c = strrchr(fn,'.');

    if(c == NULL)return false;

    if(NULL != strstr(filter,c))
    {
        return true;
    }

    return false;
}


static selct_list_t * alloc_node(const char *fn)
{
    selct_list_t  *node = lv_mem_alloc(sizeof(selct_list_t));

    if(node == NULL)
    {
        LV_LOG_ERROR("lv_mem_alloc failed!");
        return NULL;
    }

    lv_memset_00(node,sizeof(selct_list_t));
    node->text = lv_mem_alloc(strlen(fn)+1);

    if(node->text == NULL)
    {
        lv_mem_free(node);
        LV_LOG_ERROR("lv_mem_alloc failed!");
        return NULL;
    }

    strcpy(node->text,fn);
    return node;
}


static void free_node(m_lv_file_select_t *f)
{
    for(uint16_t i = 0; i < f->open_list_len; i++)
    {
        if(f->open_list[i]->text)
            lv_mem_free(f->open_list[i]->text);

        if(f->open_list[i])
            lv_mem_free(f->open_list[i]);

        f->open_list[i] = NULL;
    }

    f->open_list_len = 0;
}

#ifdef M_FILE_SELECT_SUPPORT_STDIO_FS

static inline lv_fs_res_t stdio_fs_open_dir(m_lv_file_select_t *f)
{
    DIR *dir;
    struct  dirent *dp;
    selct_list_t *node;
    dir =  opendir(f->cur_dir);

    if(dir == NULL)
    {
        return LV_FS_RES_NOT_EX;
    }

    while(1)
    {
        dp =  readdir(dir);

        if(dp != NULL)
        {
            if(dp->d_type == DT_DIR)
            {
                if(strcmp(dp->d_name,".")== 0|| strcmp(dp->d_name,"..")== 0)
                {
                    continue;;
                }

                node = alloc_node(dp->d_name);

                if(node == NULL)
                {
                    closedir(dir);
                    return LV_FS_RES_UNKNOWN;
                }

                node->is_dir = true;
                f->open_list[f->open_list_len] = node;
            }
            else if(dp->d_type == DT_REG && (f->mode & 0x0F) == 0 && file_filter(f->filter,dp->d_name))
            {
                node = alloc_node(dp->d_name);

                if(node == NULL)
                {
                    closedir(dir);
                    return LV_FS_RES_UNKNOWN;
                }

                node->is_dir = false;
                f->open_list[f->open_list_len] = node;
            }
            else
            {
                continue;;
            }
        }
        else
        {
            break;
        }

        f->open_list_len ++;
    }

    closedir(dir);
    return LV_FS_RES_OK;
}
#endif /*M_FILE_SELECT_SUPPORT_STDIO_FS*/


#ifdef M_FILE_SELECT_SUPPORT_LVGL_FS

static  inline lv_fs_res_t  lvgl_fs_open_dir(m_lv_file_select_t *f)
{
    lv_fs_dir_t dir;
    lv_fs_res_t res;
    selct_list_t *node;
    char fn[256];
	
    res = lv_fs_dir_open(&dir,f->cur_dir);

    if(res != LV_FS_RES_OK) return res;

    while(1)
    {
        res = lv_fs_dir_read(&dir,fn);

        if(res != LV_FS_RES_OK)
        {
            lv_fs_dir_close(&dir);
            return res;
        }

        if(strlen(fn) != 0)
        {
            if(fn[0] == '/')
            {
                node = alloc_node(fn+1);

                if(node == NULL)
                {
                    lv_fs_dir_close(&dir);
                    return LV_FS_RES_UNKNOWN;
                }

                node->is_dir = true;
                f->open_list[f->open_list_len] = node;
            }
            else if((f->mode &(0x0F)) == 0 && file_filter(f->filter,fn))
            {
                node = alloc_node(fn);

                if(node == NULL)
                {
                    lv_fs_dir_close(&dir);
                    return LV_FS_RES_UNKNOWN;
                }

                node->is_dir = false;
                f->open_list[f->open_list_len] = node;
            }
            else
            {
                continue;;
            }

        }
        else
        {
            break;
        }

        f->open_list_len ++;
    }

    lv_fs_dir_close(&dir);
    return LV_FS_RES_OK;
}

#endif /*M_FILE_SELECT_SUPPORT_LVGL_FS*/

static bool  sort_name_fn(const char *str1,const char *str2,bool f)
{
    uint8_t tmp;
    char  *c;

    if((str1[0] > 0x2F && str1[0] < 0x3A) && (str2[0] > 0x2F && str2[0] < 0x3A))
    {

        if(atol(str1) > atol(str2))
        {
            return f;
        }
        else if(atol(str1) == atol(str2))
        {
            tmp = 0;
            c = str1;

            while((*c > 0x2F) && (*c < 0x3A))
            {
                c ++;
                tmp ++;
            }

            if(tmp)
            {
                if(strcmp(&str1[tmp],&str2[tmp]) > 0)
                    return f;
                else
                    return !f;
            }
            else
            {
                return !f;
            }
        }
        else
        {
            return !f;
        }
    }
    else if((str1[0] > 0x2F && str1[0] < 0x3A) || (str2[0] > 0x2F && str2[0] < 0x3A))
    {
        if(str1[0] > 0x2F && str1[0] < 0x3A)
            return !f;
        else
            return f;
    }
    else
    {
        if(strcmp(str1,str2) > 0)
            return f;
        else
            return !f;
    }

    return !f;
}


static void _quicksort(selct_list_t **list,uint16_t  left,uint16_t  right,bool(*fn)(const char*,const char*,bool),bool f)
{
    uint16_t  i,j;
    selct_list_t* key;
    key = list[left];
    i = left;
    j = right;

    while(i<j)
    {
        while(i < j && fn(list[j]->text,key->text,f))
        {
            j --;
        }

        list[i] = list[j];

        while(i < j && !fn(list[i]->text,key->text,f))
        {
            i++;
        }

        list[j] = list[i];
    }

    list[i] = key;

    if(i-1 > left)
    {
        _quicksort(list,left,i - 1,fn,f);
    }

    if(i + 1 < right)
    {
        _quicksort(list,i+1,right,fn,f);
    }
}


static void dir_file_sort(m_lv_file_select_t *f)
{
    selct_list_t **list_file;
    selct_list_t **list_dir;
    selct_list_t  **tmp;
    uint16_t  index_file = 0;
    uint16_t  index_dir = 0;
    list_file  = lv_mem_alloc(f->open_list_len * sizeof( selct_list_t*));

    if(list_file == NULL)
    {
        LV_LOG_WARN("null !");
        return;
    }

    list_dir  = lv_mem_alloc(f->open_list_len * sizeof( selct_list_t*));

    if(list_dir == NULL)
    {
        lv_mem_free(list_file);
        LV_LOG_WARN("null !");
        return;
    }

    lv_memset_00(list_file,f->open_list_len * sizeof( selct_list_t*));
    lv_memset_00(list_dir,f->open_list_len * sizeof( selct_list_t*));

    for(uint16_t i = 0; i < f->open_list_len; i++)
    {
        if(f->open_list[i]->is_dir)
        {
            list_dir[index_dir] = f->open_list[i];
            index_dir ++;
        }
        else
        {
            list_file[index_file] = f->open_list[i];
            index_file ++;
        }
    }


    if(index_dir >2)
        _quicksort(list_dir,0,index_dir-1,f->srot,f->srot_flag);

    if(index_file >2)
        _quicksort(list_file,0,index_file-1,f->srot,f->srot_flag);


    tmp =  f->open_list;

    for(uint8_t i = 0; i< index_dir; i++)
    {
        *tmp++ = list_dir[i];
    }

    for(uint8_t i = 0; i< index_file; i++)
    {
        *tmp++ = list_file[i];
    }

    lv_mem_free(list_file);
    lv_mem_free(list_dir);
}

static lv_fs_res_t file_select_open( m_lv_file_select_t *f)
{
    lv_fs_res_t res;
    lv_table_set_col_cnt(f->table,1);
    lv_table_set_row_cnt(f->table,0);
    lv_table_set_col_width(f->table,0,LV_PCT(100));
    lv_label_set_text(f->ok_lab,M_FILE_SELECT_ICON_CLOSE);
    lv_obj_set_style_text_color(f->ok_lab,lv_obj_get_style_text_color(f->table,0),0);
    f->select_flag = false;
    f->selct_count = 0;
    free_node(f);

#if (defined(M_FILE_SELECT_SUPPORT_LVGL_FS) && defined(M_FILE_SELECT_SUPPORT_STDIO_FS))

    if(strlen(f->cur_dir) > 2 && f->cur_dir[1] == ':')
    {
        res = lvgl_fs_open_dir(f);
    }
    else
    {
        res = stdio_fs_open_dir(f);
    }

#else
#ifdef  M_FILE_SELECT_SUPPORT_LVGL_FS
    res = lvgl_fs_open_dir(f);
#endif
#ifdef M_FILE_SELECT_SUPPORT_STDIO_FS
    res = stdio_fs_open_dir(f);
#endif
#endif /*(defined(M_FILE_SELECT_SUPPORT_LVGL_FS) && defined(M_FILE_SELECT_SUPPORT_STDIO_FS))*/

    if(res == LV_FS_RES_OK)
    {
        lv_event_send((lv_obj_t*)f,LV_EVENT_VALUE_CHANGED,NULL);
        dir_file_sort(f);
        table_show(f);
        lv_obj_scroll_to_y(f->table,0,0);
        lv_label_set_text_fmt(f->cur_dir_lab,"%s %s",LV_SYMBOL_EYE_OPEN,f->cur_dir);
    }

    return res;
}
#endif /*(defined(M_FILE_SELECT_SUPPORT_LVGL_FS) || defined(M_FILE_SELECT_SUPPORT_STDIO_FS))*/

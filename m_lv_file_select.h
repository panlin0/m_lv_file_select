/**
 * @file m_lv_file_select.h
 * @brief
 * @author PanLin (786005498@qq.com)
 * @version 1.0
 * @date 2023-10-09
 *
 * @copyright Copyright (c) 2023
 */

#ifndef _M_LV_FILE_SELECT_H_
#define _M_LV_FILE_SELECT_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include "../../lvgl/src/lv_conf_internal.h"
#include "../../lvgl/src/core/lv_obj.h"

/*whether the stdio universal file system is supported*/
#define  M_FILE_SELECT_SUPPORT_STDIO_FS 

#if (LV_USE_FS_STDIO|| LV_USE_FS_POSIX || LV_USE_FS_WIN32 ||LV_USE_FS_FATFS)
#define  M_FILE_SELECT_SUPPORT_LVGL_FS 
#endif


#define   M_FILE_SELECT_DIR_MAX_LEN       256   /*maximum length of the current path*/
#define   M_FILE_SELECT_FILTER_MAX_LEN    64    /*milter the maximum text length*/
#define   M_FILE_SELECT_OPEN_LIST_MAX_LEN 256   /*maximum length of the file list*/
#define   M_FILE_SELECT_COLOR           0x338fff /*select color*/
#define   M_FILE_SELECT_LABLE_COLOR     0xFFFFFF /*select lable color*/

/*icon*/
#define   M_FILE_SELECT_ICON_BACK     LV_SYMBOL_LEFT
#define   M_FILE_SELECT_ICON_CURDIR   LV_SYMBOL_EYE_OPEN
#define   M_FILE_SELECT_ICON_CLOSE    LV_SYMBOL_CLOSE
#define   M_FILE_SELECT_ICON_OK       LV_SYMBOL_OK
#define   M_FILE_SELECT_ICON_DIR      LV_SYMBOL_DIRECTORY
#define   M_FILE_SELECT_ICON_FILE     LV_SYMBOL_FILE
#define   M_FILE_SELECT_ICON_IMAGE    LV_SYMBOL_IMAGE
#define   M_FILE_SELECT_ICON_VIDEO    LV_SYMBOL_VIDEO
#define   M_FILE_SELECT_ICON_AUDIO    LV_SYMBOL_AUDIO
/**
 * @brief m_lv_file_select_mode_t
 * @note FILE_SELECT_MODE_ALL and FILE_SELECT_MODE_DIR: can only choose one。
 * @note FILE_SELECT_MODE_ONE and FILE_SELECT_MODE_MULTI: can only choose one。
 * @note FILE_SELECT_MODE_ALL: cannot select a folder, only a file。
 */
typedef enum
{
    FILE_SELECT_MODE_ALL = 0,          /*view directories and files*/
    FILE_SELECT_MODE_DIR = 1,          /*view file directory*/
    FILE_SELECT_MODE_ONE   = (1 << 4), /*Single choice mode*/
    FILE_SELECT_MODE_MULTI = (1 << 5), /*multiple choice mode*/
} m_lv_file_select_mode_t;


typedef struct
{
    uint16_t id;
    char     *text;
    bool      is_dir;
    bool      selct;
} selct_list_t;

typedef struct
{
    lv_obj_t  obj;
    lv_obj_t  *table;
    lv_obj_t  *cur_dir_lab;
    lv_obj_t  *ok_lab;
    char       cur_dir[M_FILE_SELECT_DIR_MAX_LEN];
    char       filter[M_FILE_SELECT_FILTER_MAX_LEN];
    uint16_t   row;
    uint16_t   selct_count;
    uint16_t        open_list_len;
    char           *selct_str;
    selct_list_t   *open_list[M_FILE_SELECT_OPEN_LIST_MAX_LEN];
    bool (*srot)(const char*,const char*,bool);
    bool      srot_flag;
    uint8_t   mode;
    bool      select_flag;
} m_lv_file_select_t;

/**
 * @brief  create a file selector
 * @param  parent    
 * @return lv_obj_t *
 */
lv_obj_t *m_lv_file_select_create(lv_obj_t *parent);

/**
 * @brief  open the file selector
 * @param  obj    file selector pointer
 * @param  dir    the directory to open
 * @param  mode   open mode
 * @param  filter NULL views all files and enters a suffix for the file if filtering is required, for example :".A.O.xxx ".
 * @return LV_FS_RES_OK or lv_fs_res_t enumeration。
 */
lv_fs_res_t m_lv_file_select_open(lv_obj_t *obj, const char *dir, m_lv_file_select_mode_t mode, const char *filter);


/**
 * @brief  gets current directory
 * @param  obj   file selector pointer
 * @return current directory
 */
const char *m_lv_file_select_get_dir(lv_obj_t *obj);


/**
 * @brief  reads the current selection list
 * @param  obj     file selector pointer
 * @return returns a list of strings separated by '\n'.
 */
const char *m_lv_file_select_read(lv_obj_t *obj);

/**
 * @brief  set the ascending or descending order
 * @param  obj    file selector pointer
 * @param  flag   true:ascending | false:descending
 */
void m_lv_file_select_set_srot_flag(lv_obj_t *obj,bool flag);

/**
 * @brief  set custom sorting rules
 * @param  obj    file selector pointer
 * @param  fn    function
 * @note   fn parame0:The string 1 to be compared
 * @note   fn parame1:The string 2 to be compared
 * @note   fn parame2:true:ascending | false:descending
 * @note   fn return: swap:ture | false
 */
void m_lv_file_select_set_srot(lv_obj_t *obj,bool(*fn)(const char*,const char*,bool));

#ifdef __cplusplus
} /*extern "C"*/
#endif
#endif /*_M_LV_FILE_SELECT_H_*/

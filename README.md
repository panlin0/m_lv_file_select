# LVGL自定义控 多文件选择器

#### 介绍
基于LVGL的自定义组件文件选择器，类似windos可以打开指定的路径并选择多个文件或多个路径，接口简单只有6个。
Lvgl-based custom component file selector, similar to windos can open the specified path and select multiple files or multiple paths, 
the interface is simple only 6.
#### 支持  
1. LVGL的文件系统/stdio标准文件系统系统接口,可同时使用(单片机请注释m_lv_file_select.h文件的M_FILE_SELECT_SUPPORT_STDIO_FS)。  
2. 可选择单/多个文件或目录。 
3. 可自定义排序规则，默认以文件名排序。
4. 支持后缀筛选,例如 ".jpg .bmp .png",参数是m_lv_file_select_open的*filter。 
5. 可设置升降序。  
6. 自定义选中颜色/界面图标(需要字库有)。
   
The LVGL file system /stdio standard file system interface can be used at the same time.
    (For microcontrollers, note M_FILE_SELECT_SUPPORT_STDIO_FS in the m_lv_file_select.h file.)
can select one or more files or directories.
The sorting rules can be customized.  By default, the sorting rules are sorted by file name.
Support suffix filtering, for example, ".jpg.  bmp.  png".  The parameter is m_lv_file_select_open *filter.
can set an ascending or descending sequence.
Customize the selected color/interface icon (required font).

#### 事件    
1. LV_EVENT_VALUE_CHANGED     
打开一个目录时产生此事件，可通过m_lv_file_select_get_dir获取当前路径。
This event is generated when a directory is opened. Run m_lv_file_select_get_dir to obtain the current path. 

3. LV_EVENT_READY      
	2.1 FILE_SELECT_MODE_ALL:      
    文件浏览模式 可通过m_lv_file_select_read获取当前点击的文件，点击目录不产生此事件（不会关闭窗口）。
In file browsing mode, you can run m_lv_file_select_read to obtain the file you clicked on.
This event is not generated when you click the directory (the window is not closed).
	2.2 FILE_SELECT_MODE_XXX|FILE_SELECT_MODE_ONE/MULTI   
    单选/多选模式 可通过m_lv_file_select_read读取选中的文件或路径列表，以'\n'分隔（自动关闭窗口）。   
In single/multiple mode, m_lv_file_select_read reads the list of selected files or paths separated by '\n' (automatically closes the window).
	
#### 操作方法   
1. 单选模式：长按选中，自动关闭窗口，通过LV_EVENT_READY事件读取当前路径和文件列表。
   Single option mode: Press and hold to close the window. The LV_EVENT_READY event is used to read the current path and file list.
3. 多选模式：长按进入多选模式，点击选中，再次点击取选，最后点击右上角✔完成选中，通过LV_EVENT_READY事件读取当前路径和文件列表。 
    Multiple Selection mode: Long press to enter multiple selection mode, click Select, and finally click in the upper right corner ✔.
    Read the current path and file list through LV_EVENT_READY event.

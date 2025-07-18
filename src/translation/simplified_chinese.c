#include "translation/common.h"
#include "translation/translation.h"

static translation_string all_strings[] = {
    {TR_NO_PATCH_TITLE, "未安装1.0.1.0补丁"},
    {TR_NO_PATCH_MESSAGE,
        "《凯撒大帝3》未安装1.0.1.0补丁。"
        "补丁下载链接:\n"
        URL_PATCHES "\n"
        "继续游戏风险自担。"},
    {TR_MISSING_FONTS_TITLE, "缺失字体"},
    {TR_MISSING_FONTS_MESSAGE,
        "《凯撒大帝3》需要额外字体文件。"
        "语言包下载链接:\n"
        URL_PATCHES},
    {TR_NO_EDITOR_TITLE, "未安装地图编辑器"},
    {TR_NO_EDITOR_MESSAGE,
        "《凯撒大帝3》未检测到地图编辑器文件。"
        "地图编辑器下载链接:\n"
        URL_EDITOR},
    {TR_INVALID_LANGUAGE_TITLE, "语言包路径无效"},
    {TR_INVALID_LANGUAGE_MESSAGE,
        "指定路径未检测到有效语言包。"
        "请检视日志查看错误。"},
    {TR_BUILD_ALL_TEMPLES, "全部"},
    {TR_BUTTON_OK, "确定"},
    {TR_BUTTON_CANCEL, "取消"},
    {TR_BUTTON_RESET_DEFAULTS, "重置默认"},
    {TR_BUTTON_CONFIGURE_HOTKEYS, "热键绑定"},
    {TR_CONFIG_TITLE, "Julius 游戏配置"},
    {TR_CONFIG_LANGUAGE_LABEL, "语言包:"},
    {TR_CONFIG_LANGUAGE_DEFAULT, "默认"},
    {TR_CONFIG_DISPLAY_SCALE, "显示比例:"},
    {TR_CONFIG_CURSOR_SCALE, "鼠标比例:"},
    {TR_CONFIG_HEADER_UI_CHANGES, "用户界面更变"},
    {TR_CONFIG_HEADER_GAMEPLAY_CHANGES, "游戏内容更变"},
    {TR_CONFIG_SHOW_INTRO_VIDEO, "播放开场动画"},
    {TR_CONFIG_SIDEBAR_INFO, "控制面板更多信息"},
    {TR_CONFIG_SMOOTH_SCROLLING, "平滑视角滚动"},
    {TR_CONFIG_DISABLE_MOUSE_EDGE_SCROLLING, "禁用屏幕边缘滚动地图"},
    {TR_CONFIG_DISABLE_RIGHT_CLICK_MAP_DRAG, "禁用鼠标右键拖动地图"},
    {TR_CONFIG_INVERSE_RIGHT_CLICK_MAP_DRAG, "用鼠标右键反向拖动地图"},
    {TR_CONFIG_VISUAL_FEEDBACK_ON_DELETE, "优化清理土地视觉效果"},
    {TR_CONFIG_ALLOW_CYCLING_TEMPLES, "允许连续建造每种神庙"},
    {TR_CONFIG_SHOW_WATER_STRUCTURE_RANGE, "建造时显示贮水池水泉及水井覆盖范围"},
    {TR_CONFIG_SHOW_CONSTRUCTION_SIZE, "显示拖动建设大小"},
    {TR_CONFIG_HIGHLIGHT_LEGIONS, "鼠标悬停时高亮军团"},
    {TR_CONFIG_SHOW_MILITARY_SIDEBAR, "显示军队信息侧栏"},
    {TR_CONFIG_FIX_IMMIGRATION_BUG, "修复非常困难不来人BUG"},
    {TR_CONFIG_FIX_100_YEAR_GHOSTS, "修复人口百岁仍占房BUG"},
    {TR_HOTKEY_TITLE, "Julius 热键绑定"},
    {TR_HOTKEY_LABEL, "热键"},
    {TR_HOTKEY_ALTERNATIVE_LABEL, "可替代键"},
    {TR_HOTKEY_HEADER_ARROWS, "方向键"},
    {TR_HOTKEY_HEADER_GLOBAL, "全局热键"},
    {TR_HOTKEY_HEADER_CITY, "城市热键"},
    {TR_HOTKEY_HEADER_ADVISORS, "顾问"},
    {TR_HOTKEY_HEADER_OVERLAYS, "覆层"},
    {TR_HOTKEY_HEADER_BOOKMARKS, "城市地图视角标签"},
    {TR_HOTKEY_HEADER_EDITOR, "编辑器"},
    {TR_HOTKEY_HEADER_BUILD, "建造热键"},
    {TR_HOTKEY_ARROW_UP, "上"},
    {TR_HOTKEY_ARROW_DOWN, "下"},
    {TR_HOTKEY_ARROW_LEFT, "左"},
    {TR_HOTKEY_ARROW_RIGHT, "右"},
    {TR_HOTKEY_TOGGLE_FULLSCREEN, "全屏切换"},
    {TR_HOTKEY_CENTER_WINDOW, "中心窗口化"},
    {TR_HOTKEY_RESIZE_TO_640, "重置分辨率至640x480"},
    {TR_HOTKEY_RESIZE_TO_800, "重置分辨率至800x600"},
    {TR_HOTKEY_RESIZE_TO_1024, "重置分辨率至1024x768"},
    {TR_HOTKEY_SAVE_SCREENSHOT, "保存截图"},
    {TR_HOTKEY_SAVE_CITY_SCREENSHOT, "保存城市全景截图"},
    {TR_HOTKEY_BUILD_CLONE, "复制鼠标下的建筑"},
    {TR_HOTKEY_LOAD_FILE, "载入文件"},
    {TR_HOTKEY_SAVE_FILE, "保存文件"},
    {TR_HOTKEY_INCREASE_GAME_SPEED, "加快游戏速度"},
    {TR_HOTKEY_DECREASE_GAME_SPEED, "减慢游戏速度"},
    {TR_HOTKEY_TOGGLE_PAUSE, "暂停切换"},
    {TR_HOTKEY_CYCLE_LEGION, "切换各军团所在视角"},
    {TR_HOTKEY_ROTATE_MAP_LEFT, "顺时针旋转地图视角"},
    {TR_HOTKEY_ROTATE_MAP_RIGHT, "逆时针旋转地图视角"},
    {TR_HOTKEY_SHOW_ADVISOR_LABOR, "劳工顾问"},
    {TR_HOTKEY_SHOW_ADVISOR_MILITARY, "军事顾问"},
    {TR_HOTKEY_SHOW_ADVISOR_IMPERIAL, "皇帝顾问"},
    {TR_HOTKEY_SHOW_ADVISOR_RATINGS, "评比顾问"},
    {TR_HOTKEY_SHOW_ADVISOR_TRADE, "贸易顾问"},
    {TR_HOTKEY_SHOW_ADVISOR_POPULATION, "人口顾问"},
    {TR_HOTKEY_SHOW_ADVISOR_HEALTH, "健康顾问"},
    {TR_HOTKEY_SHOW_ADVISOR_EDUCATION, "教育顾问"},
    {TR_HOTKEY_SHOW_ADVISOR_ENTERTAINMENT, "娱乐顾问"},
    {TR_HOTKEY_SHOW_ADVISOR_RELIGION, "宗教顾问"},
    {TR_HOTKEY_SHOW_ADVISOR_FINANCIAL, "财政顾问"},
    {TR_HOTKEY_SHOW_ADVISOR_CHIEF, "首席顾问"},
    {TR_HOTKEY_TOGGLE_OVERLAY, "当前覆层视角切换"},
    {TR_HOTKEY_SHOW_OVERLAY_WATER, "显示供水覆层"},
    {TR_HOTKEY_SHOW_OVERLAY_FIRE, "显示火灾覆层"},
    {TR_HOTKEY_SHOW_OVERLAY_DAMAGE, "显示损坏覆层"},
    {TR_HOTKEY_SHOW_OVERLAY_CRIME, "显示犯罪覆层"},
    {TR_HOTKEY_SHOW_OVERLAY_PROBLEMS, "显示问题覆层"},
    {TR_HOTKEY_GO_TO_BOOKMARK_1, "前往视角标签 1"},
    {TR_HOTKEY_GO_TO_BOOKMARK_2, "前往视角标签 2"},
    {TR_HOTKEY_GO_TO_BOOKMARK_3, "前往视角标签 3"},
    {TR_HOTKEY_GO_TO_BOOKMARK_4, "前往视角标签 4"},
    {TR_HOTKEY_SET_BOOKMARK_1, "设定视角标签 1"},
    {TR_HOTKEY_SET_BOOKMARK_2, "设定视角标签 2"},
    {TR_HOTKEY_SET_BOOKMARK_3, "设定视角标签 3"},
    {TR_HOTKEY_SET_BOOKMARK_4, "设定视角标签 4"},
    {TR_HOTKEY_EDITOR_TOGGLE_BATTLE_INFO, "战斗信息切换"},
    {TR_HOTKEY_EDIT_TITLE, "按下新热键"},
    {TR_HOTKEY_DUPLICATE_TITLE, "热键已占用"},
    {TR_HOTKEY_DUPLICATE_MESSAGE, "该键位已设定为以下功能:"},
    {TR_WARNING_SCREENSHOT_SAVED, "截图已保存: "}, // TODO: Google translate
};

void translation_simplified_chinese(const translation_string **strings, int *num_strings)
{
    *strings = all_strings;
    *num_strings = sizeof(all_strings) / sizeof(translation_string);
}

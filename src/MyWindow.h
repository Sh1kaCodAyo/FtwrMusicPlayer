//
// Created by ftwrjh on 2026/11/11.
//

#pragma once
#include "Playlist.h"
#include <gst/gst.h>
#include <gtkmm.h>
#include "ConfigManager.h"

class MyWindow : public Gtk::Window {
public:
    MyWindow();
    ~MyWindow();

protected:
    static gboolean on_gst_message(GstBus *bus, GstMessage *msg, gpointer data);
    void init();
    void bind_events();

    void on_btn_choose_file();
    void choose_playlist();

    void load_m3u(const std::string &m3u_path);
    void on_row_activated(const Gtk::TreeModel::Path &path,
                            Gtk::TreeViewColumn *column);
    void play_file(const std::string &filepath);
    void toggle_play_pause();
    void seek();
    void seek_to(int seconds);
    void on_song_finished();

    void open_website();

private:
    // 界面组件
    // Gtk::MenuItem* item_setworkspace_;
    Gtk::MenuBar menubar_;
    Gtk::Entry entry_volume_;
    Gtk::HScale volume_scale_{0.0, 1.5, 0.01}; // 构造函数参数：最小值, 最大值, 步长
    Gtk::Entry entry_seekto_;
    Gtk::Button btn_choose_file_;
    Gtk::Button btn_pause_;
    Gtk::Button btn_seek_;

    Playlist columns_;
    Glib::RefPtr<Gtk::ListStore> list_store_;
    Gtk::TreeView tree_view_;
    Glib::RefPtr<Gtk::TreeModelSort> sort_model_; // 新增排序模型（可选，但推荐存下来方便以后操作）

    Gtk::Statusbar statusbar_;

    // 播放器相关
    GstElement *playbin_ = nullptr;
    bool is_playing_ = false;
    
    // 配置文件管理
    std::string get_config_path();
    ConfigManager config_{get_config_path()};
};

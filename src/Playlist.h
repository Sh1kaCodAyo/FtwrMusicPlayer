//
// Created by ftwrjh on 2026/11/11.
//

#pragma once
#include <gtkmm/treemodel.h>

class Playlist : public Gtk::TreeModel::ColumnRecord {
public:
  Playlist() {
    add(col_title_);
    add(col_path_);
  }

  Gtk::TreeModelColumn<Glib::ustring> col_title_; // 显示的歌名
  Gtk::TreeModelColumn<Glib::ustring> col_path_;  // 真实的文件绝对路径
};

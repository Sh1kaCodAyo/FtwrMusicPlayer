#include <gtkmm.h>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <thread>
#include <gst/gst.h>


class MyWindow : public Gtk::Window
{
public:
    MyWindow()
    {
        // 窗口基本样式
        set_title("FtwrMusicPlayer");
        set_default_size(400, 400);
        set_border_width(5);
        // set_keep_above(true);

        // ====== 0. 主布局 ======
        auto* main_box_v = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 10));

        // ====== 1. 菜单栏 ======
        // 文件菜单
        auto* item_file = Gtk::manage(new Gtk::MenuItem("文件"));
        auto* menu_file = Gtk::manage(new Gtk::Menu());
        item_file->set_submenu(*menu_file);

        item_setworkspace_ = Gtk::manage(new Gtk::MenuItem("打开歌单.."));
        menu_file->append(*item_setworkspace_);

        menubar_.append(*item_file);
        main_box_v->pack_start(menubar_, Gtk::PACK_SHRINK);


        auto* edit_box_h = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 6));

        auto* label_volume = Gtk::manage(new Gtk::Label("音量"));
        auto* label_seek = Gtk::manage(new Gtk::Label("跳转"));
        edit_box_h->pack_start(*label_volume, Gtk::PACK_EXPAND_WIDGET);
        // edit_box_h->pack_start(entry_volume_, Gtk::PACK_EXPAND_WIDGET);


        // 在构造函数里配置滑块
        volume_scale_.set_value(1.0);      // 默认音量 100%
        volume_scale_.set_draw_value(false); // 不显示具体数字（可选）
        volume_scale_.set_size_request(100, -1); // 可选：设置一下宽度
        edit_box_h->pack_start(volume_scale_, Gtk::PACK_SHRINK);

        edit_box_h->pack_start(*label_seek, Gtk::PACK_EXPAND_WIDGET);
        edit_box_h->pack_start(entry_seekto_, Gtk::PACK_EXPAND_WIDGET);
        main_box_v->pack_start(*edit_box_h, Gtk::PACK_SHRINK);

        // ====== 3. 按钮区 ======
        auto* button_box_h = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 6));
        btn_choose_file_.set_label("选择文件");
        btn_pause_.set_label("播放/暂停");
        btn_seek_.set_label("跳转");

        button_box_h->set_homogeneous(true);
        button_box_h->pack_start(btn_choose_file_, Gtk::PACK_EXPAND_WIDGET);
        button_box_h->pack_start(btn_pause_, Gtk::PACK_EXPAND_WIDGET);
        button_box_h->pack_start(btn_seek_, Gtk::PACK_EXPAND_WIDGET);
        main_box_v->pack_start(*button_box_h, Gtk::PACK_SHRINK);



                // 1. 实例化成员变量 base_store
        list_store_ = Gtk::ListStore::create(columns_);
        // 2. 创建排序模型，并绑定底层 ListStore
        sort_model_ = Gtk::TreeModelSort::create(list_store_);
        // sort_model_->set_sort_column(columns_.col_title, Gtk::SORT_DESCENDING);
        // 3. 把排序模型交给 TreeView
        tree_view_.set_model(sort_model_);

        // 1. 手动创建列对象，并绑定模型数据
        auto* col1 = Gtk::manage(new Gtk::TreeViewColumn("标题", columns_.col_title));
        auto* col2 = Gtk::manage(new Gtk::TreeViewColumn("路径", columns_.col_path));
        // auto* col3 = Gtk::manage(new Gtk::TreeViewColumn("权重", columns_.col_weight));
        // auto* col4 = Gtk::manage(new Gtk::TreeViewColumn("来源", columns_.col_source));
        // auto* col5 = Gtk::manage(new Gtk::TreeViewColumn("行号", columns_.col_linenumber));

        // 2. 设置自动拉伸，让三列均匀铺满宽度
        // 2. 设置各列的比例 (词条:编码:权重:来源:行号 = 1:1.5:1:1:1)
        // 基础宽度设定为 100，编码列设为 150
        col1->set_min_width(100);
        col2->set_min_width(150); // 这列较宽
        // col3->set_min_width(100);
        // col4->set_min_width(100);
        // col5->set_min_width(100);

        // 依然需要开启 expand 让它们能够分配窗口拉大时多出来的剩余空间
        col1->set_expand(true);
        col2->set_expand(true);
        // col3->set_expand(true);
        // col4->set_expand(true);
        // col5->set_expand(true);

        // 3. 把列追加到表格视图
        tree_view_.append_column(*col1);
        tree_view_.append_column(*col2);
        // tree_view_.append_column(*col3);
        // tree_view_.append_column(*col4);
        // tree_view_.append_column(*col5);

        auto* scrolled = Gtk::manage(new Gtk::ScrolledWindow());
        scrolled->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
        scrolled->set_min_content_height(120);
        scrolled->add(tree_view_);

        // 关键：让表格撑满中间区域
        main_box_v->pack_start(*scrolled, Gtk::PACK_EXPAND_WIDGET);


        // 在构造函数里绑定
        tree_view_.signal_row_activated().connect(sigc::mem_fun(*this, &MyWindow::on_row_activated));


        // ====== 添加右键菜单 ======
        auto* item_edit_weight = Gtk::manage(new Gtk::MenuItem("修改权重"));
        auto* item_delete = Gtk::manage(new Gtk::MenuItem("删除词条"));

        // 绑定菜单项点击事件
        // item_edit_weight->signal_activate().connect(sigc::mem_fun(*this, &MyWindow::on_menu_edit_weight));
        // item_delete->signal_activate().connect(sigc::mem_fun(*this, &MyWindow::on_menu_delete));

        // // 把菜单项加入菜单
        // right_click_menu_.append(*item_edit_weight);
        // right_click_menu_.append(*item_delete);
        // right_click_menu_.show_all(); // 必须 show，否则不显示
        //
        // // 允许 TreeView 接收鼠标点击事件
        // // tree_view_.add_events(Gdk::BUTTON_PRESS_MASK);
        // // tree_view_.signal_button_press_event().connect(sigc::mem_fun(*this, &MyWindow::on_tree_view_button_press));
        // tree_view_.add_events(Gdk::BUTTON_PRESS_MASK);
        // // 加上参数 false，表示在默认的信号处理（比如选中行）之前拦截事件
        // tree_view_.signal_button_press_event().connect([this](GdkEventButton* event)
        // {
        //     return on_tree_view_button_press(event);
        // }, false);





        main_box_v->pack_end(statusbar_, Gtk::PACK_SHRINK);
        add_events(Gdk::KEY_PRESS_MASK);

        bind_events();
        init();

        add(*main_box_v);
        show_all_children();
    }
    // 【新增】：析构函数
    ~MyWindow() override {
        if (playbin_) {
            // 1. 将播放器状态设为 NULL，这会停止播放并释放声卡设备
            gst_element_set_state(playbin_, GST_STATE_NULL);

            // 2. 减少引用计数，彻底销毁这个对象
            gst_object_unref(playbin_);
            playbin_ = nullptr;
        }
    }

protected:
    // 这是一个静态函数，因为 gst_bus_add_watch 需要 C 风格的函数指针
    static gboolean on_gst_message(GstBus* bus, GstMessage* msg, gpointer data) {
        // 把传进来的 data 转换回我们的主窗口对象
        MyWindow* self = static_cast<MyWindow*>(data);

        // 判断消息类型是否是 EOS（End of Stream）
        if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS) {
            // 如果是，调用主窗口的“播放下一首”逻辑
            self->on_song_finished();
        }
        // 还可以顺便拦截错误消息
        else if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
            GError* err = nullptr;
            gchar* debug_info = nullptr;
            gst_message_parse_error(msg, &err, &debug_info);
            std::cerr << "播放出错: " << err->message << std::endl;
            g_clear_error(&err);
            g_free(debug_info);
        }

        return TRUE; // 返回 TRUE 表示继续监听后续的消息
    }
    void on_song_finished() {
        // 1. 获取当前选中的行
        Glib::RefPtr<Gtk::TreeSelection> selection = tree_view_.get_selection();
        Gtk::TreeModel::iterator model = selection->get_selected();

        if (!model) return; // 如果没选中，直接返回

        // 2. 拿到底层 ListStore 的路径
        auto sort_model = Glib::RefPtr<Gtk::TreeModelSort>::cast_dynamic(tree_view_.get_model());
        Gtk::TreeModel::Path path;

        if (sort_model) {
            // 如果你用了 TreeModelSort（像词库工具那样），需要转换路径
            // path = sort_model->convert_child_path_to_path(list_store_->get_path(model));
            // 【关键修复】先拿到排序模型里的路径
            Gtk::TreeModel::Path sort_path = sort_model->get_path(model);

            // 将排序模型的路径，转换为底层 ListStore 的路径
            path = sort_model->convert_path_to_child_path(sort_path);
        } else {
            path = list_store_->get_path(model);
        }

        // 3. 找到下一行（模拟用户点下一行）
        if (!path.empty()) {
            path.next(); // 路径移到下一行
            auto next_iter = list_store_->get_iter(path);

            if (next_iter) {
                // 如果存在下一首，选中它并播放
                tree_view_.get_selection()->select(next_iter);
                std::string next_path = (Glib::ustring)(*next_iter)[columns_.col_path];
                play_file(next_path);
            } else {
                // 如果已经是最后一首了，可以：
                // 1. 停止播放
                // 2. 或者循环回第一首（列表的第一行）
                statusbar_.push("歌单播放完毕！", 0);

                // 如果想循环播放：
                // auto first_iter = list_store_->children().begin();
                // tree_view_.get_selection()->select(first_iter);
                // play_file((Glib::ustring)(*first_iter)[columns_.col_path]);
            }
        }
    }
    void bind_events()
    {
        // menuitem
        item_setworkspace_->signal_activate().connect(sigc::mem_fun(*this, &MyWindow::choose_playlist));
        // button
        btn_choose_file_.signal_clicked().connect(sigc::mem_fun(*this, &MyWindow::on_btn_choose_file));
        btn_pause_.signal_clicked().connect(sigc::mem_fun(*this, &MyWindow::toggle_play_pause));
        btn_seek_.signal_clicked().connect(sigc::mem_fun(*this, &MyWindow::seek));
        // scale
        volume_scale_.signal_value_changed().connect([this]() {
            if (playbin_) {
                // 1. 获取当前滑块的值
                double current_volume = volume_scale_.get_value();

                // 2. 利用 g_object_set 修改 playbin 的 volume 属性
                g_object_set(playbin_, "volume", current_volume, nullptr);
            }
        });
    }
    void init()
    {
        // 初始化 GStreamer
        gst_init(nullptr, nullptr);
        // 创建播放引擎 (playbin 是 GStreamer 最万能的播放器组件)
        playbin_ = gst_element_factory_make("playbin", "player");
        if (!playbin_) {
            std::cerr << "创建 GStreamer 播放器失败！" << std::endl;
        }
        // 假设你在构造函数里已经创建了 playbin_
        // playbin_ = gst_element_factory_make("playbin", "player");

        // 【核心代码】获取总线并挂载监听
        GstBus* bus = gst_element_get_bus(playbin_);
        gst_bus_add_watch(bus, on_gst_message, this);
        gst_object_unref(bus); // 挂载完就可以释放本地的引用了
    }
    void on_btn_choose_file()
    {

        // 1. 创建文件选择对话框
        Gtk::FileChooserDialog dialog(*this, "请选择 Rime 工作目录");
        dialog.set_transient_for(*this); // 设置父窗口
        // 假设 dialog 是 Gtk::FileChooserDialog 的实例
        dialog.set_current_folder("~");

        // 2. 添加按钮（取消和打开）
        dialog.add_button("_取消", Gtk::RESPONSE_CANCEL);
        dialog.add_button("_选择", Gtk::RESPONSE_OK);

        // 3. 关键：设置为只能选择文件夹（不能选文件）
        dialog.set_action(Gtk::FILE_CHOOSER_ACTION_OPEN);

        // 4. 显示对话框，并阻塞等待用户操作
        const int result = dialog.run();

        // 5. 处理用户的选择结果
        if (result == Gtk::RESPONSE_OK)
        {
            std::string file_path = dialog.get_filename();

            play_file(file_path);

            // 更新底部状态栏提示
            statusbar_.push("已选择文件: " + file_path, 0);
        }
        else
        {
            statusbar_.push("已取消选择目录", 0);
        }
    }
    void choose_playlist()
    {
        // 1. 创建文件选择对话框
        Gtk::FileChooserDialog dialog(*this, "请选择歌单文件");
        dialog.set_transient_for(*this); // 设置父窗口

        // 2. 添加按钮（取消和打开）
        dialog.add_button("_取消", Gtk::RESPONSE_CANCEL);
        dialog.add_button("_选择", Gtk::RESPONSE_OK);

        // 【核心代码】获取系统的“音乐”目录
        std::string music_dir = Glib::get_user_special_dir(Glib::UserDirectory::USER_DIRECTORY_MUSIC);

        // 容错处理：如果系统没有配置音乐目录（极少见），就退回默认
        if (music_dir.empty()) {
            music_dir = Glib::get_home_dir();
        }

        // 设置默认目录
        dialog.set_current_folder(music_dir);

        // 3. 关键：设置为只能选择文件夹（不能选文件）
        dialog.set_action(Gtk::FILE_CHOOSER_ACTION_OPEN);

        // 4. 显示对话框，并阻塞等待用户操作
        const int result = dialog.run();

        // 5. 处理用户的选择结果
        if (result == Gtk::RESPONSE_OK)
        {
            // 获取用户选中的文件夹路径
            std::string folder_path = dialog.get_filename();

            load_m3u(folder_path);

        }
        else
        {
            // statusbar_.push("已取消选择目录", 0);
        }
    }
    void load_m3u(const std::string& m3u_path) {
        std::ifstream file(m3u_path);
        if (!file.is_open()) {
            statusbar_.push("无法打开歌单文件！", 0);
            return;
        }

        // 1. 清空现有列表
        list_store_->clear();

        std::string line;
        std::string current_title;
        // 获取 M3U 所在目录，用于处理相对路径
        std::filesystem::path m3u_dir = std::filesystem::path(m3u_path).parent_path();

        while (std::getline(file, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back(); // 兼容 Windows 换行

            if (line.empty()) continue;

            // 处理扩展信息：解析 #EXTINF
            if (line.rfind("#EXTINF:", 0) == 0) {
                // 格式通常是 #EXTINF:时长,歌手 - 歌名
                size_t comma_pos = line.find(',');
                if (comma_pos != std::string::npos) {
                    current_title = line.substr(comma_pos + 1); // 提取逗号后面的名字
                }
                continue;
            }

            // 跳过其他注释
            if (line[0] == '#') continue;

            // 剩下的就是真正的音频路径
            std::filesystem::path audio_path(line);
            if (audio_path.is_relative()) {
                audio_path = m3u_dir / audio_path; // 如果是相对路径，拼成绝对路径
            }

            // 如果没有 EXTINF 标签，就用文件名当歌名
            if (current_title.empty()) {
                current_title = audio_path.filename().string();
            }

            // 塞入列表
            auto row = *(list_store_->append());
            row[columns_.col_title] = current_title;
            row[columns_.col_path] = audio_path.string();

            current_title.clear(); // 重置，准备读下一首
        }

        statusbar_.push("歌单加载成功！", 0);
    }

    void on_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column) {
        auto iter = list_store_->get_iter(path);
        if (iter) {
            auto row = *iter;
            std::string filepath = (Glib::ustring)row[columns_.col_path];

            // 调用你之前写好的 GStreamer 播放逻辑
            play_file(filepath);
        }
    }


    // void play_file(const std::string& filepath) {
    //     if (!playbin_) return;
    //
    //     // 拿到 URI (gtkmm 里的 Glib 还是很好用的)
    //     std::string uri = Glib::filename_to_uri(filepath);
    //
    //     // 设置属性 (C API)
    //     g_object_set(playbin_, "uri", uri.c_str(), nullptr);
    //
    //     // 改变状态开始播放
    //     gst_element_set_state(playbin_, GST_STATE_PLAYING);
    //
    //     is_playing_ = true;
    // }
    void play_file(const std::string& filepath) {
        if (!playbin_) return;

        // 1. 【关键】先把当前的播放器停下来，清空资源
        gst_element_set_state(playbin_, GST_STATE_NULL);

        // 2. 将文件路径转为 URI
        std::string uri = Glib::filename_to_uri(filepath);

        // 3. 设置新的 URI
        g_object_set(playbin_, "uri", uri.c_str(), nullptr);

        // 4. 再次启动播放
        gst_element_set_state(playbin_, GST_STATE_PLAYING);

        is_playing_ = true;
        // 可选：更新一下状态栏
        statusbar_.push("正在播放: " + std::filesystem::path(filepath).filename().string(), 0);
    }
    void toggle_play_pause() {
        if (!playbin_) return;
        if (is_playing_) {
            gst_element_set_state(playbin_, GST_STATE_PAUSED);
            is_playing_ = false;
        } else {
            gst_element_set_state(playbin_, GST_STATE_PLAYING);
            is_playing_ = true;
        }
    }
    // 假设你获取到了拖动条的目标秒数（比如拖到了 60 秒处）
    void seek() {
        std::string seekto_str = entry_seekto_.get_text();
        int sec = 20; // 默认权重 20
        if (!seekto_str.empty())
        {
            try
            {
                sec = std::stoi(seekto_str);
            }
            catch (...)
            {
                sec = 20; // 解析失败用默认值
            }
        }
        seek_to(sec);
    }
    void seek_to(int seconds) {
        if (!playbin_) return;
        gint64 position = seconds * GST_SECOND; // GStreamer 的时间单位是纳秒
        gst_element_seek_simple(playbin_, GST_FORMAT_TIME,
            (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT), position);
    }
    void open_website()
    {
        try
        {
            Gio::AppInfo::launch_default_for_uri("https://github.com/Sh1kaCodAyo/RimeDictHelperLinux");
        }
        catch (const Glib::Error& ex)
        {
            // statusbar_.push("无法打开网页: " + ex.what(), 0);
        }
    }

private:
    // 界面组件
    Gtk::MenuItem* item_setworkspace_;
    Gtk::MenuBar menubar_;
    Gtk::Entry entry_volume_;
    // Gtk::Scale volume_scale_; // 声明为类的成员变量
    Gtk::HScale volume_scale_{0.0, 1.5, 0.01}; // 构造函数参数：最小值, 最大值, 步长
    Gtk::Entry entry_seekto_;
    Gtk::Button btn_choose_file_;
    Gtk::Button btn_pause_;
    Gtk::Button btn_seek_;

    class Playlist : public Gtk::TreeModel::ColumnRecord {
    public:
        Playlist() {
            add(col_title);
            add(col_path);
        }
        Gtk::TreeModelColumn<Glib::ustring> col_title; // 显示的歌名
        Gtk::TreeModelColumn<Glib::ustring> col_path;  // 真实的文件绝对路径
    };


    Playlist columns_;
    Glib::RefPtr<Gtk::ListStore> list_store_;
    Gtk::TreeView tree_view_;
    Glib::RefPtr<Gtk::TreeModelSort> sort_model_; // 新增排序模型（可选，但推荐存下来方便以后操作）

    Gtk::Statusbar statusbar_;

    // 播放器相关
    GstElement* playbin_ = nullptr;
    bool is_playing_ = false;

};

int main(int argc, char* argv[])
{
    auto app = Gtk::Application::create(argc, argv, "org.example.rimedicthelper");

    Glib::set_prgname("ftwrmusicplayer");
    gdk_set_program_class("ftwrmusicplayer");

    MyWindow window;
    return app->run(window);
}

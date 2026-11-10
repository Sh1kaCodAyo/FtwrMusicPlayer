#include <gtkmm.h>
#include <iostream>
#include <algorithm>
#include <cctype>
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
        set_keep_above(true);

        // ====== 0. 主布局 ======
        auto* main_box_v = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 10));

        // ====== 1. 菜单栏 ======
        // 文件菜单
        auto* item_file = Gtk::manage(new Gtk::MenuItem("文件"));
        auto* menu_file = Gtk::manage(new Gtk::Menu());
        item_file->set_submenu(*menu_file);

        auto* item_setworkspace = Gtk::manage(new Gtk::MenuItem("指定工作目录.."));
        auto* item_reloaddict = Gtk::manage(new Gtk::MenuItem("重新加载词库"));
        auto* item_quit = Gtk::manage(new Gtk::MenuItem("退出"));
        menu_file->append(*item_setworkspace);
        menu_file->append(*item_reloaddict);
        menu_file->append(*item_quit);

        // 帮助菜单
        auto* item_help = Gtk::manage(new Gtk::MenuItem("帮助"));
        auto* menu_help = Gtk::manage(new Gtk::Menu());
        item_help->set_submenu(*menu_help);

        auto* item_about = Gtk::manage(new Gtk::MenuItem("关于"));
        auto* item_website = Gtk::manage(new Gtk::MenuItem("主页"));
        menu_help->append(*item_about);
        menu_help->append(*item_website);

        menubar_.append(*item_file);
        menubar_.append(*item_help);
        main_box_v->pack_start(menubar_, Gtk::PACK_SHRINK);


        auto* edit_box_h = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 6));

        auto* label_volume = Gtk::manage(new Gtk::Label("音量"));
        auto* label_seek = Gtk::manage(new Gtk::Label("跳转"));
        edit_box_h->pack_start(*label_volume, Gtk::PACK_EXPAND_WIDGET);
        // edit_box_h->pack_start(entry_volume_, Gtk::PACK_EXPAND_WIDGET);


        // 在构造函数里配置滑块
        // volume_scale_.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
        // volume_scale_.set_range(0.0, 1.5); // 范围从 0（静音）到 1.5（150%）
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
    void bind_events()
    {
        btn_choose_file_.signal_clicked().connect(sigc::mem_fun(*this, &MyWindow::on_btn_choose_file));
        btn_pause_.signal_clicked().connect(sigc::mem_fun(*this, &MyWindow::toggle_play_pause));
        btn_seek_.signal_clicked().connect(sigc::mem_fun(*this, &MyWindow::seek));
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
    void play_file(const std::string& filepath) {
        if (!playbin_) return;

        // 拿到 URI (gtkmm 里的 Glib 还是很好用的)
        std::string uri = Glib::filename_to_uri(filepath);

        // 设置属性 (C API)
        g_object_set(playbin_, "uri", uri.c_str(), nullptr);

        // 改变状态开始播放
        gst_element_set_state(playbin_, GST_STATE_PLAYING);

        is_playing_ = true;
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
    Gtk::MenuBar menubar_;
    Gtk::Entry entry_volume_;
    // Gtk::Scale volume_scale_; // 声明为类的成员变量
    Gtk::HScale volume_scale_{0.0, 1.5, 0.01}; // 构造函数参数：最小值, 最大值, 步长
    Gtk::Entry entry_seekto_;
    Gtk::Button btn_choose_file_;
    Gtk::Button btn_pause_;
    Gtk::Button btn_seek_;
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

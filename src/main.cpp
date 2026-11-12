//
// Created by ftwrjh on 2026/11/12.
//

#include "MyWindow.h"

int main(int argc, char *argv[]) {
    auto app = Gtk::Application::create(argc, argv, "org.example.ftwrmusicplayer");

    Glib::set_prgname("ftwrmusicplayer");
    gdk_set_program_class("ftwrmusicplayer");

    MyWindow window;
    return app->run(window);
}

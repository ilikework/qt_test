// MMCameraCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>

// VideoServer.cpp
// 32bit VC++ 相机 Server Demo
// - 监听 127.0.0.1:52345
// - 支持 JSON 命令 + 二进制帧数据
// - startPreview 后周期推送 dummy 帧
// - getPreviewFrame / capture 时立即返回一帧


#include "SocketServer.h"


// ---------- main ----------

int main()
{
    SocketServer::Config cfg;
    cfg.bindIp = "127.0.0.1";
    cfg.port = 52345;

    SocketServer server(cfg);
    if (!server.start()) return 1;
    server.run();
    return 0;
}


// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件

# computer_networs_lab1
A multi-user Chinese-English chat program implemented using sockets
计网第一次实验作业
1.聊天协议的完整说明在实验报告中体现
2.支持中英文多人聊天
3.服务器端：运行chat_server.cpp文件启动服务器端程序
    服务器收到并且转发消息，开始运行服务端程序时在固定端口监听客户端程序，等待连接，
    利用多线程实现，每次客户端连接之后服务器端为客户端创建一个线程来相应客户端的程序
4.客户端：运行chat_client.cpp文件启动客户端程序（需要首先启动服务器端程序）
  输入history查看历史纪录
  输入exit退出
  每次发送消息之后屏幕上会显示 you：发送的消息
  每次收到消息之后屏幕上会显示 用户编号：发送的消息
5.数据丢失的检查和判断
  利用wireshark实现抓包观察是否存在数据丢失和恢复的情况，这部分内容在实验报告中进行了详细的说明

// Linux 平台的任务处理代码

#if defined(_LINUX) || defined(__linux__)

#include <iostream>
#include <fstream>

#include "include/paddleocr.h"
#include "include/args.h"
#include "include/task.h"

// 套接字
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#undef INVALID_SOCKET
#define INVALID_SOCKET -1


namespace PaddleOCR
{
    // 代替cv imread，接收utf-8字符串传入，返回Mat。
    cv::Mat Task::imread_u8(std::string pathU8, int flag)
    {
        std::ifstream fileInput;
        size_t fileLength;
        char *buffer;
        
        // 尝试将pathU8读取到内存
        try
        {
            fileInput.open(pathU8, std::ios::binary);
            // 路径不存在且无法输出
            if (!fileInput)
            {
                set_state(CODE_ERR_PATH_EXIST, MSG_ERR_PATH_EXIST(pathU8));
                return cv::Mat();
            }
            
            // 获取文件大小并将文件读到内存
            fileInput.seekg (0, fileInput.end);
            fileLength = fileInput.tellg();
            fileInput.seekg (0, fileInput.beg);
            buffer = new char[fileLength];
            fileInput.read(buffer, fileLength);
            
            // 无法读取
            if (!fileInput)
            {
                set_state(CODE_ERR_PATH_READ, MSG_ERR_PATH_READ(pathU8));
                return cv::Mat();
            }
        }
        catch (std::exception& err)
        {
            // 文件打开失败或者是读取失败，这里默认是读取失败
            set_state(CODE_ERR_PATH_READ, MSG_ERR_PATH_READ(pathU8));
            return cv::Mat();
        }
        
        // 解码内存数据，变成cv::Mat数据
        cv::_InputArray array(buffer, fileLength);
        cv::Mat image = cv::imdecode(array, flag);
        // 释放buffer空间必须放在 cv::imdecode() 之后，
        // 因为 cv::_InputArray() 并不会复制buffer内的元素，
        // 而 cv::imdecode() 在解码时会新开辟一块内存并复制解码后的数据
        delete[] buffer;
        fileInput.close();
        
        // 解码失败
        if (image.empty())
        {
            set_state(CODE_ERR_PATH_DECODE, MSG_ERR_PATH_DECODE(pathU8));
            return cv::Mat();
        }
        
        return image;
    }
    
    int Task::socket_mode()
    {
        // 创建套接字，协议族为TCP/IP
        int socketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (socketFd == INVALID_SOCKET)
        {
            std::cerr << "Failed to create socket." << std::endl;
            close(socketFd);
            return -1;
        }
        
        // 配置地址和端口号
        struct sockaddr_in socketAddr;
        // 地址族：IPv4
        socketAddr.sin_family = AF_INET;
        // IP地址模式：本地环回/任何可用/其他IPv4
        if (addr_to_uint32(FLAGS_addr, socketAddr.sin_addr.s_addr) < 0)
        {
            std::cerr << "Failed to parse input address." << std::endl;
            close(socketFd);
            return -1;
        }
        // 端口号
        socketAddr.sin_port = htons(FLAGS_port);
        
        // 绑定地址和端口号到套接字句柄socketFd
        if (bind(socketFd, (struct sockaddr*)&socketAddr, sizeof(socketAddr)) == INVALID_SOCKET)
        {
            std::cerr << "Failed to bind address." << std::endl;
            close(socketFd);
            return -1;
        }
        
        // 将套接字socketFd设为监听状态，只允许1个客户端排队连接
        if (listen(socketFd, 1) == INVALID_SOCKET)
        {
            std::cerr << "Failed to set listen." << std::endl;
            close(socketFd);
            return -1;
        }
        
        // 获取服务端实际ip和端口
        struct sockaddr_in serverAddr;
        socklen_t len = sizeof(serverAddr);
        if (getsockname(socketFd, (sockaddr*)&serverAddr, &len) == INVALID_SOCKET)
        {
            std::cerr << "Failed to get sockname." << std::endl;
            close(socketFd);
            return -1;
        }
        // 获取端口号 & ip地址
        int serverPort = ntohs(serverAddr.sin_port);
        char* serverIp = inet_ntoa(socketAddr.sin_addr);
        std::cout << "Socket init completed. " << serverIp << ":" << serverPort << std::endl;
        
        // 启动内存清理线程
        cleanup_thread_start();
        
        // 循环等待接收连接
        while (true)
        {
            // 接受连接请求
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);
            int clientFd = accept(socketFd, (sockaddr*)&clientAddr, &clientAddrLen);
            if (clientFd == INVALID_SOCKET)
            {
                std::cerr << "Failed to accept connection." << std::endl;
                continue;
            }
            
            // 获取客户端实际ip和端口
            char* clientIp = inet_ntoa(clientAddr.sin_addr);
            int clientPort = ntohs(clientAddr.sin_port);
            std::cerr << "Client connected. IP address: " << clientIp << ":" << clientPort << std::endl;
            
            // 接收任意长度数据
            std::string strIn; // 接收数据存放处
            char buffer[1024]; // 缓冲区
            int bytesRecv = 0;
            while (true)
            {
                bytesRecv = recv(clientFd, buffer, sizeof(buffer), 0);
                
                // 没收到数据，可能客户端断开连接或连接错误
                if (bytesRecv <= 0)
                    break;
                
                // 将本次接收到的数据追加到存放处末尾
                strIn.append(buffer, bytesRecv);
                
                // 到达末尾符，认为数据已全部接收完毕
                if (buffer[bytesRecv - 1] == '\0' || buffer[bytesRecv - 1] == '\n')
                    break;
            }
            
            // 没有接收到数据 | 接收到0字节
            if (bytesRecv <= 0)
            {
                std::cerr << "Failed to receive data." << std::endl;
                close(clientFd);
                continue;
            }
            std::cerr << "Get string. Length: " << strIn.length() << std::endl;
            
            
            // =============== OCR开始 ===============
            set_state(); // 初始化状态
            // 获取ocr结果
            std::string strOut = run_ocr(strIn);
            // 接收到退出指令，关闭连接，退出主循环，結束服务器
            if (is_exit)
            {
                close(clientFd);
                break;
            }
            // =============== OCR完毕 ===============
            
            // 发送数据
            std::cerr << strOut << std::endl;
            int bytesSent = send(clientFd, strOut.c_str(), strlen(strOut.c_str()), 0);
            // 没有发送出数据 | 发送出0字节
            if (bytesSent <= 0)
                std::cerr << "Failed to send data." << std::endl;
            
            // 关闭连接
            close(clientFd);
        }
        
        // 关闭套接字
        close(socketFd);
        
        // 结束内存清理线程
        cleanup_thread_join();
        
        return 0;
    }
}

#endif
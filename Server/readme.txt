数据库配置文件
    G2652.sql - 初始化数据库
    clear.sql - 清空文件表但不清空用户表    

构建
    cd src/build && cmake .. && make 

执行
    ./src/build/tmy_server <port>

环境要求
    cmake 2.6+
    GCC 4.9.2+ 或 clang 3.4+
        
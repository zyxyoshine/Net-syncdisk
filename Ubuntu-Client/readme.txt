系统环境
    Ubuntu 16.04 LTS

构建
    Qt Creator 打开工程文件 src/TMYClient.pro 即可
    
配置文件
    两配置文件放置在可执行文件同目录下
    tmyignore.txt - 正则表达式过滤不需要同步的文件
    tmyuser.txt - 4行配置文件。依次为 服务器IP、端口、session和绑定路径
                  第一次使用需要在手动指定 服务器IP、端口。session和绑定路径留空即可
    （可参照build/下例子）                   
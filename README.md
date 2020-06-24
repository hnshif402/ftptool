# ftptool
transfer local file to remote server by ftp .
此工具将本地的文件通过ftp传送到远程服务器上
实现以下几个目标：
1. 在处理本地文件时，支持正则表达式，根据文件名，将数据传送到不同的ftp服务器上;
2. 文件上传成功后，将本地文件删除
3. 文件上传失败，不对本地文件进行处理

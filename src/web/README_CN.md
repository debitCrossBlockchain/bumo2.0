# WEB

## 基本介绍
提供基本的 web 页面访问功能。目前提供的功能是智能合约语法检测工具。

## 模块结构

名称 | 目录 | 功能
|:--- | --- | ---
| `jslint` | [jslint](./jslint) | 智能合约检测工具。参考文档 [智能合约语法说明](./jslint/ContractRules_CN.md)。

## 配置方法
- 打开 [bumo.json](../../build/win32/config/bumo.json) 配置如下：
    ```
    "webserver": {
        "listen_addresses": "0.0.0.0:36002",
        "index_name": "",           #主页路径
        "directory": "web/",        #web 目录
    }
    ```
- 拷贝该目录（web） 下的文件夹和文件至部署目录 `./bumo/web/` 下
- 重启 `bumo` 程序
- 访问 `http://ip:36002/jslint/index.html`

# dde-widgets-weather-plugin
 [![英文](https://img.shields.io/badge/语言-英文-blue.svg)](https://github.com/zty199/dde-widgets-weather-plugin/blob/main/README.md)
 [![中文](https://img.shields.io/badge/语言-中文-red.svg)](https://github.com/zty199/dde-widgets-weather-plugin/blob/main/README.zh-Hans.md)

 一个简单的 [dde-widgets](https://github.com/linuxdeepin/dde-widgets) 天气插件，基于 [和风天气开发服务](https://dev.qweather.com/) 开发。

# 依赖
 查看 `debian/control` 文件以获取构建及运行时依赖。

 使用以下命令安装所需依赖：
 ```bash
 $ sudo apt build-dep .
 ```

# 安装
 确保你已经安装全部依赖。

 使用以下命令构建二进制软件包：
 ```bash
 $ dpkg-buildpackage -Zxz -rfakeroot -uc -us -nc -b -j$(nproc)
 ```

 > 二进制软件包将生成在上级目录。

 * 在构建二进制软件包时预置 [KEY](https://dev.qweather.com/docs/configuration/project-and-key/)，仅需在命令前设置环境变量即可：

   ```bash
   $ env DEFAULT_WEB_API_KEY=xxxxxxxx dpkg-buildpackage -Zxz -rfakeroot -uc -us -nc -b -j$(nproc)
   ```

# 参考
 * [linuxdeepin/dde-widgets](https://github.com/linuxdeepin/dde-widgets)
 * [ShawZG/xWeather](https://github.com/ShawZG/xWeather)
 * [ipbase Documentation](https://ipbase.com/docs)
 * [文档 | 和风天气开发服务](https://dev.qweather.com/docs)

# 开源许可证
 dde-widgets-weather-plugin 遵循 [GPL-3.0-or-later](https://github.com/zty199/dde-widgets-weather-plugin/blob/main/LICENSE) 协议发布。

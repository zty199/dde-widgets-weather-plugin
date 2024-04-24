# dde-widgets-weather-plugin
 [![en-US](https://img.shields.io/badge/lang-en--US-blue.svg)](https://github.com/zty199/dde-widgets-weather-plugin/blob/main/README.md)
 [![zh-Hans](https://img.shields.io/badge/lang-zh--Hans-red.svg)](https://github.com/zty199/dde-widgets-weather-plugin/blob/main/README.zh-Hans.md)

 A simple weather plugin of [dde-widgets](https://github.com/linuxdeepin/dde-widgets), based on [QWeather Web API](https://dev.qweather.com/).

# Dependencies
 Check `debian/control` for build-time and runtime dependencies.

 Use the following command to install required dependencies:
 ```bash
 $ sudo apt build-dep .
 ```

# Installation
 Make sure you have installed all dependencies.

 Use the following command to build binary package:
 ```bash
 $ dpkg-buildpackage -Zxz -rfakeroot -uc -us -nc -b -j$(nproc)
 ```

 > The binary package will be generated in parent directory.

 * To build binary package with built-in [KEY](https://dev.qweather.com/docs/configuration/project-and-key/), just set environment variable before the command:

   ```bash
   $ env DEFAULT_WEB_API_KEY=xxxxxxxx dpkg-buildpackage -Zxz -rfakeroot -uc -us -nc -b -j$(nproc)
   ```

# Reference
 * [linuxdeepin/dde-widgets](https://github.com/linuxdeepin/dde-widgets)
 * [ShawZG/xWeather](https://github.com/ShawZG/xWeather)
 * [ipbase Documentation](https://ipbase.com/docs)
 * [文档 | 和风天气开发服务](https://dev.qweather.com/docs)

# License
 dde-widgets-weather-plugin is licensed under [GPL-3.0-or-later](https://github.com/zty199/dde-widgets-weather-plugin/blob/main/LICENSE).

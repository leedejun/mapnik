/* This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

// qt
#include <QApplication>
#include <QStringList>
#include <QSettings>
#include <QString>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include "mainwindow.hpp"
#include "roadmerger.h"
#include "ThreadPool.h"
#include <QTimer>
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include <iostream>
#include <fstream>
#include <windows.h>


namespace fd{

struct MergemapParam
{
  QString basemap;
  QMap<QString, QString> cehuiDataSource2path;
  QString featureid2osmidPath;
  QString midlinePath;
  QString appBinDir;
  QString groupidsPath;
  QString completeRoadsPath;
};


QMap<QString, QString> loadCehuiDataSource2path(const QString& cehuijsonpath)
{
    QMap<QString, QString> resultMap;
    QFile file(cehuijsonpath); 
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // 读取文件内容到一个字符串中
         QTextStream in(&file);
        // 设置 QTextStream 的编码为 UTF-8
        in.setCodec("UTF-8");
        QString content = in.readAll();
        file.close();

        // 现在 content 字符串包含 UTF-8 编码的文件内容
        // 你可以将其转换为 std::string，如果需要的话
        std::string contentUtf8 = content.toStdString();

        // 使用rapidjson的Document类解析字符串
        rapidjson::Document doc;
        doc.Parse(contentUtf8.c_str());

        // 检查是否是有效的json文档
        if (doc.HasParseError()) {
            // 处理错误情况
            fprintf(stderr, "\nHasParseError: %s(pos: %u)\n",
            rapidjson::GetParseError_En(doc.GetParseError()),
            (unsigned)(doc.GetErrorOffset()));

            return resultMap;
        }

        // 确保它是一个数组
        if (doc.IsArray()) {
            // 遍历数组
            for (rapidjson::SizeType i = 0; i < doc.Size(); i++) {
                // 获取数组中的对象
                const rapidjson::Value& obj = doc[i];

                // 确保对象包含"sourcename"和"path"
                if (obj.HasMember("sourcename") && obj.HasMember("path")) {
                    QString sourceName = QString::fromStdString(obj["sourcename"].GetString());
                    QString path = QString::fromStdString(obj["path"].GetString());
                    // 将解析的数据添加到QMap中
                    resultMap.insert(sourceName, path);
                }
            }
        }
        return resultMap;
    }
    return resultMap;
}

void loadMergemapIniFile(const QString& mergemapIniFile, MergemapParam& mergParam)
{
  QSettings settings(mergemapIniFile, QSettings::IniFormat);
  // 读取数据表字段名配置
  settings.beginGroup("mergemap");
  mergParam.basemap = settings.value("basemap", "basemap.shp").toString();
  QString cehuijsonpath = settings.value("cehuijsonpath", "cehui.json").toString();
  mergParam.cehuiDataSource2path = loadCehuiDataSource2path(cehuijsonpath);
  mergParam.featureid2osmidPath = settings.value("featureid2osmidPath", "featureid2osmid.json").toString();
  mergParam.midlinePath = settings.value("midlinePath", "midline.json").toString();
  mergParam.appBinDir = settings.value("appBinDir", "./").toString();
  mergParam.groupidsPath = settings.value("groupidsPath", "groupids.json").toString();
  mergParam.completeRoadsPath = settings.value("completeRoadsPath", "completeRoads.json").toString();
  settings.endGroup();
}


}

int main(int argc, char** argv)
{
    BOOL success = SetDllDirectory(TEXT("./"));
    if (!success) {
	    // 设置路径失败，进行错误处理
        std::cerr << "设置路径失败，进行错误处理!"  << "'\n";
        return -1;
    }
    using mapnik::datasource_cache;
    using mapnik::freetype_engine;
    try
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
        if(argc<2)
        {
         std::cerr << "Program is missing parameters!"  << "'\n";
         return 1;
        }
        else
        {
         std::cout<<"arcv[0]:"<<argv[0]<<std::endl;
         std::cout<<"arcv[1]:"<<argv[1]<<std::endl;
        }

        QString mergemapIniFilePath = argv[1];
        fd::MergemapParam mergParam;
        fd::loadMergemapIniFile(mergemapIniFilePath, mergParam);


        QString appPath = mergParam.appBinDir;
        QString iniFilePath = appPath + "/viewer.ini";
        QCoreApplication::setApplicationName("Viewer");
        QSettings settings(iniFilePath, QSettings::IniFormat);

        // register input plug-ins
        QString plugins_dir = settings.value("mapnik/plugins_dir", QVariant("/usr/local/lib/mapnik/input/")).toString();
        datasource_cache::instance().register_datasources(plugins_dir.toStdString());
        // register fonts
        int count = settings.beginReadArray("mapnik/fonts");
        for (int index = 0; index < count; ++index)
        {
            settings.setArrayIndex(index);
            QString font_dir = settings.value("dir").toString();
            freetype_engine::register_fonts(font_dir.toStdString());
        }
        settings.endArray();

        QApplication app(argc, argv);
        MainWindow window;
        // connect(&window, SIGNAL(quit_signal()), &app, SLOT(QCoreApplication::quit()), Qt::QueuedConnection);
        QString featureid2osmidPath = mergParam.featureid2osmidPath;
        if(!window.loadFeatureid2osmid(featureid2osmidPath))
        {
          std::cerr << "Loading Featureid2osmid json failed!"  << "'\n";
          return 1;
        }

        //读取测绘数据字段的配置
        QString cehuiTableIniFilePath = iniFilePath;
        window.loadCehuiTableFields(cehuiTableIniFilePath);

        //更新groupid下拉框
        QString groupidsPath = mergParam.groupidsPath;
        if(!window.updateGroupidComboBox(groupidsPath))
        {
          std::cout << "updateGroupidComboBox failed!"  << "'\n";
          //return 1;
        }

        window.setMidLineJsonPath(mergParam.midlinePath);
        window.setCompleteRoadsFile(mergParam.completeRoadsPath);
        window.show();
        //std::cout<<"mergParam.basemap:"<<mergParam.basemap.toStdString()<<"mergParam.cehuipath:"<<mergParam.cehuipath.toStdString()<<std::endl;
        // window.mapWidget()->roadMerger->merge(mergParam.basemap,mergParam.cehuipath);
        // Quit application when work is finished
        QObject::connect(&window, SIGNAL(completeRoads_quit_signal()), &app, SLOT(quit()));
        window.merge(mergParam.basemap, mergParam.cehuiDataSource2path);
        return app.exec();
    } catch (std::exception const& ex)
    {
        std::cerr << "Could not start viewer: '" << ex.what() << "'\n";
        return 1;
    }
}

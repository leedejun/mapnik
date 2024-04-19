#ifndef ROADMERGER_H
#define ROADMERGER_H


#include "mapwidget.hpp"
#include <mapnik/memory_datasource.hpp>
#include <mapnik/geometry/boost_adapters.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry/multi_polygon.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/register/multi_linestring.hpp>
#include <boost/geometry/multi/geometries/register/multi_polygon.hpp>
#include <QThread>
#include <mapnik/color.hpp>
#include "cehuidatainfo.hpp"

class RoadMerger : public QThread
{
    Q_OBJECT
    // 基础路网剪裁后的数据源
    std::shared_ptr<mapnik::memory_datasource> clipedBaseSource;

    // 测绘数据缓冲区数据源
    std::shared_ptr<mapnik::memory_datasource> cehuiBufferSource;

    // 融合后的数据源
    std::shared_ptr<mapnik::memory_datasource> mergedSource;

    std::shared_ptr<mapnik::memory_datasource> clipedCehuiSource;

    std::shared_ptr<mapnik::memory_datasource> selectedResultBufferSource;

    QString baseShp;
    QString cehuiShp;
    MapWidget* mapWidget;

    int m_mergedSourceIndex;
    int m_clipedCehuiSourceIndex;
public:
    RoadMerger(MapWidget* mapWidget);

    // 启动融合过程
    void merge(QString const& base,QString const& cehui);

    // 显示路网图层
    void showRoadLayers();

    // 显示路网图层
    void toggleMergedRoad(double x, double y);

    void toggleNeedCompleteRoad(double x, double y);

    // 获取融合结果
    void getMergeResult(std::vector<long>& result);

    void generateResultBuffer(std::vector<mapnik::geometry::multi_polygon<double>>& out);

    void clipedLineEx(mapnik::geometry::geometry<double>& in,std::vector<mapnik::geometry::multi_polygon<double>>& buffers,mapnik::geometry::multi_line_string<double>& out);

    void clipedCehuiData();

    void clearLayers();

    void showClipedCehuiOnMap();

    bool exportCompleteRoads(const QString& completeRoadsFile);

    void getCompleteRoadsResult(std::vector<cehuidataInfo>& result);

    std::string convertToWKT(const mapnik::geometry::line_string<double>& lineString);

    std::string convertToWKT(const mapnik::geometry::multi_line_string<double>& multiLineString);

    bool SerializeCompleteRoadInfos(const std::vector<cehuidataInfo>& result, const std::string& groupId, const QString& completeRoadsFile);

protected:
    void run();

signals:
    void signalMergeStart();
    void signalMergeEnd();

private:
    void addLineLayer(QString const& name,QString const& baseShp,std::string color, double lineWidth = 1.0);
    void addLineLayer(QString const& name,std::shared_ptr<mapnik::datasource> ds,std::string color, double lineWidth = 1.0);
    void addPolygonLayer(QString const& name,std::shared_ptr<mapnik::datasource> ds,mapnik::color stroke,mapnik::color fill);
    void addMergedLayer();
    void addClipedCehuiLayer();

    void addSelectedResultBufferLayer();
};

#endif // ROADMERGER_H

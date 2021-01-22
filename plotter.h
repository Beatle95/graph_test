#ifndef PLOTTER_H
#define PLOTTER_H

#include <QWidget>
#include <QMutex>
#include <QMutexLocker>
#include <QList>
#include <QLabel>
#include <QPen>
#include <QPainter>
#include <QPoint>

#include <stdexcept>

#include <QDebug>

class Plotter : public QWidget
{
public:
    QMutex mutex;
    QLabel* xAxisLabel;
    QLabel* yAxisLabel;
    QLabel* mainLabel;
    QLabel** xAxisPoints;
    QLabel** yAxisPoints;

    Plotter(int x, int y, int w, int h, QWidget* parent = nullptr);
    ~Plotter();
    //Adds new point to plot inner buffer, that can be drawn later
    void appendPlotPoint(float _x, float _y);
    //Clears inner buffer of new points
    void clearPlotPoints();
    //Clears plot area (not necessary to call before drawView())
    void clearView();
    //This method process inner buffer of new points and transforms them to points on plot view, and markes this widget as nedded to update
    void drawView();

private:
    QVector<float> x;
    QVector<float> y;
    QPoint* drawPoints = nullptr;

    int size_x;
    int size_y;
    int X_OFFSET;
    int Y_OFFSET;
    int X_RIGHT_OFFSET;
    int Y_TOP_OFFSET;
    int xAxisDelta;
    int yAxisDelta;
    int drawCount;

    void placeLabels();
    //transformes point (x;y) from input data coordinate system to coordinate on current widget
    inline QPoint transformCoordinate(float x, float y, float minX, float maxX, float minY, float maxY){
        float x_ret = (x - minX) / (maxX - minX);
        float y_ret = (y - minY) / (maxY - minY);
        return QPoint(X_OFFSET + int(float(size_x - X_OFFSET - X_RIGHT_OFFSET)*x_ret), size_y - Y_OFFSET - int(float(size_y - Y_OFFSET - Y_TOP_OFFSET)*y_ret));
    }

protected:
    //overrided painevent to draw plots
    void paintEvent(QPaintEvent*) override;

};

#endif // PLOTTER_H

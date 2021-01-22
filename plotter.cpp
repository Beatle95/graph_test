#include "plotter.h"

Plotter::Plotter(int x, int y, int w, int h, QWidget* parent) : QWidget(parent)
{
    //preparing all data
    setGeometry(x, y, w, h);
    size_x = w;
    size_y = h;
    if(size_x < 200 || size_y < 200)
        throw std::runtime_error("Plot area has to be at least 200x200 pixels.");
    X_OFFSET = 80;
    Y_OFFSET = 60;
    X_RIGHT_OFFSET = 30;
    Y_TOP_OFFSET = 30;
    drawPoints = new QPoint[(size_x - X_OFFSET - X_RIGHT_OFFSET)*2];
    xAxisLabel = new QLabel(this);
    yAxisLabel = new QLabel(this);
    mainLabel = new QLabel(this);
    xAxisLabel->setText("<b>timestamps</b>");
    yAxisLabel->setText("<b>measurments</b>");
    mainLabel->setText("<h3>Measurement data</h3>");
    xAxisLabel->move(size_x/2 - xAxisLabel->width()/2, size_y - xAxisLabel->height());
    yAxisLabel->move(0, 10);
    mainLabel->move(size_x/2 - mainLabel->width()/2, 0);

    xAxisPoints = new QLabel*[10];
    yAxisPoints = new QLabel*[10];

    for(int i = 0; i < 10; i++){
        xAxisPoints[i] = new QLabel(this);
        yAxisPoints[i] = new QLabel(this);
    }
    xAxisDelta = (size_x - X_OFFSET - X_RIGHT_OFFSET) / 10;     //calculate deltas for grid layout display
    yAxisDelta = (size_y - Y_OFFSET - Y_TOP_OFFSET) / 10;
    clearView();
}

Plotter::~Plotter()
{
    //need to delete only this, cause all other objects will be deleted from parent this destructor through QObject parent system invalidation
    delete[] drawPoints;
}

void Plotter::appendPlotPoint(float _x, float _y)
{
    //adding points to vector (vector not so efficient in that case, but it is way more efficient, then using QList rundom access in calculations)
    x.append(_x);
    y.append(_y);
}

void Plotter::clearPlotPoints()
{
    x.clear();
    y.clear();
}

void Plotter::clearView()
{
    //write all points to 0, and all labels to 0.00
    drawCount = size_x - X_OFFSET - X_RIGHT_OFFSET;
    for(int i = 0; i < (size_x - X_OFFSET - X_RIGHT_OFFSET)*2; i+=2){
        if(i == 0){
            drawPoints[i].setX(i/2 + X_OFFSET);
            drawPoints[i].setY(size_y - Y_OFFSET - 10);
            drawPoints[i+1] = drawPoints[i];
            continue;
        }
        drawPoints[i] = drawPoints[i-1];
        drawPoints[i+1].setX(X_OFFSET + i/2);
        drawPoints[i+1].setY(size_y - Y_OFFSET);
    }
    for(int i = 0; i < 10; i++){
        xAxisPoints[i]->setText("0.00");
        yAxisPoints[i]->setText("0.00");
    }
    placeLabels();
    update();
}

void Plotter::drawView()
{
    //first check inner buffers
    if(x.size() != y.size())
        throw std::runtime_error("Number of points in x and y lists are not the same!");
    if(x.isEmpty())
        return;
    QMutexLocker locker(&mutex);    //Lock mutex to prevent for multiaccessing (not necessary in this app, but could be useful in more complex)
    //first we need to find min/max elements
    float minX = x[0], maxX = x[0], minY = y[0], maxY = y[0];
    for(int i = 1; i < x.size(); i++){
        if(x[i] < minX)
            minX = x[i];
        else if(x[i] > maxX)
            maxX = x[i];
        if(y[i] < minY)
            minY = y[i];
        else if(y[i] > maxY)
            maxY = y[i];
    }
    int pointsCount = size_x - X_OFFSET - X_RIGHT_OFFSET;
    if(x.size() > pointsCount){
        float delta = float(x.size()) / float(pointsCount);
        float next_pointer = 0.0f;
        int pointer = 0;
        drawCount = size_x - X_OFFSET - X_RIGHT_OFFSET;
        for(int i = 0; i < drawCount; i++){     //iterate through all points that will be drawn
            //here we can't draw all points, so now we have to choose the most significant data
            next_pointer = next_pointer + delta;
            if(i == drawCount-1)
                next_pointer = x.size() - 0.95f;
            //now we will find min/max elements and then, to choose beetwen them, we calculate min/max defference from average
            float min = y[pointer], max = y[pointer], sum = 0;
            for(; pointer < int(next_pointer); pointer++){
                if(y[pointer] < min)
                    min = y[pointer];
                else if(y[pointer] > max)
                    max = y[pointer];
                sum += y[pointer];
            }
            float avg = sum / delta;
            float cur_y;
            if(abs(max - avg) > abs(min - avg))
                cur_y = max;
            else
                cur_y = min;
            if(i == 0){
                drawPoints[i] = transformCoordinate(x[pointer - delta/2], cur_y, minX, maxX, minY, maxY);
                drawPoints[i+1] = drawPoints[i];
                continue;
            }
            drawPoints[i*2] = drawPoints[i*2-1];    //as we are using drawLines method from QPainter class, we need to duplicate previous point everytime
            drawPoints[i*2+1] = transformCoordinate(x[pointer - delta/2], cur_y, minX, maxX, minY, maxY);
        }
    }
    else{
        drawCount = x.size();
        for(int i = 0; i < x.size(); i++){
            if(i == 0){
                drawPoints[i] = transformCoordinate(x[i], y[i], minX, maxX, minY, maxY);
                drawPoints[i+1] = drawPoints[i];
                continue;
            }
            drawPoints[i*2] = drawPoints[i*2-1];    //as we are using drawLines method from QPainter class, we need to duplicate previous point everytime
            drawPoints[i*2+1] = transformCoordinate(x[i], y[i], minX, maxX, minY, maxY);
        }
    }
    //now we need to update grid labels
    float deltaX = (float(maxX) - float(minX)) / 10;
    float deltaY = (float(maxY) - float(minY)) / 10;
    for(int i = 0; i < 10; i++){
        xAxisPoints[i]->setText(QString::number(minX+(deltaX*(i+1)), 'f', 2));
        yAxisPoints[i]->setText(QString::number(minY+(deltaY*(i+1)), 'f', 2));
    }
    placeLabels();
    update();
}

void Plotter::placeLabels()
{
    //calculate additional variables and relocate grid labels, according to their current size
    int xAxisY = size_y - Y_OFFSET + 8;
    int xAxisXStart = X_OFFSET;
    int yAxisYStart = size_y - Y_OFFSET - yAxisDelta;

    for(int i = 0; i < 10; i++){
        xAxisPoints[i]->setFixedWidth(7*xAxisPoints[i]->text().size());
        yAxisPoints[i]->setFixedWidth(7*yAxisPoints[i]->text().size());
        xAxisPoints[i]->setFixedHeight(11);
        yAxisPoints[i]->setFixedHeight(11);

        xAxisPoints[i]->move(xAxisXStart + xAxisDelta*(i+1) - xAxisPoints[i]->width()/2, xAxisY);
        int yAxisX = X_OFFSET/2 - yAxisPoints[i]->width()/2;
        yAxisPoints[i]->move(yAxisX, yAxisYStart - yAxisDelta*i);
    }
}

void Plotter::paintEvent(QPaintEvent*)
{
    //draw grid
    QPainter painter(this);
    QPen pen2(QColor(180, 180, 180));
    pen2.setStyle(Qt::DashLine);
    painter.setPen(pen2);
    for(int i = 1; i < 11; i++){
        painter.drawLine(X_OFFSET + xAxisDelta*i, size_y - Y_OFFSET, X_OFFSET + xAxisDelta*i, Y_TOP_OFFSET);
        painter.drawLine(X_OFFSET, size_y - Y_OFFSET - yAxisDelta*i, size_x, size_y - Y_OFFSET - yAxisDelta*i);
    }
    //draw plot lines
    QPen graphPen(QColor(0, 0, 205));
    painter.setPen(graphPen);
    painter.drawLines(drawPoints, drawCount);
    //draw plot points
    QPen dotPen(QColor(50, 50, 50));
    dotPen.setWidth(3);
    painter.setPen(dotPen);
    for(int i = 1; i < drawCount*2; i+=2)
        painter.drawPoint(drawPoints[i]);
    //draw grid arrows
    QPen pen(QColor(0, 0, 0));
    pen.setWidth(2);
    painter.setPen(pen);
    QPoint p[] = {
        QPoint(X_OFFSET, Y_TOP_OFFSET),
        QPoint(X_OFFSET, size_y - Y_OFFSET),
        QPoint(X_OFFSET, size_y - Y_OFFSET),
        QPoint(size_x, size_y - Y_OFFSET),
        QPoint(X_OFFSET - 5, Y_TOP_OFFSET + 10),
        QPoint(X_OFFSET, Y_TOP_OFFSET),
        QPoint(X_OFFSET, Y_TOP_OFFSET),
        QPoint(X_OFFSET + 5, Y_TOP_OFFSET + 10),
        QPoint(size_x - 10, size_y - Y_OFFSET - 5),
        QPoint(size_x, size_y - Y_OFFSET),
        QPoint(size_x, size_y - Y_OFFSET),
        QPoint(size_x - 10, size_y - Y_OFFSET + 5)
    };
    painter.drawLines(p, 6);
}

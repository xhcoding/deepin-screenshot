#include "shapeswidget.h"

#include "utils/calculaterect.h"

#include <QApplication>
#include <QPainter>
#include <QDebug>

const int DRAG_BOUND_RADIUS = 8;
const int SPACING = 12;

ShapesWidget::ShapesWidget(QWidget *parent)
    : QFrame(parent),
      m_shapesMap(QMap<int ,QString>()),
      m_selectedIndex(-1),
      m_isMoving(false),
      m_isSelected(false),
      m_isShiftPressed(false)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setAcceptDrops(true);
    installEventFilter(this);
}

ShapesWidget::~ShapesWidget() {

}

void ShapesWidget::setCurrentShape(QString shapeType) {
    m_currentShape = shapeType;
}

ResizeDirection ShapesWidget::getResizeDirection(QPointF point1, QPointF point2,
                                    QPointF point3, QPointF point4, QPointF pos) {

    QPointF rotatePoint = getRotatePoint(point1, point2, point3, point4);
    rotatePoint = QPointF(rotatePoint.x() - 5, rotatePoint.y() - 5);

    if (pointClickIn(rotatePoint, pos)) {
        m_isResize = true;
        m_resizeDirection = Rotate;
    } else if (pointClickIn(point1, pos)) {
        m_isResize = true;
        m_resizeDirection = TopLeft;
    } else if (pointClickIn(point2, pos)) {
        m_isResize = true;
        m_resizeDirection = BottomLeft;
    } else if (pointClickIn(point3, pos)) {
        m_isResize = true;
        m_resizeDirection = TopRight;
    } else if (pointClickIn(point4, pos)) {
        m_isResize = true;
        m_resizeDirection = BottomRight;
    }/* else if (pointOnLine(point1, point2, pos)) {
        m_isResize = true;
//        m_resizeDirection = Left;
        m_resizeDirection == Moving;
    } else if (pointOnLine(point1, point3, pos)) {
        m_isResize = true;
//        m_resizeDirection = Top;
        m_resizeDirection = Moving;
    } else if (pointOnLine(point3, point4, pos)) {
        m_isResize = true;
//        m_resizeDirection = Right;
        m_resizeDirection = Moving;
    } else if (pointOnLine(point2, point4, pos)) {
        m_isResize = true;
//        m_resizeDirection = Bottom;
        m_resizeDirection = Moving;
    }*/
    else {
        m_resizeDirection = Moving;
    }

    return m_resizeDirection;
}

bool ShapesWidget::clickedOnShapes(QPointF pos) {
    bool onShapes = false;
    m_currentDiagPoints.masterPoint = QPointF(0, 0);
    m_currentDiagPoints.deputyPoint = QPointF(0, 0);
    m_selectedIndex = -1;

    for (int i = 0; i < m_mFPointsList.length(); i++) {
        if (clickedOnPoint(m_mFPointsList[i].point1, m_mFPointsList[i].point2,
                           m_mFPointsList[i].point3, m_mFPointsList[i].point4, pos)) {
            m_currentSelectedDiagPoints.masterPoint = m_mFPointsList[i].point1;
            m_currentSelectedDiagPoints.deputyPoint = m_mFPointsList[i].point4;
            m_currentSelectedFPoints.point1 = m_mFPointsList[i].point1;
            m_currentSelectedFPoints.point2 = m_mFPointsList[i].point2;
            m_currentSelectedFPoints.point3 = m_mFPointsList[i].point3;
            m_currentSelectedFPoints.point4 = m_mFPointsList[i].point4;
            m_selectedIndex = i;

            onShapes = true;
            break;
        } else {
            update();
            continue;
        }
    }
    update();
    return onShapes;

    //TODO: selectUnique
}

bool ShapesWidget::clickedOnPoint(QPointF point1, QPointF point2,
                                  QPointF point3, QPointF point4, QPointF pos) {
    m_isSelected = false;
    m_isResize = false;
    m_isRotated = false;

    m_pressedPoint = QPoint(0, 0);

    if (pointClickIn(point1, pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = First;
        m_resizeDirection = TopLeft;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(point2, pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Second;
        m_resizeDirection = BottomLeft;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(point3, pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Third;
        m_resizeDirection = TopRight;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(point4, pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Fourth;
        m_resizeDirection = BottomRight;
        m_pressedPoint = pos;
        return true;
    } else if (rotateOnPoint(point1, point2, point3, point4, pos)) {
        m_isSelected = true;
        m_isRotated = true;
        m_isResize = false;
        m_resizeDirection = Rotate;
        m_pressedPoint = pos;
        return true;
    } else if (pointOnLine(point1, point2, pos) || pointOnLine(point1, point3, pos)
             || pointOnLine(point2, point4, pos) || pointOnLine(point3, point4, pos)) {
        m_isSelected = true;
        m_isResize = false;
        m_resizeDirection = Moving;
        m_pressedPoint = pos;
        return true;
    } else {
        m_isSelected = false;
        m_isResize = false;
        m_isRotated = false;
    }

    return false;
}

bool ShapesWidget::hoverOnShapes(QPointF point1, QPointF point2,
                                  QPointF point3, QPointF point4, QPointF pos) {
    if (pointClickIn(point1, pos)) {
        m_resizeDirection = TopLeft;
        return true;
    } else if (pointClickIn(point2, pos)) {
        m_resizeDirection = BottomLeft;
        return true;
    } else if (pointClickIn(point3, pos)) {
        m_resizeDirection = TopRight;
        return true;
    } else if (pointClickIn(point4, pos)) {
        m_resizeDirection = BottomRight;
        return true;
    } else if (rotateOnPoint(point1, point2, point3, point4, pos)) {
        m_resizeDirection = Rotate;
        return true;
    } else if (pointOnLine(point1, point2, pos) || pointOnLine(point1, point3, pos)
             || pointOnLine(point2, point4, pos) || pointOnLine(point3, point4, pos)) {
        m_resizeDirection = Moving;
        return true;
    } else {
        m_resizeDirection = Outting;
    }
    return false;
}

bool ShapesWidget::rotateOnPoint(QPointF point1, QPointF point2,
                                 QPointF point3, QPointF point4,
                                 QPointF pos) {
    bool result = hoverOnRotatePoint(point1, point2, point3, point4, pos);
    return result;
}

bool ShapesWidget::hoverOnRotatePoint(QPointF point1, QPointF point2,
                                      QPointF point3, QPointF point4,
                                      QPointF pos) {
    QPointF rotatePoint = getRotatePoint(point1, point2, point3, point4);
    rotatePoint = QPointF(rotatePoint.x() - 5, rotatePoint.y() - 5);
    bool result = false;
    if (pos.x() >= rotatePoint.x() - SPACING && pos.x() <= rotatePoint.x() + SPACING
            && pos.y() >= rotatePoint.y() - SPACING && pos.y() <= rotatePoint.y() + SPACING) {
        result = true;
    } else {
        result = false;
    }

    m_pressedPoint = rotatePoint;
    return result;
}

////////////////////TODO: perfect handleRotate..
void ShapesWidget::handleRotate(QPointF pos) {
    QList<QPointF> tmpFourPoint;
    tmpFourPoint.append(m_currentSelectedFPoints.point1);
    tmpFourPoint.append(m_currentSelectedFPoints.point2);
    tmpFourPoint.append(m_currentSelectedFPoints.point3);
    tmpFourPoint.append(m_currentSelectedFPoints.point4);

    QPointF centerInPoint = QPointF((m_currentSelectedFPoints.point1.x() + m_currentSelectedFPoints.point4.x())/2,
                                  (m_currentSelectedFPoints.point1.y() + m_currentSelectedFPoints.point4.y())/2);

    qreal angle = calculateAngle(m_pressedPoint, pos, centerInPoint)/35;
    for (int i = 0; i < 4; i++) {
        tmpFourPoint[i] = pointRotate(centerInPoint, tmpFourPoint[i], angle);
    }  
    m_diagPointsList[m_selectedIndex].masterPoint = tmpFourPoint[0];
    m_diagPointsList[m_selectedIndex].deputyPoint = tmpFourPoint[3];

    m_mFPointsList[m_selectedIndex].point1 = tmpFourPoint[0];
    m_mFPointsList[m_selectedIndex].point2 = tmpFourPoint[1];
    m_mFPointsList[m_selectedIndex].point3 = tmpFourPoint[2];
    m_mFPointsList[m_selectedIndex].point4 = tmpFourPoint[3];

    m_currentSelectedFPoints.point1 = tmpFourPoint[0];
    m_currentSelectedFPoints.point2 = tmpFourPoint[1];
    m_currentSelectedFPoints.point3 = tmpFourPoint[2];
    m_currentSelectedFPoints.point4 = tmpFourPoint[3];

    m_currentHoveredFPoints.point1 = tmpFourPoint[0];
    m_currentHoveredFPoints.point2 = tmpFourPoint[1];
    m_currentHoveredFPoints.point3 = tmpFourPoint[2];
    m_currentHoveredFPoints.point4 = tmpFourPoint[3];

    m_pressedPoint = pos;
}

void ShapesWidget::handleResize(QPointF pos, int key) {
        if (m_isResize) {
            QList<QPointF> tmpFourPoint;
            tmpFourPoint.append(m_currentSelectedFPoints.point1);
            tmpFourPoint.append(m_currentSelectedFPoints.point2);
            tmpFourPoint.append(m_currentSelectedFPoints.point3);
            tmpFourPoint.append(m_currentSelectedFPoints.point4);
            FourPoints newResizeFPoints = resizePointPosition(tmpFourPoint[0], tmpFourPoint[1],
                    tmpFourPoint[2], tmpFourPoint[3], pos, key,  m_isShiftPressed);
        qDebug() << "___________newResizeFPoints:" << newResizeFPoints.point1 << "\n"
                 << newResizeFPoints.point2 << "\n"
                 << newResizeFPoints.point3 <<  "\n"
                 << newResizeFPoints.point4 << "\n";
            m_mFPointsList[m_selectedIndex].point1 = newResizeFPoints.point1;
            m_mFPointsList[m_selectedIndex].point2 = newResizeFPoints.point2;
            m_mFPointsList[m_selectedIndex].point3 = newResizeFPoints.point3;
            m_mFPointsList[m_selectedIndex].point4 = newResizeFPoints.point4;
            m_currentSelectedFPoints.point1 = newResizeFPoints.point1;
            m_currentSelectedFPoints.point2 = newResizeFPoints.point2;
            m_currentSelectedFPoints.point3 = newResizeFPoints.point3;
            m_currentSelectedFPoints.point4 = newResizeFPoints.point4;
            m_currentHoveredFPoints.point1 = newResizeFPoints.point1;
            m_currentHoveredFPoints.point2 = newResizeFPoints.point2;
            m_currentHoveredFPoints.point3 = newResizeFPoints.point3;
            m_currentHoveredFPoints.point4 = newResizeFPoints.point4;
        }
}

void ShapesWidget::mousePressEvent(QMouseEvent *e) {
    m_pressedPoint = e->pos();
    m_isPressed = true;
    if (!clickedOnShapes(m_pressedPoint)) {
        qDebug() << "no one shape be clicked!";
        m_currentDiagPoints.masterPoint = QPoint(0, 0);
        m_currentDiagPoints.deputyPoint = QPoint(0, 0);
        m_currentSelectedFPoints.point1 = QPoint(0, 0);
        m_currentSelectedFPoints.point2 = QPoint(0, 0);
        m_currentSelectedFPoints.point3 = QPoint(0, 0);
        m_currentSelectedFPoints.point4 = QPoint(0, 0);

        m_selectedIndex = -1;
        m_isRecording = true;
        if (m_pos1 == QPointF(0, 0)) {

            m_pos1 = e->pos();
            m_currentDiagPoints.masterPoint = m_pos1;
            m_shapesMap.insert(m_shapesMap.count(), m_currentShape);
            update();
        }
    } else {
        m_isRecording = false;
        qDebug() << "some on shape be clicked!";
    }


    QFrame::mousePressEvent(e);
}

void ShapesWidget::mouseReleaseEvent(QMouseEvent *e) {
    m_isPressed = false;
    if (m_isRecording && !m_isSelected && m_pos2 != QPointF(0, 0)) {
        m_currentDiagPoints.deputyPoint = m_pos2;
        m_diagPointsList.append(m_currentDiagPoints);

        FourPoints tmpPoints = fourPointsOnRect(m_currentDiagPoints);
        m_mFPointsList.append(tmpPoints);
        m_isRecording = false;
        m_isMoving = false;
    }

    m_pos1 = QPointF(0, 0);
    m_pos2 = QPointF(0, 0);


    update();

    QFrame::mouseMoveEvent(e);
}

void ShapesWidget::mouseMoveEvent(QMouseEvent *e) {
    m_isMoving = true;
    m_movingPoint = e->pos();

    if (m_isRecording && m_isPressed) {
        m_pos2 = e->pos();
        m_currentDiagPoints.deputyPoint = m_pos2;
        update();
    } else if (!m_isRecording && m_isPressed) {
        if (m_isRotated && m_isPressed) {
            handleRotate(e->pos());
            update();
        }

        if (m_isResize && m_isPressed) {
            //TODO resize function
            qDebug() << "$$$$$$" << m_clickedKey;
             handleResize(QPointF(e->pos()), m_clickedKey);
            update();
            return;
        }

        if (m_isSelected && m_isPressed && m_selectedIndex != -1) {
            m_mFPointsList[m_selectedIndex].point1 = QPointF(
                        m_mFPointsList[m_selectedIndex].point1.x() + (m_movingPoint.x() - m_pressedPoint.x()),
                        m_mFPointsList[m_selectedIndex].point1.y() + (m_movingPoint.y() - m_pressedPoint.y()));
            m_mFPointsList[m_selectedIndex].point2 = QPointF(
                        m_mFPointsList[m_selectedIndex].point2.x() + (m_movingPoint.x() - m_pressedPoint.x()),
                        m_mFPointsList[m_selectedIndex].point2.y() + (m_movingPoint.y() - m_pressedPoint.y()));
            m_mFPointsList[m_selectedIndex].point3 = QPointF(
                        m_mFPointsList[m_selectedIndex].point3.x() + (m_movingPoint.x() - m_pressedPoint.x()),
                        m_mFPointsList[m_selectedIndex].point3.y() + (m_movingPoint.y() - m_pressedPoint.y()));
            m_mFPointsList[m_selectedIndex].point4 = QPointF(
                        m_mFPointsList[m_selectedIndex].point4.x() + (m_movingPoint.x() - m_pressedPoint.x()),
                        m_mFPointsList[m_selectedIndex].point4.y() + (m_movingPoint.y() - m_pressedPoint.y()));

            m_currentSelectedFPoints = m_mFPointsList[m_selectedIndex];
            m_currentHoveredFPoints = m_mFPointsList[m_selectedIndex];
            m_currentSelectedDiagPoints.masterPoint = m_mFPointsList[m_selectedIndex].point1;
            m_currentSelectedDiagPoints.deputyPoint = m_mFPointsList[m_selectedIndex].point4;
            m_pressedPoint = e->pos();
            update();
        }
    } else {
        if (!m_isRecording) {
            m_isHovered = false;
            for (int i = 0; i < m_mFPointsList.length(); i++) {
                 FourPoints tmpHoverFPoints =  m_mFPointsList[i];
                 if (hoverOnShapes(tmpHoverFPoints.point1, tmpHoverFPoints.point2,
                     tmpHoverFPoints.point3, tmpHoverFPoints.point4, e->pos())) {
                     m_isHovered = true;
                     m_currentHoverDiagPoints = m_diagPointsList[i];
                     m_currentHoveredFPoints = tmpHoverFPoints;
                     if (m_resizeDirection == TopLeft) {
                         if (m_isSelected || m_isRotated) {
                            qApp->setOverrideCursor(Qt::SizeFDiagCursor);
                         } else {
                            qApp->setOverrideCursor(Qt::ClosedHandCursor);
                         }
                     } else if (m_resizeDirection == BottomLeft) {
                         if (m_isSelected || m_isRotated) {
                            qApp->setOverrideCursor(Qt::SizeBDiagCursor);
                         } else {
                            qApp->setOverrideCursor(Qt::ClosedHandCursor);
                         }
                     } else if (m_resizeDirection == TopRight) {
                         if (m_isSelected || m_isRotated) {
                            qApp->setOverrideCursor(Qt::SizeBDiagCursor);
                         } else {
                            qApp->setOverrideCursor(Qt::ClosedHandCursor);
                         }
                     } else if (m_resizeDirection == BottomRight) {
                         if (m_isSelected || m_isRotated) {
                            qApp->setOverrideCursor(Qt::SizeFDiagCursor);
                         } else {
                            qApp->setOverrideCursor(Qt::ClosedHandCursor);
                         }
                     } else if (m_resizeDirection == Rotate) {
                         qApp->setOverrideCursor(setCursorShape("rotate"));
                     } else if (m_resizeDirection == Moving) {
                         qApp->setOverrideCursor(Qt::ClosedHandCursor);
                     } else {
                         qApp->setOverrideCursor(setCursorShape("rect"));
                     }
                     update();
                     break;
                 } else {
                     qApp->setOverrideCursor(setCursorShape(m_currentShape));
                     update();
                 }
            }
            if (!m_isHovered) {
                m_currentHoveredFPoints.point1 = QPointF(0, 0);
                m_currentHoveredFPoints.point2 = QPointF(0, 0);
                m_currentHoveredFPoints.point3 = QPointF(0, 0);
                m_currentHoveredFPoints.point4 = QPointF(0, 0);
                m_currentHoverDiagPoints.masterPoint = QPointF(0, 0);
                m_currentHoverDiagPoints.deputyPoint = QPointF(0, 0);
                update();
            }
            if (m_mFPointsList.length() == 0) {
                qApp->setOverrideCursor(setCursorShape(m_currentShape));
            }
        } else {
            //TODO text
        }
    }

    QFrame::mouseMoveEvent(e);
}

void ShapesWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    QPen pen;
    pen.setColor(Qt::red);
    pen.setWidth(3);
    painter.setPen(pen);

    if (m_currentDiagPoints.masterPoint != QPointF(0, 0) &&
            m_currentDiagPoints.deputyPoint != QPointF(0, 0)) {
        FourPoints currentFPoint = fourPointsOnRect(m_currentDiagPoints);
        painter.drawLine(currentFPoint.point1, currentFPoint.point2);
        painter.drawLine(currentFPoint.point2, currentFPoint.point4);
        painter.drawLine(currentFPoint.point4, currentFPoint.point3);
        painter.drawLine(currentFPoint.point3, currentFPoint.point1);
    }

    for(int i = 0; i < m_mFPointsList.length(); i++) {
        painter.drawLine(m_mFPointsList[i].point1, m_mFPointsList[i].point2);
        painter.drawLine(m_mFPointsList[i].point2, m_mFPointsList[i].point4);
        painter.drawLine(m_mFPointsList[i].point4, m_mFPointsList[i].point3);
        painter.drawLine(m_mFPointsList[i].point3, m_mFPointsList[i].point1);
    }


    if (m_currentSelectedDiagPoints.masterPoint != QPointF(0, 0) &&
            m_currentSelectedDiagPoints.deputyPoint != QPointF(0, 0)) {
        painter.drawPixmap(QPointF(m_currentSelectedFPoints.point1.x() - DRAG_BOUND_RADIUS,
                                  m_currentSelectedFPoints.point1.y() - DRAG_BOUND_RADIUS),
                           QPixmap(":/resources/images/size/resize_handle_big.png"));
        painter.drawPixmap(QPointF(m_currentSelectedFPoints.point2.x() - DRAG_BOUND_RADIUS,
                                  m_currentSelectedFPoints.point2.y() - DRAG_BOUND_RADIUS),
                           QPixmap(":/resources/images/size/resize_handle_big.png"));
        painter.drawPixmap(QPointF(m_currentSelectedFPoints.point3.x() - DRAG_BOUND_RADIUS,
                                  m_currentSelectedFPoints.point3.y() - DRAG_BOUND_RADIUS),
                           QPixmap(":/resources/images/size/resize_handle_big.png"));
        painter.drawPixmap(QPointF(m_currentSelectedFPoints.point4.x() - DRAG_BOUND_RADIUS,
                                  m_currentSelectedFPoints.point4.y() - DRAG_BOUND_RADIUS),
                           QPixmap(":/resources/images/size/resize_handle_big.png"));

        QPointF rotatePoint = getRotatePoint(m_currentSelectedFPoints.point1, m_currentSelectedFPoints.point2,
                m_currentSelectedFPoints.point3, m_currentSelectedFPoints.point4);
        QPointF middlePoint((m_currentSelectedFPoints.point1.x() + m_currentSelectedFPoints.point3.x())/2,
                           (m_currentSelectedFPoints.point1.y() + m_currentSelectedFPoints.point3.y())/2);
        painter.setPen(QColor(Qt::white));
        painter.drawLine(rotatePoint, middlePoint);
        painter.drawPixmap(QPointF(rotatePoint.x() - 12, rotatePoint.y() - 12),
                           QPixmap(":/resources/images/size/rotate.png"));
    }

    if (m_currentHoverDiagPoints.masterPoint != QPointF(0, 0)) {
        pen.setWidth(1);
        pen.setColor(QColor(Qt::white));
        painter.setPen(pen);
        painter.drawLine(m_currentHoveredFPoints.point1, m_currentHoveredFPoints.point2);
        painter.drawLine(m_currentHoveredFPoints.point2, m_currentHoveredFPoints.point4);
        painter.drawLine(m_currentHoveredFPoints.point4, m_currentHoveredFPoints.point3);
        painter.drawLine(m_currentHoveredFPoints.point3, m_currentHoveredFPoints.point1);
    }
}

bool ShapesWidget::eventFilter(QObject *watched, QEvent *event) {
    Q_UNUSED(watched);

    if (event->type() == QEvent::Enter) {
        if (m_currentShape == "rect") {
            qApp->setOverrideCursor(setCursorShape("rect"));
        }
    }

    return false;
}
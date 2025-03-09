#include <gtest/gtest.h>
#include <QApplication>
#include <QVector3D>

#include "../src/OpenGLWidget.h"

TEST(BezierCurveTest, BasicCalculation)
{
    QVector3D p0(0.0f, 0.0f, 0.0f);
    QVector3D p1(0.25f, 0.5f, 0.0f);
    QVector3D p2(0.75f, -0.5f, 0.0f);
    QVector3D p3(1.0f, 0.0f, 0.0f);

    QVector3D resultStart = OpenGLWidget::bezierCurve(p0, p1, p2, p3, 0.0f);
    EXPECT_FLOAT_EQ(resultStart.x(), p0.x());
    EXPECT_FLOAT_EQ(resultStart.y(), p0.y());
    EXPECT_FLOAT_EQ(resultStart.z(), p0.z());

    QVector3D resultEnd = OpenGLWidget::bezierCurve(p0, p1, p2, p3, 1.0f);
    EXPECT_FLOAT_EQ(resultEnd.x(), p3.x());
    EXPECT_FLOAT_EQ(resultEnd.y(), p3.y());
    EXPECT_FLOAT_EQ(resultEnd.z(), p3.z());

    // resultMid(0.5f, 0.0f, 0.0f)
    QVector3D resultMid = OpenGLWidget::bezierCurve(p0, p1, p2, p3, 0.5f);
    EXPECT_NEAR(resultMid.x(), 0.5f, 0.01f);    // Right
    EXPECT_NEAR(resultMid.x(), 0.9f, 0.01f);    // Error
    EXPECT_NEAR(resultMid.y(), 0.0f, 0.01f);
    EXPECT_FLOAT_EQ(resultMid.z(), 0.0f);
}

TEST(OpenGLWidgetTest, Initialization)
{
    OpenGLWidget widget;
    EXPECT_TRUE(true);
}

int main(int argc, char** argv)
{
    // 初始化QApplication
    QApplication app(argc, argv);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
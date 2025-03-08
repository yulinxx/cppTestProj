//Windows下OSG从编译到使用 - CSDN博客
//https ://blog.csdn.net/hhhhhh098/article/details/145700206

#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osgViewer/Viewer>
#include <osg/Group>
#include <osg/Material>
#include <osg/StateSet>
#include <osg/Geometry>
#include <cstdlib>
#include <ctime>

osg::ref_ptr<osg::Geode> createRandomColoredSphere(float radius, unsigned int subdivisions)
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;

    // 创建球体的顶点和颜色数据
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;

    // 随机数种子
    std::srand(std::time(0));

    // 创建球体表面的顶点（简单的球体网格细分）
    for (unsigned int i = 0; i <= subdivisions; ++i) {
        float phi = osg::PI * i / subdivisions;  // 纵向角度
        for (unsigned int j = 0; j <= subdivisions; ++j) {
            float theta = 2 * osg::PI * j / subdivisions;  // 横向角度

            // 球体上的一个点
            float x = radius * sin(phi) * cos(theta);
            float y = radius * sin(phi) * sin(theta);
            float z = radius * cos(phi);

            vertices->push_back(osg::Vec3(x, y, z));

            // 随机分配颜色（蓝色或绿色）
            if (std::rand() % 2 == 0) {
                colors->push_back(osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f));  // 蓝色
            }
            else {
                colors->push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f));  // 绿色
            }
        }
    }

    // 添加顶点和颜色数据到几何体
    geometry->setVertexArray(vertices);
    geometry->setColorArray(colors, osg::Array::BIND_PER_VERTEX);

    // 创建球体的三角形面
    osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(GL_TRIANGLES);

    for (unsigned int i = 0; i < subdivisions; ++i) 
    {
        for (unsigned int j = 0; j < subdivisions; ++j) 
        {
            unsigned int first = i * (subdivisions + 1) + j;
            unsigned int second = first + subdivisions + 1;

            // 生成两个三角形（为了每个四边形）
            indices->push_back(first);
            indices->push_back(second);
            indices->push_back(first + 1);

            indices->push_back(second);
            indices->push_back(second + 1);
            indices->push_back(first + 1);
        }
    }

    geometry->addPrimitiveSet(indices);

    // 将几何体添加到 Geode
    geode->addDrawable(geometry);

    return geode;
}


int main()
{
    // 创建一个随机颜色的球体
    osg::ref_ptr<osg::Geode> sphere = createRandomColoredSphere(1.0f, 20);

    // 创建 Viewer
    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow(100, 100, 800, 600);  // 参数分别为：x, y, width, height

    viewer.setSceneData(sphere);

    // 运行 Viewer
    return viewer.run();
}
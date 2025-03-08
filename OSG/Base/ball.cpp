//Windows��OSG�ӱ��뵽ʹ�� - CSDN����
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

    // ��������Ķ������ɫ����
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;

    // ���������
    std::srand(std::time(0));

    // �����������Ķ��㣨�򵥵���������ϸ�֣�
    for (unsigned int i = 0; i <= subdivisions; ++i) {
        float phi = osg::PI * i / subdivisions;  // ����Ƕ�
        for (unsigned int j = 0; j <= subdivisions; ++j) {
            float theta = 2 * osg::PI * j / subdivisions;  // ����Ƕ�

            // �����ϵ�һ����
            float x = radius * sin(phi) * cos(theta);
            float y = radius * sin(phi) * sin(theta);
            float z = radius * cos(phi);

            vertices->push_back(osg::Vec3(x, y, z));

            // ���������ɫ����ɫ����ɫ��
            if (std::rand() % 2 == 0) {
                colors->push_back(osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f));  // ��ɫ
            }
            else {
                colors->push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f));  // ��ɫ
            }
        }
    }

    // ��Ӷ������ɫ���ݵ�������
    geometry->setVertexArray(vertices);
    geometry->setColorArray(colors, osg::Array::BIND_PER_VERTEX);

    // �����������������
    osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(GL_TRIANGLES);

    for (unsigned int i = 0; i < subdivisions; ++i) 
    {
        for (unsigned int j = 0; j < subdivisions; ++j) 
        {
            unsigned int first = i * (subdivisions + 1) + j;
            unsigned int second = first + subdivisions + 1;

            // �������������Σ�Ϊ��ÿ���ı��Σ�
            indices->push_back(first);
            indices->push_back(second);
            indices->push_back(first + 1);

            indices->push_back(second);
            indices->push_back(second + 1);
            indices->push_back(first + 1);
        }
    }

    geometry->addPrimitiveSet(indices);

    // ����������ӵ� Geode
    geode->addDrawable(geometry);

    return geode;
}


int main()
{
    // ����һ�������ɫ������
    osg::ref_ptr<osg::Geode> sphere = createRandomColoredSphere(1.0f, 20);

    // ���� Viewer
    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow(100, 100, 800, 600);  // �����ֱ�Ϊ��x, y, width, height

    viewer.setSceneData(sphere);

    // ���� Viewer
    return viewer.run();
}
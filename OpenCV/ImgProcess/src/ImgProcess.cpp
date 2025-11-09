#include "ImgProcess.h"
// #include <opencv2/freetype.hpp>

using namespace ImgSpace;

imgProcess::imgProcess()
{
}

imgProcess::imgProcess(std::string& strPath)
{
    this->readImg(strPath);
}

imgProcess::~imgProcess()
{
}

cv::Mat imgProcess::readImg(std::string& strPath, int nType /*=1*/)
{
    Mat matImg = imread(strPath);
    int nChannel = matImg.channels();

    if (matImg.empty())
    {
        // printf("不能加载图像！\n");
        return Mat();
    }

    // imshow("PNG", matImg);

    m_matOri = matImg;
    return m_matOri;
}

void imgProcess::saveImg(const Mat& mat, std::string strPath)
{
    if (mat.data)
    {
        imwrite(strPath, mat);
        m_matOri = mat;
    }
}

Mat imgProcess::getOriginImg()
{
    return m_matOri;
}

Mat imgProcess::resizeImg(const Mat& mat, double dRatio)
{
    if (fabs(dRatio) < 0.001)
        dRatio = 0.001;

    Mat matRes;
    Size dsize = Size(mat.cols * dRatio, mat.rows * dRatio);
    resize(mat, matRes, dsize);

    return matRes;
}

Mat imgProcess::setImgMix(const Mat& matA, Mat& matB, double dAlaphaA /*= 0.5*/, double dAlaphaB /*= 0.5*/, int nType /*=-1*/)
{
    if (!matA.data || !matB.data)
        return Mat();

    Mat matRes;

    Mat matACopy = matA.clone();
    int nCh = matACopy.channels();
    if (nCh < 3)
        cvtColor(matA, matA, COLOR_GRAY2BGR);

    Mat matBCopy = matB.clone();
    nCh = matBCopy.channels();
    if (nCh < 3)
        cvtColor(matBCopy, matBCopy, COLOR_GRAY2BGR);

    // addWeighted(InputArray src1, double alpha, InputArray src2,
    //     double beta, double gamma, OutputArray dst, int dtype = -1);
    addWeighted(matACopy, dAlaphaA * 0.1, matBCopy, 1 - (dAlaphaA * 0.1), dAlaphaB * 0.1, matRes, nType);

    if (matRes.data)
        return matRes;
    else
        return m_matOri;
}

Mat imgProcess::getGray(const Mat& mat)
{
    if (1 == mat.channels())
        return mat;

    // 将图片转换成灰度图片
    Mat matGray;
    if (mat.channels() > 2)
    {
        cvtColor(mat, matGray, COLOR_BGR2GRAY);
        return matGray;
    }
    else
        return mat;

    // // https://blog.csdn.net/mooneve/article/details/53001677
    // int rows = mat.rows;
    // int cols = mat.cols;
    // //生成和img同样大小的空白灰度图像
    // Mat grayImg = Mat(rows, cols, CV_8U);
    // for (int i = 0; i < rows; i++)
    // {
    //     //获取图像每一行的首地址
    //     Vec3b *p = mat.ptr<Vec3b>(i);      //彩色图                      //p指向地址
    //     uchar *p2 = grayImg.ptr<uchar>(i); //灰度图                 //*p指向内容
    //     for (int j = 0; j < cols; j++)
    //     {
    //         //每次迭代获取图像列的地址
    //         Vec3b &pix = *p++;   //彩色图                             //pix指向内容
    //         uchar &pix2 = *p2++; //灰度图                           //&pix指向地址
    //         pix2 = pix[0] * 0.114 + pix[1] * 0.587 + pix[2] * 0.299;
    //     }
    // }
    // return grayImg;
}

Mat imgProcess::setThreshold(const Mat& mat, int nValueA, int nValueB /*= 255*/, int nType /*= 0*/)
{
    // threshold(m_oriImg, m_matRes, dValueA, dValueB, nType);
    //  cv::THRESH_BINARY

    // threshold<> 会接受到5个参数:
    // src_gray: 输入的灰度图像的地址.
    // matRes: 输出图像的地址.
    // threshold_value: 进行阈值操作时阈值的大小.
    // max_BINARY_value: 设定的最大灰度值(该参数运用在二进制与反二进制阈值操作中).
    // threshold_type: 阈值的类型.从上面提到的5种中选择出的结果.

    Mat matGray = this->getGray(mat);

    Mat matRes;
    cv::threshold(matGray, matRes, nValueA, nValueB, nType % 5);
    return matRes;
}

Mat imgProcess::getRedChannel(const Mat& mat)
{
    vector<Mat> mv;

    split(mat, mv);
    mv[0] = Scalar(0);
    mv[1] = Scalar(0);
    Mat matRes;
    merge(mv, matRes);

    return matRes;
}

Mat imgProcess::getGreeChannel(const Mat& mat)
{
    vector<Mat> mv;

    split(mat, mv);
    mv[0] = Scalar(0);
    mv[2] = Scalar(0);
    Mat matRes;
    merge(mv, matRes);

    return matRes;
}

Mat imgProcess::getBlueChannel(const Mat& mat)
{
    vector<Mat> mv;

    split(mat, mv);
    mv[1] = Scalar(0);
    mv[2] = Scalar(0);
    Mat matRes;
    merge(mv, matRes);

    return matRes;
}

Mat imgProcess::getRGChannel(const Mat& mat)
{
    vector<Mat> mv;

    split(mat, mv);
    mv[0] = Scalar(0);
    Mat matRes;
    merge(mv, matRes);

    return matRes;
}

Mat imgProcess::getRBChannel(const Mat& mat)
{
    vector<Mat> mv;

    split(mat, mv);
    mv[1] = Scalar(0);
    Mat matRes;
    merge(mv, matRes);

    return matRes;
}

// 绿蓝
Mat imgProcess::getGBChannel(const Mat& mat)
{
    vector<Mat> mv;

    split(mat, mv);
    mv[2] = Scalar(0);
    Mat matRes;
    merge(mv, matRes);
    return matRes;
}

Mat imgProcess::setRotateImg(const Mat& mat, double dAngle /*=0.0*/, bool bChangeSize /*= false*/)
{
    if (bChangeSize)
    {
        // https://www.cnblogs.com/konglongdanfo/p/9135501.html
        float theta = dAngle * CV_PI / 180.0;
        int nRowsSrc = mat.rows;
        int nColsSrc = mat.cols;

        // 如果是顺时针旋转
        bool direction = true;
        if (!direction)
            theta = 2 * CV_PI - theta;

        // 全部以逆时针旋转来计算
        // 逆时针旋转矩阵
        float matRotate[3][3]{
            {std::cos(theta), -std::sin(theta), 0},
            {std::sin(theta), std::cos(theta), 0},
            {0, 0, 1} };

        float pt[3][2]{
            {0, (float)nRowsSrc},
            {(float)nColsSrc, (float)nRowsSrc},
            {(float)nColsSrc, 0} };

        for (int i = 0; i < 3; i++)
        {
            float x = pt[i][0] * matRotate[0][0] + pt[i][1] * matRotate[1][0];
            float y = pt[i][0] * matRotate[0][1] + pt[i][1] * matRotate[1][1];
            pt[i][0] = x;
            pt[i][1] = y;
        }

        // 计算出旋转后图像的极值点和尺寸
        float fMin_x = min(min(min(pt[0][0], pt[1][0]), pt[2][0]), (float)0.0);
        float fMin_y = min(min(min(pt[0][1], pt[1][1]), pt[2][1]), (float)0.0);
        float fMax_x = max(max(max(pt[0][0], pt[1][0]), pt[2][0]), (float)0.0);
        float fMax_y = max(max(max(pt[0][1], pt[1][1]), pt[2][1]), (float)0.0);

        int nRows = cvRound(fMax_y - fMin_y + 0.5) + 1;
        int nCols = cvRound(fMax_x - fMin_x + 0.5) + 1;
        int nMin_x = cvRound(fMin_x + 0.5);
        int nMin_y = cvRound(fMin_y + 0.5);

        bool bOneCh = false;
        if (1 == mat.channels())
            bOneCh = true;

        // 拷贝输出图像
        Mat matRet(nRows, nCols, mat.type(), Scalar(0));
        for (int j = 0; j < nRows; j++)
        {
            for (int i = 0; i < nCols; i++)
            {
                // 计算出输出图像在原图像中的对应点的坐标,然后复制该坐标的灰度值
                // 因为是逆时针转换,所以这里映射到原图像的时候可以看成是,输出图像
                // 到顺时针旋转到原图像的,而顺时针旋转矩阵刚好是逆时针旋转矩阵的转置
                // 同时还要考虑到要把旋转后的图像的左上角移动到坐标原点.
                int x = (i + nMin_x) * matRotate[0][0] + (j + nMin_y) * matRotate[0][1];
                int y = (i + nMin_x) * matRotate[1][0] + (j + nMin_y) * matRotate[1][1];

                if (x >= 0 && x < nColsSrc && y >= 0 && y < nRowsSrc)
                {
                    if (bOneCh)
                        matRet.at<uchar>(j, i) = mat.at<uchar>(y, x);
                    else
                        matRet.at<Vec3b>(j, i) = mat.at<Vec3b>(y, x);
                }
            }
        }
        return matRet;
    }

    else
    {
        Mat matTemp = mat.clone();
        int nCenterX = matTemp.cols / 2;
        int nCenterY = matTemp.rows / 2;

        Mat matRes = getRotationMatrix2D(Point(nCenterX, nCenterY), (dAngle - 180), 1.0);

        Mat matOut;
        warpAffine(matTemp, matOut, matRes, matTemp.size(), INTER_LINEAR, 0, Scalar());

        return matOut;
    }
}

Mat imgProcess::setScaleImg(const Mat& mat, double dScale /*= 1.0*/)
{
    if (fabs(dScale) < 0.000001)
        dScale = 0.000001;

    int nCenterX = mat.rows / 2;
    int nCenterY = mat.cols / 2;

    // Mat matRotation = getRotationMatrix2D(Point(nCenterX, nCenterY), (nAngle - 180), iScale / 50.0);
    Mat matRes = getRotationMatrix2D(Point(nCenterX, nCenterY), 0.0, dScale);

    warpAffine(mat, matRes, matRes, mat.size(), INTER_LINEAR, 0, Scalar());

    return matRes;
}

Mat imgProcess::setScaleImg(const Mat& mat, double dScaleX, double dScaleY)
{
    if (fabs(dScaleX) < 0.000001)
        dScaleX = 0.000001;

    if (fabs(dScaleY) < 0.000001)
        dScaleY = 0.000001;

    int nCenterX = mat.rows / 2;
    int nCenterY = mat.cols / 2;

    Mat matRes;
    Size dsize = Size(mat.cols * dScaleX, mat.rows * dScaleY);
    resize(mat, matRes, dsize);

    return matRes;
}
Mat imgProcess::rotate90(const Mat& mat, int nIndex /*= 0*/)
{
    Mat matRes;

    nIndex %= 4;
    if (nIndex == 3)
        return mat;

    rotate(mat, matRes, nIndex); // 0 1 2 顺时针旋转 90 180 270 度

    return matRes;
}

Mat imgProcess::setErodeImg(const Mat& mat, int w /*= 5*/, int h /*= 5*/, int nType /*= 0*/) // 腐蚀
{
    if (w < 1 || h < 1)
        return mat;

    Mat matRes;
    if (0 == nType % 2)
    {
        // getStructuringElement函数返回的是指定形状和尺寸的结构元素
        //获取自定义核  MORPH_RECT表示矩形的卷积核,当然还可以选择椭圆形的、交叉型的
        Mat element = getStructuringElement(MORPH_RECT, Size(w, h));
        erode(mat, matRes, element); // 腐蚀操作
    }
    else
    {
        Mat element = getStructuringElement(MORPH_RECT, Size(w, h));
        dilate(mat, matRes, element);
    }

    return matRes;
}

Mat imgProcess::setBlurImg(const Mat& mat, int w /*= 7*/, int h /*= 7*/) // 模糊
{
    if (w < 1 || h < 1)
        return mat;

    Mat matRes;
    blur(mat, matRes, Size(w, h)); // 模糊
    return matRes;
}

Mat imgProcess::setCannyImg(const Mat& mat, int a, int b, int c, int w /*= 13*/, int h /*= 13*/) // 提边
{
    // if(w < 1 || h < 1 || a < 1 || b<1 || c<1)
    //     return m_matRes;

    if (w < 1)
        w = 3;

    if (w % 2 == 0)
        w += 1;

#if 0
    Mat matBlur;
    const char* window_name = "Sobel Demo - Simple Edge Detector";
    int scale = 1;
    int delta = 0;
    int ddepth = CV_16S;

    // 对原图像使用 GaussianBlur 降噪(内核大小 = 3)
    GaussianBlur(mat, matBlur, Size(w, w), 0, 0, BORDER_DEFAULT); // 高斯平滑

    /// 转换为灰度图
    Mat matGray = getGray(matBlur);

    /// 创建 matGradX 和 matGradY 矩阵
    Mat matGradX, matGradY;
    Mat matGradAbsX, matGradAbsY;

    //在 x 和 y 方向分别”求导“. 为此,我们使用函数 Sobel :
    // src_gray: 在本例中为输入图像,元素类型 CV_8U
    // matGradX / matGradY : 输出图像.
    // ddepth : 输出图像的深度,设定为 CV_16S 避免外溢.
    // x_order : x 方向求导的阶数.
    // y_order : y 方向求导的阶数.
    // scale, delta 和 BORDER_DEFAULT : 使用默认值

    //范围内, 计算绝对值, 并将结果转换为8位.
    // 输入数组中的每一个元素,函数 convertScaleAbs 顺序执行三个操作 :缩放、取绝对值、转换成一个无符号8位类型.
    // void cv::convertScaleAbs(
    //    cv::InputArray mat, // 输入数组
    //    cv::OutputArray matRes, // 输出数组
    //    double alpha = 1.0, // 乘数因子
    //    double beta = 0.0 // 偏移量
    //);

    /// 求 X方向梯度
    // Scharr( src_gray, matGradX, ddepth, 1, 0, scale, delta, BORDER_DEFAULT );
    Sobel(matGray, matGradX, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT);
    convertScaleAbs(matGradX, matGradAbsX);

    /// 求Y方向梯度
    // Scharr( src_gray, matGradY, ddepth, 0, 1, scale, delta, BORDER_DEFAULT );
    Sobel(matGray, matGradY, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT);
    convertScaleAbs(matGradY, matGradAbsY);

    /// 合并梯度(近似) // addWeighted使用OpenCV对两幅图像求和(求混合(blending))
    addWeighted(matGradAbsX, 0.5, matGradAbsY, 0.5, 0, m_matRes);
    return m_matRes;

#endif

    if (a < 1)
        a = 1;
    if (b < 1)
        b = 1;
    if (c < 3)
        c = 3;

    Mat src, matGray;
    Mat matRes, detected;

    /// 创建与src同类型和大小的矩阵(matRes)
    matRes.create(mat.size(), mat.type());

    cvtColor(mat, matGray, COLOR_BGR2GRAY);

    /// 使用 3x3内核降噪
    blur(matGray, detected, Size(w, h));

    /// 运行Canny算子
    Canny(detected, detected, a, b, c);
    // Canny( detected, detected,  3, 59, 3);

    /// 使用 Canny算子输出边缘作为掩码显示原图像
    matRes = Scalar::all(0);

    mat.copyTo(matRes, detected);
    // imshow("Test", matRes );

    return matRes;
}

Mat imgProcess::setContrastAndBright(const Mat& mat, double dH, double dS, double dV)
{
    // // 查到一般HSV的范围是
    // // H: [0,360]
    // // S: [0,100]
    // // V: [0,100]

    // // 但是在openCV中,HSV的范围却是
    // // H: [0,180]
    // // S: [0,255]
    // // V: [0,255]

    // // 那是因为openCV做了如下的变换

    // // h= H/2;
    // // s = (float)S/100*255;
    // // v = (float)v/100*255;
    // // https://www.jianshu.com/p/7361652e15b8

    // // H指hue(色相)、S指saturation(饱和度)、L指lightness(亮度)、V指value(色调)、B指brightness(明度)
    // // HSL是色相(Hue)、饱和度(Saturation)和亮度(Lightness)

    // 色相调节
    Mat matHSV;

    int nCh = mat.channels();

    if (nCh < 3)
        return mat;

    if (nCh == 4)
        cvtColor(mat, matHSV, COLOR_RGBA2BGR);

    cvtColor(mat, matHSV, COLOR_BGR2HSV);
    std::vector<Mat> vecPlanes;
    split(matHSV, vecPlanes);

    const Mat& matTemp = vecPlanes[0];
    int nW = matTemp.cols;
    int nH = matTemp.rows;

    for (int y = 0; y < nH; y++)
    {
        for (int x = 0; x < nW; x++)
        {
            // uchar h = matHSV.at<Vec3b>(y, x)[0];
            // if (h + dH > 180)
            //     h = (h + dH) - 180;
            // else
            //     h = h + dH;

            // matHSV.at<Vec3b>(y, x)[0] = h;      // 色相调节

            // uchar s = matHSV.at<Vec3b>(y, x)[1];
            // if (s + dS > 100)
            //     s = (s + dS) - 100;
            // else
            //     s = s + dS;
            // matHSV.at<Vec3b>(y, x)[1] = s;

            // uchar l = matHSV.at<Vec3b>(y, x)[2];
            // if (l + dV > 100)
            //     l = (l + dV) - 100;
            // else
            //     l = s + dV;
            // matHSV.at<Vec3b>(y, x)[2] = l;

            // 色相
            signed short h = matHSV.at<Vec3b>(y, x)[0];
            signed short h_plus_shift = h;
            h_plus_shift += dH;

            if (h_plus_shift < 0)
                h = 180 + h_plus_shift;
            else if (h_plus_shift > 180)
                h = h_plus_shift - 180;
            else
                h = h_plus_shift;

            matHSV.at<Vec3b>(y, x)[0] = static_cast<unsigned char>(h);

            // 饱和度
            double v = matHSV.at<Vec3b>(y, x)[2];
            // double v_shift = (dV - 100) / 100.0;

            double v_plus_shift = v + dS;

            if (v_plus_shift < 0)
                v_plus_shift = 0;
            else if (v_plus_shift > 255)
                v_plus_shift = 255;

            matHSV.at<Vec3b>(y, x)[2] = static_cast<unsigned char>(v_plus_shift);

            // 明度
            double s = matHSV.at<Vec3b>(y, x)[1];
            // double s_shift = (dS - 100) / 100.0
            double s_plus_shift = s + dV;

            if (s_plus_shift < 0)
                s_plus_shift = 0;
            else if (s_plus_shift > 255)
                s_plus_shift = 255;

            matHSV.at<Vec3b>(y, x)[1] = static_cast<unsigned char>(s_plus_shift);
        }
    }

    Mat matShow;
    cvtColor(matHSV, matShow, COLOR_HSV2BGR);
    return matShow;

    // // 所以我们需要一个新的Mat对象,以存储变换后的图像.我们希望这个Mat对象拥有下面的性质:
    // // 像素值初始化为0 与原图像有相同的大小和类型
    // Mat matRes = Mat::zeros(mat.size(), mat.type());
    // int channels = mat.channels(); //获取图像通道数

    // switch (channels)
    // {
    // case 1:
    //     for (int row = 0; row < mat.rows; row++)
    //     {
    //         for (int col = 0; col < mat.cols; col++)
    //         {
    //             float v = mat.at<uchar>(row, col);
    //             // saturate_cast 限制结果在0-255之间
    //             matRes.at<uchar>(row, col) = saturate_cast<uchar>(v * dH * 0.01 + dS); //调整
    //         }
    //     }
    // case 3:
    //     for (int row = 0; row < mat.rows; row++)
    //     {
    //         for (int col = 0; col < mat.cols; col++)
    //         {
    //             //     float b = mat.at<Vec3b>(row, col)[0];// blue
    //             //     float g = mat.at<Vec3b>(row, col)[1]; // green
    //             //     float r = mat.at<Vec3b>(row, col)[2]; // red
    //             //     //调整
    //             //     matRes.at<Vec3b>(row, col)[0] = saturate_cast<uchar>(b * dContrast * 0.01 + dBright);
    //             //     matRes.at<Vec3b>(row, col)[1] = saturate_cast<uchar>(g * dContrast * 0.01 + dBright);
    //             //     matRes.at<Vec3b>(row, col)[2] = saturate_cast<uchar>(r * dContrast * 0.01 + dBright);

    //             // 或
    //             for (int c = 0; c < 3; c++)
    //             {
    //                 //    实现调整图片亮度和对比度的公式 new_img = a*original_img + b
    //                 //      a 表示图片的对比度,> 1,就是高对比度;如果在 0 - 1 之间,那就是低对比度; 等于 1,表示没有任何变化
    //                 //      b  数值范围是 - 127 到 127;
    //                 matRes.at<Vec3b>(row, col)[c] = saturate_cast<uchar>((mat.at<Vec3b>(row, col)[c] * dH * 0.01) + dS);
    //             }
    //         }
    //     }
    // }

    // // m_matRes = matRes;
    // return matRes;
}

Mat imgProcess::setSharpening(const Mat& mat, double sigmaX /*= 100.0*/, double sigmaY /*= 0.0*/, int nBorderType /*=BORDER_DEFAULT*/)
{
    if (sigmaX < 1)
        return mat;

    // USM锐化
    Mat blurUsm, matRes;
    // CV_EXPORTS_W void GaussianBlur(InputArray src, OutputArray matRes, Size ksize,
    //     double sigmaX, double sigmaY = 0,
    //     int borderType = BORDER_DEFAULT);
    //  cv::GaussianBlur(mat, blurUsm, Size(0, 0), sigmaX, sigmaY, nBorderType); // 高斯滤波GaussianBlur
    cv::GaussianBlur(mat, blurUsm, Size(0, 0), sigmaX);
    cv::addWeighted(mat, 1.5, blurUsm, -0.5, 0, matRes);
    return matRes;
}

Mat imgProcess::setImgContours(const Mat& mat, int nBrushSize)
{
    /// 转成灰度并模糊化降噪
    Mat matGray = getGray(mat);

    blur(matGray, matGray, Size(3, 3));

    Mat canny_output;
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;

    /// 用Canny算子检测边缘
    Canny(matGray, canny_output, nBrushSize, nBrushSize * 2, 3);

    /// 寻找轮廓
    findContours(canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

    /// 绘出轮廓
    Mat matDraw = Mat::zeros(canny_output.size(), CV_8UC3);
    for (int i = 0; i < contours.size(); i++)
    {
        // Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
        Scalar color = Scalar(255, 255, 0);
        drawContours(matDraw, contours, i, color, 2, 1, hierarchy, 0, Point());
    }

    return matDraw;
}

Mat imgProcess::setImgBlur(const Mat& mat, int nBrushSize, int nType)
{
    if (nBrushSize < 1)
        nBrushSize = 3;
    if (nBrushSize % 2 == 0)
        nBrushSize += 1;

    Mat matCpy = mat.clone();

    // 归一化块滤波器(Normalized Box Filter)  就是 均值平滑
    // 最简单的滤波器, 输出像素值是核窗口内像素值的 均值(所有像素加权系数相等)

    int DELAY_CAPTION = 1500;
    int DELAY_BLUR = 300;
    int MAX_KERNEL_LENGTH = 31;

    switch (nType % 4)
    {
    case 0:
    {
        /// 使用 均值平滑
        // mat: 输入图像
        // matCpy : 输出图像
        // Size(w, h) : 定义内核大小(w 像素宽度, h 像素高度)
        // Point(-1, -1) : 指定锚点位置(被平滑点), 如果是负值,取核的中心为锚点.
        blur(mat, matCpy, Size(nBrushSize, nBrushSize), Point(-1, -1)); // 归一化块滤波器  均值平滑
    }
    break;
    case 1:
    {
        /// 使用高斯平滑 (Gaussian Filter)
        //高斯滤波是将输入数组的每一个像素点与 高斯内核 卷积将卷积和当作输出像素值.
        // mat: 输入图像
        // matCpy : 输出图像
        // Size(w, h) : 定义内核的大小(需要考虑的邻域范围). w 和 h 必须是正奇数,否则将使用 \sigma_{ x } 和 \sigma_{ y } 参数来计算内核大小.
        //\sigma_{ x }: x 方向标准方差, 如果是 0 则 \sigma_{ x } 使用内核大小计算得到.
        //\sigma_{ y }: y 方向标准方差, 如果是 0 则 \sigma_{ y } 使用内核大小计算得到..
        GaussianBlur(mat, matCpy, Size(nBrushSize, nBrushSize), 0, 0); // 高斯平滑
    }
    break;
    case 2:
    {
        /// 使用中值平滑  (Median Filter)
        // 中值滤波将图像的每个像素用邻域 (以当前像素为中心的正方形区域)像素的 中值 代替
        // mat: 输入图像
        // matCpy : 输出图像, 必须与 mat 相同类型
        // nBrushSize : 内核大小(只需一个值,因为我们使用正方形窗口),必须为奇数.
        medianBlur(mat, matCpy, nBrushSize); // 中值滤波
    }
    break;
    case 3:
    {
        /// 使用双边平滑 (Bilateral Filter)
        // mat: 输入图像
        // matCpy : 输出图像
        // d : 像素的邻域直径
        //\sigma_{ Color }: 颜色空间的标准方差
        //\sigma_{ Space }: 坐标空间的标准方差(像素单位)
        bilateralFilter(mat, matCpy, nBrushSize, nBrushSize * 2, nBrushSize / 2); // 双边滤波
    }
    break;
    }

    return matCpy;
}

// https://blog.csdn.net/weixin_44394801/article/details/118077769
Mat imgProcess::setMosaic(const Mat& mat, int nBrushSize, int nType)
{
    if (nBrushSize < 3)
        nBrushSize = 3;

    Mat matRes;
    if (mat.channels() < 3)
        cvtColor(mat, matRes, COLOR_GRAY2BGR);
    else
        matRes = mat;

    Mat matMosaic;

    int nW = matRes.cols; // 宽
    int nH = matRes.rows; // 高

    // Mat matRes(mat.size(), CV_8UC3);
    for (int i = 0; i < nH; i += nBrushSize)
    {
        for (int j = 0; j < nW; j += nBrushSize)
        {
            // 获取像素点
            Scalar scalar = Scalar(mat.at<Vec3b>(i, j));

            // 将像素点赋给Mat

            for (int m = i; m < i + nBrushSize && m < nH; m++)
            {
                for (int n = j; n < j + nBrushSize && n < nW; n++)
                {
                    // 将Mat移至原图覆盖
                    // Mat matM(nBrushSize, nBrushSize, CV_8UC3, scalar);
                    matRes.at<Vec3b>(m, n)[0] = scalar[0];
                    matRes.at<Vec3b>(m, n)[1] = scalar[1];
                    matRes.at<Vec3b>(m, n)[2] = scalar[2];
                }
            }
        }
    }

    return matRes;
}

// 浮雕
Mat imgProcess::setRelief(const Mat& mat, int nBrushSize, int nType)
{
    Mat matImgA(mat.size(), CV_8UC3);

    for (int y = 1; y < mat.rows - 1; y++)
    {
        const uchar* p0 = mat.ptr<uchar>(y);
        const uchar* p1 = mat.ptr<uchar>(y + 1);

        uchar* q0 = matImgA.ptr<uchar>(y);

        for (int x = 1; x < mat.cols - 1; x++)
        {
            for (int i = 0; i < 3; i++)
            {
                // 当前点和右边一个点相减,再加150
                int tmp0 = p1[3 * (x + 1) + i] - p0[3 * (x - 1) + i] + nBrushSize; //浮雕

                if (tmp0 < 0)
                    q0[3 * x + i] = 0;
                else if (tmp0 > 255)
                    q0[3 * x + i] = 255;
                else
                    q0[3 * x + i] = tmp0;
            }
        }
    }

    cvtColor(matImgA, matImgA, COLOR_BGR2GRAY);
    return matImgA;

    // Mat matImgA(mat.size(), CV_8UC3);
    // Mat matImgB(mat.size(), CV_8UC3);

    // for (int y = 1; y < mat.rows - 1; y++)
    // {
    //     uchar *p0 = mat.ptr<uchar>(y);
    //     uchar *p1 = mat.ptr<uchar>(y + 1);

    //     uchar *q0 = matImgA.ptr<uchar>(y);
    //     uchar *q1 = matImgB.ptr<uchar>(y);

    //     for (int x = 1; x < mat.cols - 1; x++)
    //     {
    //         for (int i = 0; i < 3; i++)
    //         {
    //             int tmp0 = p1[3 * (x + 1) + i] - p0[3 * (x - 1) + i] + 128; //浮雕
    //             if (tmp0 < 0)
    //                 q0[3 * x + i] = 0;
    //             else if (tmp0 > 255)
    //                 q0[3 * x + i] = 255;
    //             else
    //                 q0[3 * x + i] = tmp0;

    //             int tmp1 = p0[3 * (x - 1) + i] - p1[3 * (x + 1) + i] + 128; //雕刻
    //             if (tmp1 < 0)
    //                 q1[3 * x + i] = 0;
    //             else if (tmp1 > 255)
    //                 q1[3 * x + i] = 255;
    //             else
    //                 q1[3 * x + i] = tmp1;
    //         }
    //     }
    // }

    // Mat matRes;
    // vector<Mat> vImgs;
    // vImgs.emplace_back(matImgA);
    // vImgs.emplace_back(matImgB);
    // hconcat(vImgs, matRes); //水平方向拼接
    // // vconcat(vImgs, matRes); //垂直方向拼接
    // m_matRes = matRes;
    // return matRes;
}

// 素描
Mat imgProcess::setImgSketch(const Mat& mat, int nBrushSize, int nType)
{
    int nW = mat.cols;
    int heigh = mat.rows;

    Mat matGrayA, matGrayB;

    matGrayA = getGray(mat);

    addWeighted(matGrayA, -1.0, 0, 0.0, 255.0, matGrayB);

    if (nBrushSize <= 0)
        nBrushSize = 3;
    if (nBrushSize % 2 == 0)
        nBrushSize += 1;

    GaussianBlur(matGrayB, matGrayB, Size(nBrushSize, nBrushSize), 0);

    // 融合:颜色减淡
    Mat matRes(matGrayB.size(), CV_8UC1);
    for (int y = 0; y < heigh; y++)
    {
        uchar* pA = matGrayA.ptr<uchar>(y);
        uchar* pB = matGrayB.ptr<uchar>(y);
        uchar* p = matRes.ptr<uchar>(y);
        for (int x = 0; x < nW; x++)
        {
            int nA = pA[x];
            int nB = pB[x];
            p[x] = (uchar)min((nA + (nA * nB) / (256 - nB)), 255);
        }
    }
    // imshow(" ", matRes);
    return matRes;
}

Mat imgProcess::setColorStyle(const Mat& mat, int nA)
{
    if (nA < 0)
        nA = 0;

    // int width = mat.cols;
    // int heigh = mat.rows;
    Mat matGray = getGray(mat);

    // Mat matImgColor[12];
    Mat matRes(mat.size(), CV_8UC3);

    // for (int i = 0; i < 12; i++)
    {
        applyColorMap(matGray, matRes, nA % 12);
        // int x = i % 4;
        // int y = i / 4;
        // Mat displayROI = matRes(Rect(x * width, y * heigh, width, heigh));

        // resize(InputArray mat, OutputArray matDst, Size dsize...);
        //  resize(matImgColor[i], displayROI, displayROI.size());
    }
    // imshow("colorImg", matRes);
    return matRes;
}

Mat imgProcess::setImgOilPaint(const Mat& mat, int nBrushSize, int nCoarseness)
{
    // 1.把(0~255)灰度值均分成n个区间
    // 2.遍历图像的每个像素点 将模板范围内的所有像素值进一步离散化,根据像素的灰度落入不同的区间,
    // 3.找到落入像素最多的一个区间 并桶对该区间中的所有像素求出颜色平均值 作为位置 (x, y) 的结果值
    // 链接:https://www.jianshu.com/p/7237529e1eb1

    //将一副图像转换成油画 - yxysuanfa - 博客园
    // https ://www.cnblogs.com/yxysuanfa/p/7289369.html

    // 油画原理:
    // 一、获取图像灰度图,灰度值范围0~255
    // 二、mask大小,比如5*5,计算mask对应灰度图像的灰度等级(0~level)
    // 三、将当前mask,level占有最多数量的像素点,对应到彩色图像进行RGB求和,算平均值.
    // 四、以此类推,最终得到油画特效后的彩色图像.

    //  将图像分为8x8的多个小块,并统计小块每个像素的灰度值
    //  划分灰度等级,例如0-255: 0-63, 64-127, ...
    //  找出每个小块中,每个等级的个数,找出最多的
    //  用最多的的平均值替换原像素

    // nBrushSize = 5;  // 笔刷大小
    // nCoarseness = 6; // 粗糙度

    assert(!mat.empty());
    if (nBrushSize < 1)
        nBrushSize = 1;
    if (nBrushSize > 8)
        nBrushSize = 8;

    if (nCoarseness < 1)
        nCoarseness = 1;
    if (nCoarseness > 255)
        nCoarseness = 255;

    int nW = mat.cols;
    int nH = mat.rows;

    int nLenArray = nCoarseness + 1;
    int* pCountIntensity = new int[nLenArray];

    uint* pRedAverage = new uint[nLenArray];
    uint* pGreenAverage = new uint[nLenArray];
    uint* pBlueAverage = new uint[nLenArray];

    /// 图像灰度化
    Mat matGray = getGray(mat);

    /// 目标图像
    Mat matRes = Mat::zeros(mat.size(), mat.type());

    // 1.把(0~255)灰度值均分成n个区间
    for (int nY = 0; nY < nH; nY++)
    {
        // 油画渲染范围上下边界
        int nTop = nY - nBrushSize;
        int nBottom = nY + nBrushSize + 1;

        if (nTop < 0)
            nTop = 0;
        if (nBottom >= nH)
            nBottom = nH - 1;

        for (int nX = 0; nX < nW; nX++)
        {
            // 油画渲染范围左右边界
            int nLeft = nX - nBrushSize;
            int nRight = nX + nBrushSize + 1;

            if (nLeft < 0)
                nLeft = 0;
            if (nRight >= nW)
                nRight = nW - 1;

            //初始化数组
            for (int i = 0; i < nLenArray; i++)
            {
                pCountIntensity[i] = 0;
                pRedAverage[i] = 0;
                pGreenAverage[i] = 0;
                pBlueAverage[i] = 0;
            }

            // 下面这个内循环相似于外面的大循环 也是油画特效处理的关键部分

            // 2.遍历图像的每个像素点 将模板范围内的所有像素值进一步离散化,根据像素的灰度落入不同的区间,
            // 3.找到落入像素最多的一个区间 并桶对该区间中的所有像素求出颜色平均值 作为位置 (x, y) 的结果值
            for (int j = nTop; j < nBottom; j++)
            {
                for (int i = nLeft; i < nRight; i++)
                {
                    // 像素值进一步离散化
                    int intensity = static_cast<uchar>(nCoarseness * (int)matGray.at<uchar>(j, i) / 255.0);

                    pCountIntensity[intensity]++;

                    pRedAverage[intensity] += mat.at<Vec3b>(j, i)[2];
                    pGreenAverage[intensity] += mat.at<Vec3b>(j, i)[1];
                    pBlueAverage[intensity] += mat.at<Vec3b>(j, i)[0];
                }
            }

            // 求最大值,并记录下数组索引
            int chosenIntensity = 0;
            int maxInstance = pCountIntensity[0];
            for (int i = 1; i < nLenArray; i++)
            {
                if (pCountIntensity[i] > maxInstance)
                {
                    chosenIntensity = i;
                    maxInstance = pCountIntensity[i];
                }
            }

            matRes.at<Vec3b>(nY, nX)[2] = static_cast<uchar>(pRedAverage[chosenIntensity] /
                static_cast<float>(maxInstance));
            matRes.at<Vec3b>(nY, nX)[1] = static_cast<uchar>(pGreenAverage[chosenIntensity] /
                static_cast<float>(maxInstance));
            matRes.at<Vec3b>(nY, nX)[0] = static_cast<uchar>(pBlueAverage[chosenIntensity] /
                static_cast<float>(maxInstance));
        }
    }

    delete[] pCountIntensity;
    delete[] pRedAverage;
    delete[] pGreenAverage;
    delete[] pBlueAverage;
    return matRes;
}

// 挂网
Mat imgProcess::setDotPaint(const Mat& mat, int nBrushSize, int nCoarseness)
{
    assert(!mat.empty());

    int nW = mat.cols;
    int nH = mat.rows;

    if (nBrushSize < 1)
        nBrushSize = 5;

    if (nBrushSize >= nW)
        nBrushSize = nW - 2;

    if (nBrushSize % 2 == 0)
        nBrushSize += 1;

    double scale = 1;
    Mat matResize = mat;
    Size dsize = Size(matResize.cols * scale, matResize.rows * scale);
    resize(mat, matResize, dsize);

    // 图像灰度化
    Mat matGray = getGray(matResize);
    // imshow("Gray", matGray);

    nW = matGray.cols;
    nH = matGray.rows;

    // 目标图像
    Mat matRes(matResize.size(), matResize.type(), Scalar(255, 255, 255));

    // 遍历图像
    for (int nY = 0; nY < nH; nY += nBrushSize)
    {
        int nTop = nY;
        int nBottom = nY + nBrushSize;

        if (nBottom >= nH)
            nBottom = nH - 1;

        for (int nX = 0; nX < nW; nX += nBrushSize)
        {
            int nLeft = nX;
            int nRight = nX + nBrushSize;

            if (nRight >= nW)
                nRight = nW - 1;

            const int nCount = nBrushSize * nBrushSize;
            int nGrayCount = 0;
            int nGrayMin = 256;
            int nGrayMax = -1;

            // 区间中心点
            int nMinX = nX + nBrushSize * 0.5;
            int nMinY = nY + nBrushSize * 0.5;

            // std::vector<int> vecValue;
            // vecValue.reserve(nCount);
            // vecValue.resize(nCount, 255);
            std::vector<float> vecDistance;
            vecDistance.reserve(nCount);
            // vecDistance.resize(nCount, 0);

            // 遍历小区间

            int nBlack = 0; // 小于126的个数
            int nGray = 0;  // 灰度的个数
            int nWhite = 0; // 大于126的个数

            for (int m = nTop; m < nBottom; m++)
            {
                for (int n = nLeft; n < nRight; n++)
                {
                    int nValue = static_cast<uchar>(matGray.at<uchar>(m, n));
                    if (nValue < 85)
                        nBlack++;
                    else if (nValue > 170)
                        nWhite++;
                    else
                        nGray++;

                    float dDis = sqrt(pow(abs(n - nMinX), 2) + pow(abs(m - nMinY), 2));
                    vecDistance.emplace_back(dDis);

                    if (nValue > nGrayMax)
                        nGrayMax = nValue;
                    if (nValue < nGrayMin)
                        nGrayMin = nValue;
                    nGrayCount += nValue;
                }
            }

            if (vecDistance.size() < 1)
                continue;

            float dRatio = (nBlack + nGray * 0.5) / nCount;
            std::sort(vecDistance.begin(), vecDistance.end());
            int nSize = vecDistance.size() * dRatio;
            if (nSize >= vecDistance.size())
                nSize = (int)vecDistance.size() - 1;
            if (nSize < 0)
                nSize = 0;

            float dX = vecDistance[nSize]; // 分割线

            int nStep = (nGrayMax - nGrayMin) * 2 / nBrushSize;
            int nGrayAveragy = nGrayCount / nCount;

            // 遍历小区间
            for (int m = nTop; m < nBottom; m++)
            {
                int nFlag = 255;

                for (int n = nLeft; n < nRight; n++)
                {
                    nFlag--;
                    if (nFlag < 0)
                        nFlag = 255;

                    float dDis = sqrt(pow(abs(n - nMinX), 2) + pow(abs(m - nMinY), 2));
                    if (dDis >= dX)
                    {
                        matRes.at<Vec3b>(m, n)[0] = 255;
                        matRes.at<Vec3b>(m, n)[1] = 255;
                        matRes.at<Vec3b>(m, n)[2] = 255;
                    }
                    else
                    {
                        matRes.at<Vec3b>(m, n)[0] = 0;
                        matRes.at<Vec3b>(m, n)[1] = 0;
                        matRes.at<Vec3b>(m, n)[2] = 0;
                    }
                }
            }
        }
    }
    // imshow("Gray 2", matGray);
    scale = 1;
    dsize = Size(matRes.cols * scale, matRes.rows * scale);
    // Mat matResize = Mat(dsize, matResize.type());

    Mat matxx;
    resize(matRes, matxx, dsize);
    // imwrite("F:/img/maxttt.png", matRes);
    return matxx;
}

// 散点图
Mat imgProcess::setDither(const Mat& mat, int nBrushSize, int nCoarseness)
{
    Mat matDith = getGray(mat);

    int nW = matDith.cols;
    int nH = matDith.rows;

    /* Run the 'Floyd-Steinberg' dithering algorithm ... */
    int err;
    int8_t a, b, c, d;

    auto saturated_add = [](uint8_t val1, int8_t val2) -> uint8_t {
        int16_t val1_int = val1;
        int16_t val2_int = val2;
        int16_t tmp = val1_int + val2_int;

        if (tmp > 255)
            return 255;
        else if (tmp < 0)
            return 0;
        else
            return tmp;
        };

    for (int i = 0; i < nH; i++)
    {
        for (int j = 0; j < nW; j++)
        {
            if (matDith.at<uint8_t>(i, j) > 127)
            {
                err = matDith.at<uint8_t>(i, j) - 255;
                matDith.at<uint8_t>(i, j) = 255;
            }
            else
            {
                err = matDith.at<uint8_t>(i, j) - 0;
                matDith.at<uint8_t>(i, j) = 0;
            }

            a = (err * 7) / 16;
            b = (err * 1) / 16;
            c = (err * 5) / 16;
            d = (err * 3) / 16;

            if ((i != (nH - 1)) && (j != 0) && (j != (nW - 1)))
            {
                matDith.at<uint8_t>(i + 0, j + 1) = saturated_add(matDith.at<uint8_t>(i + 0, j + 1), a);
                matDith.at<uint8_t>(i + 1, j + 1) = saturated_add(matDith.at<uint8_t>(i + 1, j + 1), b);
                matDith.at<uint8_t>(i + 1, j + 0) = saturated_add(matDith.at<uint8_t>(i + 1, j + 0), c);
                matDith.at<uint8_t>(i + 1, j - 1) = saturated_add(matDith.at<uint8_t>(i + 1, j - 1), d);
            }
        }
    }

    // imshow("Raw Image", mat);
    // imshow("Dithered Image", matDith);
    // imwrite("F:/img/sdt.png", matDith);

    return matDith;
}

Mat imgProcess::setDither(const Mat& mat, double dScale, int nBright, int nType, int nAdjust, int nE)
{
    // Screw Ordered Dithering
    int Mask0[] = { 64, 53, 42, 26, 27, 43, 54, 61,
                   60, 41, 25, 14, 15, 28, 44, 55,
                   52, 40, 13, 5, 6, 16, 29, 45,
                   39, 24, 12, 1, 2, 7, 17, 30,
                   38, 23, 11, 4, 3, 8, 18, 31,
                   51, 37, 22, 10, 9, 19, 32, 41,
                   59, 50, 36, 21, 20, 33, 47, 56,
                   63, 58, 49, 35, 34, 48, 57, 62 };

    // CoarseFatting Ordered Dithering
    int Mask1[] = { 4, 14, 52, 58, 56, 45, 20, 6,
                   16, 26, 38, 50, 48, 36, 28, 18,
                   43, 35, 31, 9, 11, 25, 33, 41,
                   61, 46, 23, 1, 3, 13, 55, 60,
                   57, 47, 21, 7, 5, 15, 53, 59,
                   49, 37, 29, 19, 17, 27, 39, 51,
                   10, 24, 32, 40, 42, 34, 30, 8,
                   2, 12, 54, 60, 51, 44, 22, 0 };

    // 有序抖动算法 Bayer Ordered Dithering
    int Mask2[] = { 0, 32, 8, 40, 2, 34, 10, 42,
                   48, 16, 56, 42, 50, 18, 58, 26,
                   12, 44, 4, 36, 14, 46, 6, 38,
                   60, 28, 52, 20, 62, 30, 54, 22,
                   3, 35, 11, 43, 1, 33, 9, 41,
                   51, 19, 59, 27, 49, 17, 57, 25,
                   15, 47, 7, 39, 13, 45, 5, 37,
                   63, 31, 55, 23, 61, 29, 53, 21 };

    // Halftone Ordered Dithering
    int Mask3[] = { 28, 10, 18, 26, 36, 44, 52, 34,
                   22, 2, 4, 12, 48, 58, 60, 42,
                   14, 6, 0, 20, 40, 56, 62, 50,
                   24, 16, 8, 30, 32, 54, 46, 38,
                   37, 45, 53, 35, 29, 11, 19, 27,
                   49, 59, 61, 43, 23, 3, 5, 13,
                   41, 57, 63, 51, 15, 7, 1, 21,
                   33, 55, 47, 39, 25, 17, 9, 31 };

    // 12 * 12  大颗粒
    int Mask4[] = { 144, 140, 132, 122, 107, 63, 54, 93, 106, 123, 133, 142,
                   143, 137, 128, 104, 94, 41, 31, 65, 98, 116, 120, 139,
                   135, 131, 114, 97, 61, 35, 24, 55, 80, 103, 113, 125,
                   126, 117, 88, 83, 56, 29, 15, 51, 68, 90, 99, 111,
                   109, 100, 81, 77, 48, 22, 8, 28, 47, 76, 85, 96,
                   91, 44, 16, 12, 9, 3, 5, 21, 25, 33, 37, 73,
                   59, 58, 30, 18, 10, 1, 2, 4, 11, 19, 34, 42,
                   92, 64, 57, 52, 26, 6, 7, 14, 32, 46, 53, 74,
                   101, 95, 70, 67, 38, 13, 20, 36, 50, 75, 82, 108,
                   121, 110, 86, 78, 45, 17, 27, 39, 69, 79, 102, 119,
                   134, 129, 112, 89, 49, 23, 43, 60, 71, 87, 115, 127,
                   141, 138, 124, 118, 66, 40, 62, 72, 84, 105, 130, 136 };

    // 16 * 16
    int Mask5[] = { 0, 191, 48, 239, 12, 203, 60, 251, 3, 194, 51, 242, 15, 206, 63, 254,
                   127, 64, 175, 112, 139, 76, 187, 124, 130, 67, 178, 115, 142, 79, 190, 127,
                   32, 223, 16, 207, 44, 235, 28, 219, 35, 226, 19, 210, 47, 238, 31, 222,
                   159, 96, 143, 80, 171, 108, 155, 92, 162, 99, 146, 83, 174, 111, 158, 95,
                   8, 199, 56, 247, 4, 195, 52, 243, 11, 202, 59, 250, 7, 198, 55, 246,
                   135, 72, 183, 120, 131, 68, 179, 116, 138, 75, 186, 123, 134, 71, 182, 119,
                   40, 231, 24, 215, 36, 227, 20, 211, 43, 234, 27, 218, 39, 230, 23, 214,
                   167, 104, 151, 88, 163, 100, 147, 84, 170, 107, 154, 91, 166, 103, 150, 87,
                   2, 193, 50, 241, 14, 205, 62, 253, 1, 192, 49, 240, 13, 204, 61, 252,
                   129, 66, 177, 114, 141, 78, 189, 126, 128, 65, 176, 113, 140, 77, 188, 125,
                   34, 225, 18, 209, 46, 237, 30, 221, 33, 224, 17, 208, 45, 236, 29, 220,
                   161, 98, 145, 82, 173, 110, 157, 94, 160, 97, 144, 81, 172, 109, 156, 93,
                   10, 201, 58, 249, 6, 197, 54, 245, 9, 200, 57, 248, 5, 196, 53, 244,
                   137, 74, 185, 122, 133, 70, 181, 118, 136, 73, 184, 121, 132, 69, 180, 117,
                   42, 233, 26, 217, 38, 229, 22, 213, 41, 232, 25, 216, 37, 228, 21, 212,
                   169, 106, 153, 90, 165, 102, 149, 86, 168, 105, 152, 89, 164, 101, 148, 85 };

    // 9 * 9
    int Mask6[] = {
        53, 53, 54, 55, 56, 57, 58, 59, 60,
        51, 27, 28, 29, 30, 31, 32, 33, 61,
        50, 26, 10, 11, 12, 13, 14, 34, 62,
        49, 25, 9, 1, 2, 3, 15, 35, 63,
        80, 48, 24, 8, 0, 4, 16, 36, 64,
        79, 47, 23, 7, 6, 5, 17, 37, 65,
        78, 46, 22, 21, 20, 19, 18, 38, 66,
        77, 45, 44, 43, 42, 41, 40, 39, 67,
        76, 75, 74, 73, 72, 71, 70, 69, 68 };

    Mat matOri = mat.clone();
    Mat matTemp = setRotateImg(matOri, nE, true);

    // -------------------------------------------------
    Mat matTempx = matTemp.clone();
    // 对原图像使用 GaussianBlur 降噪(内核大小 = 3)
    GaussianBlur(matTempx, matTempx, Size(3, 3), 0, 0, BORDER_DEFAULT); // 高斯平滑

    // ------------------------------------------------
    // 锐化測試
    GaussianBlur(matTemp, matTempx, Size(49, 49), 26);
    addWeighted(matTempx, 1.0 + 3.0, matTempx, -3.0, 0, matTempx);

    // imshow("Test", matTempx);
    //  ------------------------------------------------

    Mat src_gray;
    /// 转换为灰度图
    cvtColor(matTempx, src_gray, COLOR_RGB2GRAY);

    /// 创建 grad_x 和 grad_y 矩阵
    Mat grad_x, grad_y;
    Mat abs_grad_x, abs_grad_y;

    int scalex = 1;
    int delta = 0;
    int ddepth = CV_16S;

    /// 求 X方向梯度
    // Scharr( src_gray, grad_x, ddepth, 1, 0, scalex, delta, BORDER_DEFAULT );
    Sobel(src_gray, grad_x, ddepth, 1, 0, 3, scalex, delta, BORDER_DEFAULT);
    convertScaleAbs(grad_x, abs_grad_x);

    /// 求Y方向梯度
    // Scharr( src_gray, grad_y, ddepth, 0, 1, scalex, delta, BORDER_DEFAULT );
    Sobel(src_gray, grad_y, ddepth, 0, 1, 3, scalex, delta, BORDER_DEFAULT);
    convertScaleAbs(grad_y, abs_grad_y);

    Mat gradEdge;
    /// 合并梯度(近似) // addWeighted使用OpenCV对两幅图像求和(求混合(blending))
    addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, gradEdge);

    // imshow("Grad", grad);

    // -------------------------------------------------

    // mask

    if (nType < 0)
        nType *= -1;

    if (mat.channels() < 3)
        cvtColor(mat, matOri, COLOR_GRAY2BGR);

    // Mat matTemp = matOri;
    // matTemp = setRotateImg(matOri, nE, true);

    Mat matGray = getGray(matTemp);

    // Mat matSmooth;
    // cvSmooth( matGray, matSmooth, CV_BLUR_NO_SCALE, 3, 0, 0, 0);

    // for (int y = 0; y < matGray.rows; y++)
    // {
    //     for (int x = 0; x < matGray.cols; x++)
    //     {
    //     }
    // }

    // 混合
    Mat matMix;
    addWeighted(gradEdge, 0.5, matGray, 0.5, 0, matMix);
    // imshow("matMix", matMix);

    // matGray = setRotateImg(matMix, nE, true);

    std::vector<int*> vecArr;
    vecArr.emplace_back(Mask0);
    vecArr.emplace_back(Mask1);
    vecArr.emplace_back(Mask2);
    vecArr.emplace_back(Mask3);
    vecArr.emplace_back(Mask4);
    vecArr.emplace_back(Mask5);
    vecArr.emplace_back(Mask6);

    if (dScale < 1)
        dScale = 1;

    if (dScale > 8)
        dScale = 8;

    int K = 8;
    int L = 8;
    int N = 63;

    if (nType % 7 == 4)
    {
        K = 12;
        L = 12;
        N = 123;
    }
    else if (nType % 7 == 5)
    {
        K = 16;
        L = 16;
        N = 255;
    }
    else if (nType % 7 == 5)
    {
        K = 16;
        L = 16;
        N = 255;
    }

    Size dsize = Size(matGray.cols * dScale, matGray.rows * dScale);
    Mat matGrayResize;
    resize(matGray, matGrayResize, dsize);

    uchar* p = nullptr;

    int nW = matGrayResize.cols;
    int nH = matGrayResize.rows;

    for (int y = 0; y < nH; y++)
    {
        // p = matGray.ptr(y);//获取每行首地址

        int k = y % K;

        for (int x = 0; x < nW; x++)
        {
            int l = x % L;

            // float nPix = matGray.at<uint8_t>(Point(x / scale, y / scale)) / 255.0 * N + 0.5 + nAdjust * 0.1;
            double dPix = matGray.at<uint8_t>(Point(x / dScale, y / dScale)) / 255.0 * N + 0.5 + nAdjust * 0.1;
            // auto color = matGray.at<Vec3b>(Point(x,y));
            // if(nPix > Mask[(k * L + l) % 64])
            if (dPix > vecArr[nType % 7][(k * L + l)])

                // if (nPix > Mask0[(k * L + l)])
            {
                matGrayResize.at<uint8_t>(Point(x, y)) = 255;
                // p[x] = 255; //对灰度图像素操作赋值
            }
            else
            {
                matGrayResize.at<uint8_t>(Point(x, y)) = 0;
                // p[x] = 0; //对灰度图像素操作赋值
            }
        }
    }

    Mat matOut;
    cvtColor(matGrayResize, matOut, COLOR_GRAY2BGR);
    Mat matOutX;
    matOutX = setRotateImg(matOut, -1 * nE, true);

    return matOutX;
}

// 镜像
Mat imgProcess::setImgMirror(const Mat& mat, int type /*= 0*/)
{
    int row = mat.rows;
    int col = mat.cols;
    Mat matRes = mat.clone();
    if (type % 2 == 0)
    {
        for (int i = 0; i < col; i++)
            mat.col(col - 1 - i).copyTo(matRes.col(i));
    }
    else
    {
        for (int i = 0; i < row; i++)
            mat.row(row - 1 - i).copyTo(matRes.row(i));
    }

    return matRes;
}

Mat imgProcess::setColorReversal(const Mat& mat, int type /*= 0*/)
{
    Mat outImg;
    // Mat dstimage = imread("F:/img/circle.png");
    // bitwise_xor(mat, dstimage, outImg);
    // imshow("bitwise_xor", outImg);

    // bitwise_or(mat, dstimage, outImg);
    // imshow("bitwise_or", outImg);

    // bitwise_and(mat, dstimage, outImg);
    // imshow("bitwise_and", outImg);

    bitwise_not(mat, outImg);
    return outImg;
}

Mat imgProcess::setColorTemperature(const Mat& mat, int nPercent)
{
    Mat matRes = mat.clone();
    int row = matRes.rows;
    int col = matRes.cols;
    int level = nPercent * 0.1;

    for (int i = 0; i < row; ++i)
    {
        uchar* a = matRes.ptr<uchar>(i);
        uchar* r = matRes.ptr<uchar>(i);

        for (int j = 0; j < col; ++j)
        {
            int R, G, B;

            // R通道
            R = a[j * 3 + 2];
            R = R + level;

            if (R > 255)
                r[j * 3 + 2] = 255;
            else if (R < 0)
                r[j * 3 + 2] = 0;
            else
                r[j * 3 + 2] = R;

            // G通道
            G = a[j * 3 + 1];
            G = G + level;
            if (G > 255)
                r[j * 3 + 1] = 255;
            else if (G < 0)
                r[j * 3 + 1] = 0;
            else
                r[j * 3 + 1] = G;

            // B通道
            B = a[j * 3];
            B = B - level;

            if (B > 255)
                r[j * 3] = 255;
            else if (B < 0)
                r[j * 3] = 0;
            else
                r[j * 3] = B;
        }
    }

    return matRes;
}

// 美颜
Mat imgProcess::setBeautify(const Mat& mat, int nA, int nB)
{
    int bilateralFilterVal = 30; // 双边模糊系数

    Mat srcMat = mat.clone();
    double dA = nA * 0.1;
    // dA = 1.1;
    // nB = 68;

    for (int y = 0; y < srcMat.rows; y++)
    {
        for (int x = 0; x < srcMat.cols; x++)
        {
            for (int c = 0; c < 3; c++)
            {
                srcMat.at<Vec3b>(y, x)[c] = saturate_cast<uchar>(dA * (srcMat.at<Vec3b>(y, x)[c]) + nB);
            }
        }
    }

    Mat matResult;
    GaussianBlur(srcMat, srcMat, Size(9, 9), 0, 0);        // 高斯模糊,消除椒盐噪声
    bilateralFilter(srcMat, matResult, bilateralFilterVal, // 整体磨皮
        bilateralFilterVal * 2, bilateralFilterVal / 2);

    Mat matFinal;

    // 图像增强,使用非锐化掩蔽(Unsharpening Mask)方案.
    cv::GaussianBlur(matResult, matFinal, cv::Size(0, 0), 9);
    cv::addWeighted(matResult, 1.5, matFinal, -0.5, 0, matFinal);
    return matFinal;
}

// 自动白平衡
Mat imgProcess::setAutoWhithBalance(const Mat& mat, double dA, double dB, double dC)
{
    int nRow = mat.rows;
    int nCol = mat.cols;
    Mat dst(nRow, nCol, CV_8UC3);
    int HistRGB[767] = { 0 };
    int nMaxVal = 0;

    for (int i = 0; i < nRow; i++)
    {
        for (int j = 0; j < nCol; j++)
        {
            nMaxVal = max(nMaxVal, (int)mat.at<Vec3b>(i, j)[0]);
            nMaxVal = max(nMaxVal, (int)mat.at<Vec3b>(i, j)[1]);
            nMaxVal = max(nMaxVal, (int)mat.at<Vec3b>(i, j)[2]);
            int sum = mat.at<Vec3b>(i, j)[0] + mat.at<Vec3b>(i, j)[1] + mat.at<Vec3b>(i, j)[2];
            HistRGB[sum]++;
        }
    }

    int Threshold = 0;
    int sum = 0;
    for (int i = 766; i >= 0; i--)
    {
        sum += HistRGB[i];
        if (sum > nRow * nCol * 0.1)
        {
            Threshold = i;
            break;
        }
    }

    int AvgB = 0;
    int AvgG = 0;
    int AvgR = 0;
    int cnt = 0;
    for (int i = 0; i < nRow; i++)
    {
        for (int j = 0; j < nCol; j++)
        {
            int sumP = mat.at<Vec3b>(i, j)[0] + mat.at<Vec3b>(i, j)[1] + mat.at<Vec3b>(i, j)[2];
            if (sumP > Threshold)
            {
                AvgB += mat.at<Vec3b>(i, j)[0];
                AvgG += mat.at<Vec3b>(i, j)[1];
                AvgR += mat.at<Vec3b>(i, j)[2];
                cnt++;
            }
        }
    }

    AvgB /= cnt;
    AvgG /= cnt;
    AvgR /= cnt;
    for (int i = 0; i < nRow; i++)
    {
        for (int j = 0; j < nCol; j++)
        {
            int Blue = mat.at<Vec3b>(i, j)[0] * nMaxVal / AvgB;
            int Green = mat.at<Vec3b>(i, j)[1] * nMaxVal / AvgG;
            int Red = mat.at<Vec3b>(i, j)[2] * nMaxVal / AvgR;
            if (Red > 255)
                Red = 255;
            else if (Red < 0)
                Red = 0;

            if (Green > 255)
                Green = 255;
            else if (Green < 0)
                Green = 0;

            if (Blue > 255)
                Blue = 255;
            else if (Blue < 0)
                Blue = 0;

            dst.at<Vec3b>(i, j)[0] = Blue;
            dst.at<Vec3b>(i, j)[1] = Green;
            dst.at<Vec3b>(i, j)[2] = Red;
        }
    }

    return dst;
}

Mat imgProcess::setColorReplace(const Mat& mat, int nR, int nG, int nB)
{
    int iterations = 5;
    Point anchor;
    Mat grayImage;
    Mat threshImage;
    Mat dilatedImage;
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;

    cvtColor(mat, grayImage, COLOR_RGB2GRAY);
    threshold(grayImage, threshImage, 150, 255, THRESH_BINARY_INV);
    Mat element = getStructuringElement(MORPH_CROSS, Size(3, 3));
    dilate(threshImage, dilatedImage, element, anchor = Point(-1, -1), iterations);
    findContours(dilatedImage, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE, Point(0, 0));

    vector<Rect> boundRect(contours.size());

    for (int contour = 0; contour < contours.size(); contour++)
    {
        boundRect[contour] = boundingRect(contours[contour]);
        rectangle(dilatedImage, boundRect[contour].tl(), boundRect[contour].br(), (255, 0, 255), 2);
    }

    // imshow("Contours", dilatedImage);
    return dilatedImage;
}

Mat imgProcess::drawLaserLine(const Mat& mat, double dA, double dB, double dC)
{
    if (dA < 1)
        dA = 1;

    int row = mat.rows;
    int col = mat.cols;

    // 二值化
    Mat matThreshold = setThreshold(mat, 98, 255, 0);
    // imshow("Threshold", matThreshold);

    int nTemp = matThreshold.channels();
    cv::Mat matDraw = cv::Mat(row, col, CV_8UC1, cv::Scalar::all(255));
    nTemp = matDraw.channels();
    int nStep = 5;
    for (int i = 0; i < col; i++)
    {
        for (int j = 0; j < row; j += dA)
        {
            int nValue = matThreshold.at<uchar>(Point(i, j));
            matDraw.at<uchar>(Point(i, j)) = nValue;
        }
    }

    return matDraw;
}

Mat imgProcess::setImgEdgeStrong(const Mat& mat, int nA, int nB, double dC)
{
    Mat matTempx = mat.clone();

    if (nA < 1)
        nA = 1;

    if (nA % 2 == 0)
        nA += 1;

    Mat matRes;
    GaussianBlur(mat, matRes, Size(nA, nA), nB); // 高斯平滑

    addWeighted(mat, 1.0 + 3.0 + dC, matRes, -3.0, 0, matRes);

    // imshow("Test", matRes);
    return matRes;
}

Mat imgProcess::setImgMask(const Mat& mat, double dA)
{
    Mat matMask(mat.size(), CV_8UC1, Scalar::all(0));

    std::vector<cv::Point> vecPts;
    vecPts.reserve(7);
    vecPts.emplace_back(Point(100, 200));
    vecPts.emplace_back(Point(300, 250));
    vecPts.emplace_back(Point(410, 350));
    vecPts.emplace_back(Point(360, 380));
    vecPts.emplace_back(Point(450, 420));
    vecPts.emplace_back(Point(250, 550));
    vecPts.emplace_back(Point(50, 450));

    std::vector<std::vector<cv::Point>> vecPtss;
    vecPtss.emplace_back(vecPts);

    //填充区域之前,首先采用polylines()函数,可以使填充的区域边缘更光滑
    cv::polylines(matMask, vecPtss, true, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
    cv::fillPoly(matMask, vecPtss, cv::Scalar(255, 255, 255));

    std::vector<cv::Point> vecPts2;
    vecPts2.reserve(7);
    vecPts2.emplace_back(Point(300, 200));
    vecPts2.emplace_back(Point(500, 250));
    vecPts2.emplace_back(Point(610, 350));
    vecPts2.emplace_back(Point(560, 380));
    vecPts2.emplace_back(Point(650, 420));
    vecPts2.emplace_back(Point(450, 550));
    vecPts2.emplace_back(Point(250, 450));

    std::vector<std::vector<cv::Point>> vecArrs2;
    vecArrs2.emplace_back(vecPts2);

    //填充区域之前,首先采用polylines()函数,可以使填充的区域边缘更光滑
    cv::polylines(matMask, vecArrs2, true, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
    cv::fillPoly(matMask, vecArrs2, cv::Scalar(255, 255, 255));

    if (dA < 0)
        bitwise_not(matMask, matMask);

    // imshow("Mask", matMask);

    // 原图与掩码进行操作
    Mat matDst = mat.clone();
    // bitwise_and(mat, mat. matDst, matMask);

    for (int x = 0; x < matDst.cols; x++)
    {
        for (int y = 0; y < matDst.rows; y++)
        {
            int nValue = matMask.at<uchar>(Point(x, y));
            if (nValue < 1)
            {
                cv::Vec3b& pixel = matDst.at<Vec3b>(Point(x, y));
                pixel[0] = 255;
                pixel[1] = 255;
                pixel[2] = 255;
            }
        }
    }

    return matDst;
}

// PNG 叠加到 JPG上
Mat imgProcess::setImgPngMerge(const Mat& mat, std::string strPngPath, int nXPos /*= 0*/, int nYPos /*= 0*/)
{
    Mat matRes = mat.clone();
    Mat matPng = imread(strPngPath, IMREAD_UNCHANGED);
    int nChannel = matPng.channels();

    if (matPng.empty())
        return Mat();

    const int channelNum = 3;
    int alpha = 0;

    for (int i = 0; i < matPng.rows; i++)
    {
        for (int j = 0; j < matPng.cols * 3; j += 3)
        {
            alpha = matPng.ptr<uchar>(i)[j / 3 * 4 + 3];

            if (alpha != 0)
            {
                for (int k = 0; k < 3; k++)
                {
                    if ((i + nYPos < matRes.rows) && (i + nYPos >= 0) &&
                        ((j + nXPos * 3) / 3 * 3 + k < matRes.cols * 3) && ((j + nXPos * 3) / 3 * 3 + k >= 0) &&
                        (i / channelNum * 4 + k < matRes.cols * 4) && (j / channelNum * 4 + k >= 0))
                    {
                        matRes.ptr<uchar>(i + nYPos)[(j + nXPos * channelNum) / channelNum * channelNum + k] =
                            matPng.ptr<uchar>(i)[(j) / channelNum * 4 + k];
                    }
                }
            }
        }
    }

    return matRes;
}

Mat imgProcess::setImgCut(const Mat& mat)
{
    // std::string strPath = "F:/img/trans.png";
    // Mat matPng = imread(strPath, IMREAD_UNCHANGED);
    // int nChannel = matPng.channels();

    // if (matPng.empty())
    //     return Mat();

    // int nW = matPng.cols;
    // int nH = matPng.rows;

    // Rect rect(0, 0, nW/2, nH/2);
    // Mat matCut = matPng(rect);
    // imwrite("F:/img/out/cutPng.png", matCut);
    // imshow("CutPng", matCut);
    // return matCut;

    std::string strPath = "F:/img/trans.png";
    Mat matPng = imread(strPath, IMREAD_UNCHANGED);
    if (matPng.empty())
        return Mat();

    int nChannel = matPng.channels();

    int x = 0;
    int y = 0;
    int nW = matPng.cols / 2;
    int nH = matPng.rows;

    for (int nx = x; nx < nW; nx++)
    {
        for (int ny = y; ny < nH; ny++)
        {
            cv::Vec4b& pixel = matPng.at<cv::Vec4b>(Point(nx, ny));
            pixel[3] = 0;
        }
    }

    imwrite("F:/img/out/cutPng.png", matPng);
    // imshow("CutPng", matPng);
    return matPng;
}

Mat imgProcess::imgTransparent(const Mat& mat, double dA, double dB, double dC)
{
    Mat matMask(mat.size(), CV_8UC1, Scalar::all(0));

    std::vector<cv::Point> vecPts;
    vecPts.reserve(7);
    vecPts.emplace_back(Point(100, 200));
    vecPts.emplace_back(Point(300, 250));
    vecPts.emplace_back(Point(410, 350));
    vecPts.emplace_back(Point(360, 380));
    vecPts.emplace_back(Point(450, 420));
    vecPts.emplace_back(Point(250, 550));
    vecPts.emplace_back(Point(50, 450));

    std::vector<std::vector<cv::Point>> vecPtss;
    vecPtss.emplace_back(vecPts);

    //填充区域之前,首先采用polylines()函数,可以使填充的区域边缘更光滑
    cv::polylines(matMask, vecPtss, true, cv::Scalar(225, 5, 5), 2, cv::LINE_AA);
    cv::fillPoly(matMask, vecPtss, cv::Scalar(125, 155, 155));

    std::vector<cv::Point> vecPts2;
    vecPts2.reserve(7);
    vecPts2.emplace_back(Point(300, 200));
    vecPts2.emplace_back(Point(500, 250));
    vecPts2.emplace_back(Point(610, 350));
    vecPts2.emplace_back(Point(560, 380));
    vecPts2.emplace_back(Point(650, 420));
    vecPts2.emplace_back(Point(450, 550));
    vecPts2.emplace_back(Point(250, 450));

    std::vector<std::vector<cv::Point>> vecArrs2;
    vecArrs2.emplace_back(vecPts2);

    cv::polylines(matMask, vecArrs2, true, cv::Scalar(225, 255, 255), 2, cv::LINE_AA);
    cv::fillPoly(matMask, vecArrs2, cv::Scalar(255, 255, 255));

    if ((int)dA % 2 == 0)
        bitwise_not(matMask, matMask);

    // 原图与掩码进行操作
    Mat matPng;
    cvtColor(mat, matPng, COLOR_BGR2BGRA);

    for (int x = 0; x < mat.cols; x++)
    {
        for (int y = 0; y < mat.rows; y++)
        {
            int nPt = matMask.at<uchar>(Point(x, y));
            if (nPt < 1)
            {
                cv::Vec4b& pixel = matPng.at<cv::Vec4b>(Point(x, y));
                pixel[3] = 0;
            }
        }
    }

    // imwrite("F:/img/out/outpng2.png", matPng);

    Mat matRes;
    cvtColor(matPng, matRes, COLOR_BGRA2BGR);
    return matRes;
}

Mat imgProcess::setImgText(const Mat& mat, std::string& strText, std::string& strFont, int nFontSize, bool bReverse /*= false*/)
{
    return Mat();
#if 0
    if (nFontSize < 1)
        nFontSize = 2;

    Mat matMask(mat.size(), CV_8UC3, Scalar::all(0));

    int thickness = -1;
    int linestyle = 8;
    int baseline = 0;

    Ptr<freetype::FreeType2> ft2;
    ft2 = freetype::createFreeType2();
    // ft2->loadFontData("/home/x/.local/share/fonts/GenWanMin.ttf", 0);
    ft2->loadFontData("C:/Windows/Fonts/CENTURY.TTF", 0);
    //ft2->loadFontData(strFont, 0);

    Size textSize = ft2->getTextSize(strText,
        nFontSize,
        thickness,
        &baseline);
    if (thickness > 0)
    {
        baseline += thickness;
    }

    Point ptText((matMask.cols - textSize.width) / 2,
        (matMask.rows + textSize.height) / 2);

    // rectangle(matMask, ptText + Point(0, baseline),
    //           ptText + Point(textSize.width, -textSize.height),
    //           Scalar(0, 255, 0), 1, 8);

    // line(matMask, ptText + Point(0, thickness),
    //      ptText + Point(textSize.width, thickness),
    //      Scalar(0, 0, 255), 1, 8);

    ft2->putText(matMask, strText, ptText, nFontSize,
        Scalar::all(255), thickness, linestyle, true);

    // imshow("mast",matMask);

    if (bReverse)
        bitwise_not(matMask, matMask);

    // imshow("Mask", matMask);

    matMask = this->setThreshold(matMask, 125, 255);

    // 原图与掩码进行操作
    Mat matDst = mat.clone();
    // bitwise_and(mat, mat, matDst, matMask);

    for (int x = 0; x < matDst.cols; x++)
    {
        for (int y = 0; y < matDst.rows; y++)
        {
            int nValue = matMask.at<uchar>(Point(x, y));
            if (nValue < 1)
            {
                cv::Vec3b& pixel = matDst.at<Vec3b>(Point(x, y));
                pixel[0] = 255;
                pixel[1] = 255;
                pixel[2] = 255;
            }
        }
    }

    return matDst;
#endif
}

Mat imgProcess::setImgGamma(const Mat& mat, const double dGamma)
{
    double dInvGamma = 1 / dGamma;

    Mat table(1, 256, CV_8U);
    uchar* p = table.ptr();
    for (int i = 0; i < 256; ++i)
    {
        p[i] = (uchar)(pow(i / 255.0, dInvGamma) * 255);
    }

    Mat matRes;
    LUT(mat, table, matRes);
    return matRes;
}

Mat imgProcess::setImgEdgePaperEffect(const Mat& mat, double dLineInterval, double dDPI, int nContrast, int nBrightness, double dGamma, int nEnhanceRadius, int nEnhanceAmount, double dAngle /*=0.0*/)
{
    int nType = dLineInterval;
    int nTemp = nContrast;
    int nAdjust = nBrightness;

    //////////////////////////////////////////////////////

    if (nEnhanceRadius < 1)
        nEnhanceRadius = 1;

    if (nEnhanceRadius % 2 == 0)
        nEnhanceRadius += 1;

    Mat matRes;
    GaussianBlur(mat, matRes, Size(nEnhanceRadius, nEnhanceRadius), nEnhanceAmount); // 高斯平滑

    addWeighted(mat, 1.0 + 3.0 + 0.33333, matRes, -3.0, 0, matRes);

    ///////////////////////
    double dInvGamma = 1 / dGamma;
    if (dInvGamma < 0)
        dInvGamma = 0.001;

    Mat table(1, 256, CV_8U);
    uchar* p = table.ptr();
    for (int i = 0; i < 256; ++i)
    {
        p[i] = (uchar)(pow(i / 255.0, dInvGamma) * 255);
    }

    LUT(matRes, table, matRes);

    // return matRes;

    imshow("Edge Strong", matRes);

    Mat matOri = matRes.clone();

    // Screw Ordered Dithering
    int Mask0[] = { 64, 53, 42, 26, 27, 43, 54, 61,
                   60, 41, 25, 14, 15, 28, 44, 55,
                   52, 40, 13, 5, 6, 16, 29, 45,
                   39, 24, 12, 1, 2, 7, 17, 30,
                   38, 23, 11, 4, 3, 8, 18, 31,
                   51, 37, 22, 10, 9, 19, 32, 41,
                   59, 50, 36, 21, 20, 33, 47, 56,
                   63, 58, 49, 35, 34, 48, 57, 62 };

    // CoarseFatting Ordered Dithering
    int Mask1[] = { 4, 14, 52, 58, 56, 45, 20, 6,
                   16, 26, 38, 50, 48, 36, 28, 18,
                   43, 35, 31, 9, 11, 25, 33, 41,
                   61, 46, 23, 1, 3, 13, 55, 60,
                   57, 47, 21, 7, 5, 15, 53, 59,
                   49, 37, 29, 19, 17, 27, 39, 51,
                   10, 24, 32, 40, 42, 34, 30, 8,
                   2, 12, 54, 60, 51, 44, 22, 0 };

    // 有序抖动算法 Bayer Ordered Dithering
    int Mask2[] = { 0, 32, 8, 40, 2, 34, 10, 42,
                   48, 16, 56, 42, 50, 18, 58, 26,
                   12, 44, 4, 36, 14, 46, 6, 38,
                   60, 28, 52, 20, 62, 30, 54, 22,
                   3, 35, 11, 43, 1, 33, 9, 41,
                   51, 19, 59, 27, 49, 17, 57, 25,
                   15, 47, 7, 39, 13, 45, 5, 37,
                   63, 31, 55, 23, 61, 29, 53, 21 };

    // Halftone Ordered Dithering
    int Mask3[] = { 28, 10, 18, 26, 36, 44, 52, 34,
                   22, 2, 4, 12, 48, 58, 60, 42,
                   14, 6, 0, 20, 40, 56, 62, 50,
                   24, 16, 8, 30, 32, 54, 46, 38,
                   37, 45, 53, 35, 29, 11, 19, 27,
                   49, 59, 61, 43, 23, 3, 5, 13,
                   41, 57, 63, 51, 15, 7, 1, 21,
                   33, 55, 47, 39, 25, 17, 9, 31 };

    // 12 * 12  大颗粒
    int Mask4[] = { 144, 140, 132, 122, 107, 63, 54, 93, 106, 123, 133, 142,
                   143, 137, 128, 104, 94, 41, 31, 65, 98, 116, 120, 139,
                   135, 131, 114, 97, 61, 35, 24, 55, 80, 103, 113, 125,
                   126, 117, 88, 83, 56, 29, 15, 51, 68, 90, 99, 111,
                   109, 100, 81, 77, 48, 22, 8, 28, 47, 76, 85, 96,
                   91, 44, 16, 12, 9, 3, 5, 21, 25, 33, 37, 73,
                   59, 58, 30, 18, 10, 1, 2, 4, 11, 19, 34, 42,
                   92, 64, 57, 52, 26, 6, 7, 14, 32, 46, 53, 74,
                   101, 95, 70, 67, 38, 13, 20, 36, 50, 75, 82, 108,
                   121, 110, 86, 78, 45, 17, 27, 39, 69, 79, 102, 119,
                   134, 129, 112, 89, 49, 23, 43, 60, 71, 87, 115, 127,
                   141, 138, 124, 118, 66, 40, 62, 72, 84, 105, 130, 136 };

    // 16 * 16
    int Mask5[] = { 0, 191, 48, 239, 12, 203, 60, 251, 3, 194, 51, 242, 15, 206, 63, 254,
                   127, 64, 175, 112, 139, 76, 187, 124, 130, 67, 178, 115, 142, 79, 190, 127,
                   32, 223, 16, 207, 44, 235, 28, 219, 35, 226, 19, 210, 47, 238, 31, 222,
                   159, 96, 143, 80, 171, 108, 155, 92, 162, 99, 146, 83, 174, 111, 158, 95,
                   8, 199, 56, 247, 4, 195, 52, 243, 11, 202, 59, 250, 7, 198, 55, 246,
                   135, 72, 183, 120, 131, 68, 179, 116, 138, 75, 186, 123, 134, 71, 182, 119,
                   40, 231, 24, 215, 36, 227, 20, 211, 43, 234, 27, 218, 39, 230, 23, 214,
                   167, 104, 151, 88, 163, 100, 147, 84, 170, 107, 154, 91, 166, 103, 150, 87,
                   2, 193, 50, 241, 14, 205, 62, 253, 1, 192, 49, 240, 13, 204, 61, 252,
                   129, 66, 177, 114, 141, 78, 189, 126, 128, 65, 176, 113, 140, 77, 188, 125,
                   34, 225, 18, 209, 46, 237, 30, 221, 33, 224, 17, 208, 45, 236, 29, 220,
                   161, 98, 145, 82, 173, 110, 157, 94, 160, 97, 144, 81, 172, 109, 156, 93,
                   10, 201, 58, 249, 6, 197, 54, 245, 9, 200, 57, 248, 5, 196, 53, 244,
                   137, 74, 185, 122, 133, 70, 181, 118, 136, 73, 184, 121, 132, 69, 180, 117,
                   42, 233, 26, 217, 38, 229, 22, 213, 41, 232, 25, 216, 37, 228, 21, 212,
                   169, 106, 153, 90, 165, 102, 149, 86, 168, 105, 152, 89, 164, 101, 148, 85 };

    // 9 * 9
    int Mask6[] = {
        53, 53, 54, 55, 56, 57, 58, 59, 60,
        51, 27, 28, 29, 30, 31, 32, 33, 61,
        50, 26, 10, 11, 12, 13, 14, 34, 62,
        49, 25, 9, 1, 2, 3, 15, 35, 63,
        80, 48, 24, 8, 0, 4, 16, 36, 64,
        79, 47, 23, 7, 6, 5, 17, 37, 65,
        78, 46, 22, 21, 20, 19, 18, 38, 66,
        77, 45, 44, 43, 42, 41, 40, 39, 67,
        76, 75, 74, 73, 72, 71, 70, 69, 68 };

    Mat matTemp = setRotateImg(matOri, dAngle, true);

    // -------------------------------------------------
    Mat matTempx = matTemp.clone();
    // 对原图像使用 GaussianBlur 降噪(内核大小 = 3)
    GaussianBlur(matTempx, matTempx, Size(3, 3), 0, 0, BORDER_DEFAULT); // 高斯平滑

    // ------------------------------------------------
    // 锐化測試
    GaussianBlur(matTemp, matTempx, Size(49, 49), 26);
    addWeighted(matTempx, 1.0 + 3.0, matTempx, -3.0, 0, matTempx);

    // imshow("Test", matTempx);
    //  ------------------------------------------------

    Mat src_gray;
    /// 转换为灰度图
    cvtColor(matTempx, src_gray, COLOR_RGB2GRAY);

    /// 创建 grad_x 和 grad_y 矩阵
    Mat grad_x, grad_y;
    Mat abs_grad_x, abs_grad_y;

    int scalex = 1;
    int delta = 0;
    int ddepth = CV_16S;

    // 求 X方向梯度
    // Scharr( src_gray, grad_x, ddepth, 1, 0, scalex, delta, BORDER_DEFAULT );
    Sobel(src_gray, grad_x, ddepth, 1, 0, 3, scalex, delta, BORDER_DEFAULT);
    convertScaleAbs(grad_x, abs_grad_x);

    // 求Y方向梯度
    // Scharr( src_gray, grad_y, ddepth, 0, 1, scalex, delta, BORDER_DEFAULT );
    Sobel(src_gray, grad_y, ddepth, 0, 1, 3, scalex, delta, BORDER_DEFAULT);
    convertScaleAbs(grad_y, abs_grad_y);

    Mat gradEdge;
    // 合并梯度(近似) // addWeighted使用OpenCV对两幅图像求和(求混合(blending))
    addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, gradEdge);

    // imshow("Grad", grad);

    // -------------------------------------------------

    // mask

    if (nType < 0)
        nType *= -1;

    if (mat.channels() < 3)
        cvtColor(mat, matOri, COLOR_GRAY2BGR);

    // Mat matTemp = matOri;
    // matTemp = setRotateImg(matOri, nE, true);

    Mat matGray = getGray(matTemp);

    // Mat matSmooth;
    // cvSmooth( matGray, matSmooth, CV_BLUR_NO_SCALE, 3, 0, 0, 0);

    // for (int y = 0; y < matGray.rows; y++)
    // {
    //     for (int x = 0; x < matGray.cols; x++)
    //     {
    //     }
    // }

    // 混合
    Mat matMix;
    addWeighted(gradEdge, 0.5, matGray, 0.5, 0, matMix);
    // imshow("matMix", matMix);
    // matGray = setRotateImg(matMix, nE, true);

    std::vector<int*> vecArr;
    vecArr.emplace_back(Mask0);
    vecArr.emplace_back(Mask1);
    vecArr.emplace_back(Mask2);
    vecArr.emplace_back(Mask3);
    vecArr.emplace_back(Mask4);
    vecArr.emplace_back(Mask5);
    vecArr.emplace_back(Mask6);

    if (dDPI < 1)
        dDPI = 1;

    if (dDPI > 8)
        dDPI = 8;

    int K = 8;
    int L = 8;
    int N = 63;

    if (nType % 7 == 4)
    {
        K = 12;
        L = 12;
        N = 123;
    }
    else if (nType % 7 == 5)
    {
        K = 16;
        L = 16;
        N = 255;
    }
    else if (nType % 7 == 5)
    {
        K = 16;
        L = 16;
        N = 255;
    }

    Size dsize = Size(matGray.cols * dDPI, matGray.rows * dDPI);
    Mat matGrayResize;
    resize(matGray, matGrayResize, dsize);

    int nW = matGrayResize.cols;
    int nH = matGrayResize.rows;

    for (int y = 0; y < nH; y++)
    {
        int k = y % K;

        for (int x = 0; x < nW; x++)
        {
            int l = x % L;

            double dPix = matGray.at<uint8_t>(Point(x / dDPI, y / dDPI)) / 255.0 * N + 0.5 + nAdjust * 0.1;

            if (dPix > vecArr[nType % 7][(k * L + l)])
            {
                matGrayResize.at<uint8_t>(Point(x, y)) = 255;
            }
            else
            {
                matGrayResize.at<uint8_t>(Point(x, y)) = 0;
            }
        }
    }

    Mat matOut;
    cvtColor(matGrayResize, matOut, COLOR_GRAY2BGR);
    Mat matOutX;
    matOutX = setRotateImg(matOut, -1.0 * dAngle, true);

    return matOutX;
}
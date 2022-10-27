/*
 * @Author: lyl vm@ubuntu.com
 * @Date: 2022-05-06 16:43:58
 * @LastEditors: lyl_Linux april@ubuntu.com
 * @LastEditTime: 2022-05-16 15:11:20
 * @FilePath: /rdimgprocess/include/imgProcess.h
 * @Description: 基于OpenCV的图像处理库
 */

#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

namespace ImgSpace
{
    class imgProcess
    {
    public:
        imgProcess();
        imgProcess(std::string &strPath);
        ~imgProcess();

    private:
    public:
        /**
         * @description: 读取图像数据
         * @param {string} &strPath  图片路径
         * @param {int} nType   读取图片类型 暂未定义, 预留
         * @return 处理后的图像数据
         */
        Mat readImg(std::string &strPath, int nType = 1);

    public:
        /**
         * @description: 获取原始图像数据
         * @param {*}
         * @return 处理后的图像数据
         */
        Mat getOriginImg();

    public:
        /**
         * @description:
         * @param {float} dRatio
         * @return 处理后的图像数据
         */
        Mat resizeImg(const Mat &mat, double dRatio);

        /**
         * @description:
         * @param {string} strPath
          @return 处理后的图像数据
         */
        void saveImg(const Mat &mat, std::string strPath);

    private:
    public:

        Mat setImgMix(const Mat &matA, Mat &matB, double dAlaphaA = 0.5, double dAlaphaB = 0.5, int nType = -1);

        /**
         * @description:
         * @param {Mat&} mat
          @return 处理后的图像数据(灰度图)
         */
        
        Mat getGray(const Mat &mat);

        /**
         * @description: 全局二值化
         * @param {Mat&} mat 源图像，可以为8位的灰度图，也可以为32位的彩色图像
         * @param {int} nValueA 阈值
         * @param {int} nValueB 最大值 
         * @param {int} nType 处理类型 0黑白 1黑白反转 2高灰低保 3高保低零 4低保高零 
          @return 处理后的图像数据
         */
        Mat setThreshold(const Mat &mat, int nValueA, int nValueB = 255, int nType = 0);

        /**
         * @description: 获取红色通道数据
         * @param {Mat&} mat
          @return 处理后的图像数据
         */
        Mat getRedChannel(const Mat &mat);

        /**
         * @description:获取绿色通道数据
         * @param {Mat&} mat
          @return 处理后的图像数据
         */
        Mat getGreeChannel(const Mat &mat);

        /**
         * @description:获取蓝色通道数据
         * @param {Mat&} mat
          @return 处理后的图像数据
         */
        Mat getBlueChannel(const Mat &mat);

        /**
         * @description:获取红蓝色通道数据
         * @param {Mat&} mat
          @return 处理后的图像数据
         */
        Mat getRGChannel(const Mat &mat);

        /**
         * @description:获取红蓝通道数据
         * @param {Mat&} mat
          @return 处理后的图像数据
         */
        Mat getRBChannel(const Mat &mat);

        /**
         * @description:获取红色通道数据
         * @param {Mat&} mat
          @return 处理后的图像数据
         */
        Mat getGBChannel(const Mat &mat);

        /**
         * @description: 旋转图像
         * @param {Mat&} mat
         * @param {double} dAngle 旋转角度(度)
         * @param {bool} bChangeSize 图像尺寸是否改变
          @return 处理后的图像数据
         */
        Mat setRotateImg(const Mat &mat, double dAngle = 0.0, bool bChangeSize = false);

        /**
         * @description: 对图片进行等比缩放
         * @param {Mat&} mat
         * @param {double} dScale 缩放比例(等比缩放)
          @return 处理后的图像数据
         */
        Mat setScaleImg(const Mat &mat, double dScale = 1.0);

        /**
         * @description: 对图片进行非等比缩放
         * @param {Mat&} mat
         * @param {double} dScaleX X轴方向的缩放比例
         * @param {double} dScaleY Y轴方向的缩放比例
         * @return {*}
         */
        Mat setScaleImg(const Mat &mat, double dScaleX, double dScaleY);

        /**
         * @description: 对图片进行90度顺时针旋转
         * @param {Mat&} mat
         * @param {int} nIndex  旋转次数 1/90度 2/180度 3/270度
         * @return {*}
         */
        Mat rotate90(const Mat &mat, int nIndex = 0);

        /**
         * @description:
         * @param {Mat&} mat
         * @param {int} w
         * @param {int} h
         * @param {int} nType
          @return 处理后的图像数据
         */
        Mat setErodeImg(const Mat &mat, int w = 5, int h = 5, int nType = 0); // 腐蚀

        /**
         * @description:
         * @param {Mat&} mat
         * @param {int} w
         * @param {int} h
          @return 处理后的图像数据
         */
        Mat setBlurImg(const Mat &mat, int w = 7, int h = 7); // 模糊

        /**
         * @description:
         * @param {Mat&} mat
         * @param {int} a
         * @param {int} b
         * @param {int} c
         * @param {int} w
         * @param {int} h
          @return 处理后的图像数据
         */
        Mat setCannyImg(const Mat &mat, int a = 3, int b = 9, int c = 3, int w = 13, int h = 13); // 提边

        /**
         * @description:
         * @param {Mat&} mat
         * @param {double} dContrast
         * @param {double} dBright
          @return 处理后的图像数据
         */
        // Mat setContrastAndBright(const Mat& mat, double dContrast = 100.0, double dBright = 0.0);
        Mat setContrastAndBright(const Mat &mat, double dH, double dS, double dV);

        /**
         * @description:
         * @param {Mat} &mat
         * @param {double} sigmaX
         * @param {double} sigmaY
         * @param {int} nBorderType
          @return 处理后的图像数据
         */
        Mat setSharpening(const Mat &mat, double sigmaX /*= 100.0*/, double sigmaY /*= 0.0*/, int nBorderType /*=BORDER_DEFAULT*/); //图像锐化(image sharpening)

        /**
         * @description:
         * @param {Mat} &mat
         * @param {int} nA
          @return 处理后的图像数据
         */
        Mat setImgContours(const Mat &mat, int nA);

        /**
         * @description:
         * @param {Mat} &mat
         * @param {int} nA
         * @param {int} nType
          @return 处理后的图像数据
         */
        Mat setImgBlur(const Mat &mat, int nA, int nType);

        /**
         * @description:
         * @param {Mat} &mat
         * @param {int} nA
         * @param {int} nType
          @return 处理后的图像数据
         */
        Mat setMosaic(const Mat &mat, int nA, int nType);

        // 浮雕
        Mat setRelief(const Mat &mat, int nA, int nType);

        // 素描
        Mat setImgSketch(const Mat &mat, int nA, int nType);
        
        // 颜色样式
        Mat setColorStyle(const Mat &mat, int nA);
        /**
         * @description:
         * @param {Mat} &mat
         * @param {int} nBrushSize
         * @param {int} nCoarseness
          @return 处理后的图像数据
         */
        /**
         * @description:
         * @param {Mat} &mat
         * @param {int} nBrushSize
         * @param {int} nCoarseness
          @return 处理后的图像数据
         */
        Mat setImgOilPaint(const Mat &mat, int nBrushSize, int nCoarseness);

        /**
         * @description:
         * @param {Mat} &mat
         * @param {int} nBrushSize
         * @param {int} nCoarseness
          @return 处理后的图像数据
         */
        Mat setDotPaint(const Mat &mat, int nBrushSize, int nCoarseness);

        /**
         * @description:
         * @param {Mat} &mat
         * @param {int} nBrushSize
         * @param {int} nCoarseness
          @return 处理后的图像数据
         */
        Mat setDither(const Mat &mat, int nBrushSize, int nCoarseness);

        /**
         * @description:
         * @param {Mat} &mat
         * @param {int} nRatio
         * @param {int} nBright
         * @param {int} nType
         * @param {int} nAdjust
         * @param {int} nE
          @return 处理后的图像数据
         */
        Mat setDither(const Mat &mat, double dScale, int nBright, int nType, int nAdjust, int nE);

        /**
         * @description:
         * @param {Mat} &mat
         * @param {int} type
          @return 处理后的图像数据
         */
        Mat setImgMirror(const Mat &mat, int type = 0);

        /**
         * @description:
         * @param {Mat} &mat
         * @param {int} type
          @return 处理后的图像数据
         */
        Mat setColorReversal(const Mat &mat, int type = 0);

        /**
         * @description: 美顔
         * @param {Mat} processMat
         * @param {int} nA
         * @param {int} nB
          @return 处理后的图像数据
         */
        Mat setBeautify(const Mat &mat, int nA, int nB);

        /**
         * @description:
         * @param {Mat} &processMat
         * @param {double} dA
         * @param {double} dB
         * @param {double} dC
          @return 处理后的图像数据
         */
        Mat setAutoWhithBalance(const Mat &mat, double dA, double dB, double dC);

        /**
         * @description:
         * @param {Mat} &mat
         * @param {int} nPercent
          @return 处理后的图像数据
         */
        Mat setColorTemperature(const Mat &mat, int nPercent);

        /**
         * @description:
         * @param {Mat} processMat
         * @param {int} nR
         * @param {int} nG
         * @param {int} nB
          @return 处理后的图像数据
         */
        Mat setColorReplace(const Mat &mat, int nR, int nG, int nB);

        /**
         * @description:
         * @param {Mat} &processMat
         * @param {double} dA
         * @param {double} dB
         * @param {double} dC
          @return 处理后的图像数据
         */
        Mat drawLaserLine(const Mat &mat, double dA, double dB, double dC);

        /**
         * @description:
         * @param {Mat} &processMat
         * @param {double} dA
         * @param {double} dB
         * @param {double} dC
          @return 处理后的图像数据
         */
        Mat setImgEdgeStrong(const Mat &mat, int nA, int nB, double dC);

        /**
         * @description:
         * @param {Mat} &processMat
         * @param {double} dA
          @return 处理后的图像数据
         */
        Mat setImgMask(const Mat &mat, double dA);

        /**
         * @description: 在图像上添加一个png
         * @param {Mat} &processMat
          @return 处理后的图像数据
         */
        Mat setImgPngMerge(const Mat &mat, std::string strPngPath, int nXPos = 0, int nYPos = 0);

        /**
         * @description:
         * @param {Mat} &processMat
          @return 处理后的图像数据
         */
        Mat setImgCut(const Mat &mat);

        /**
         * @description:
         * @param {Mat} &processMat
         * @param {double} dA
         * @param {double} dB
         * @param {double} dC
          @return 处理后的图像数据
         */
        Mat imgTransparent(const Mat &mat, double dA, double dB, double dC);

        /**
         * @description:
         * @param {Mat} processMat
         * @param {string} strText
         * @param {string} strFont
         * @param {int} nFontSize
          @return 处理后的图像数据
         */
        Mat setImgText(const Mat &mat, std::string &strText, std::string &strFont, int nFontSize, bool bReverse = false);

        /**
         * @description:
         * @param {Mat&} mmat
         * @param {double} dGamma
          @return 处理后的图像数据
         */
        Mat setImgGamma(const Mat &mat, const double dGamma);

        /**
         * @description:
         * @param {Mat&} mat
         * @param {double} dLineInterval 挂网类型
         * @param {double} dDPI
         * @param {int} nContrast
         * @param {int} nBrightness
         * @param {double} dGamma
         * @param {int} nEnhanceRadius 半径
         * @param {int} nEnhanceAmount
         * @param {double} dAngle
         * @return {*}
         */
        Mat setImgEdgePaperEffect(const Mat &mat, double dLineInterval, double dDPI, int nContrast, int nBrightness, double dGamma, int nEnhanceRadius, int nEnhanceAmount, double dAngle = 0.0);

    private:
        Mat m_matOri;
    };

}
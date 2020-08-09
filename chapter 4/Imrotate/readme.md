# 旋转图片代码优化
## 优化路径
### 初始代码（优化0）
```c++
for (int y = 0; y < imgSize.height; y++) {
    for (int x = 0; x < imgSize.width; x++) {
        int X = x - cx;
        int Y = y - cy;
        double nx = cos(rotAngle)*X - sin(rotAngle)*Y;
        double ny = sin(rotAngle)*X + cos(rotAngle)*Y;

        double diagonal = sqrt((double)imgSize.height*imgSize.height + imgSize.width*imgSize.width);
        double scaleFactor = (imgSize.width > imgSize.height) ? imgSize.height / diagonal : imgSize.width / diagonal;
        nx *= scaleFactor;
        ny *= scaleFactor;
        int nnx = nx + cx;
        int nny = ny + cy;
        if (nnx >= 0 && nnx < imgSize.width && nny >= 0 && nny < imgSize.height) {
            output.at<Vec3b>(nny, nnx) = input.at<Vec3b>(y, x);
        }
    }
}
```

### 优化1
多线程版本，加上使用`ptr`而不是`at`寻址
```c++
if (nnx >= 0 && nnx < imgSize.width && nny >= 0 && nny < imgSize.height) {
    uchar *outPr = output.ptr<uchar>(nny);
    outPr[nnx * 3] = inPr[x * 3];
    outPr[nnx * 3 + 1] = inPr[x * 3 + 1];
    outPr[nnx * 3 + 2] = inPr[x * 3 + 2];
}
```

### 优化2
继续修改，将`diagonal`和`scaleFactor`的计算移至外循环外

### 优化3
继续修改，在外循环外计算三角函数`cos()`和`sin()`

### 优化4
继续修改，提前给变量`X,Y,nnx,nny,nx,ny`分配内存

### 优化5
继续修改，每一行中`SRA*Y`的计算是相同的，可移至内循环外

### 优化6
继续修改，最终版本，在外循环计算`scaleFactor*CRA`和`scaleFactor*SRA`。内循环中将`CRA*X-SRAY`进一步分解，在外循环计算初始值后，内循环仅保留加法运算即可。
```c++
double diagonal = sqrt((double)imgSize.height*imgSize.height + imgSize.width*imgSize.width);
double scaleFactor = (imgSize.width > imgSize.height) ? imgSize.height / diagonal : imgSize.width / diagonal;
double CRA = cos(rotAngle);
double SRA = sin(rotAngle);
CRA *= scaleFactor;
SRA *= scaleFactor;
int X, Y, nnx, nny;
double nx, ny;

for (int y = be; y < en; y++) {
    uchar *inPr = input.ptr<uchar>(y);
    Y = y - cy;
    double SRAY = SRA*Y;
    double CRAY = CRA*Y;
    double nnx0 = -cx * CRA - SRAY + cx;
    double nny0 = -cx * SRA + CRAY + cy;
    for (int x = 0; x < imgSize.width; x++) {
        nnx = (int)nnx0;
        nny = (int)nny0;
        if (nnx >= 0 && nnx < imgSize.width && nny >= 0 && nny < imgSize.height) {
            uchar *outPr = output.ptr<uchar>(nny);
            outPr[nnx * 3] = inPr[x * 3];
            outPr[nnx * 3 + 1] = inPr[x * 3 + 1];
            outPr[nnx * 3 + 2] = inPr[x * 3 + 2];
        }
        nnx0 += CRA;
        nny0 += SRA;
    }
}
```

## 性能分析
| 毫秒 | 优化0 | 优化1 | 优化2 | 优化3 | 优化4 | 优化5 | 优化6 |
| :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: | 
| 线程1 | 430.6 | 182.6 | 133.5 | 105.7 | 105.5 | 93.4 | 65.8 |   
| 线程2 |       | 99.4 | 76.9 | 58.3 | 56.3 | 51.3 | 38.7 |
| 线程3 |       | 75.4 | 58.4 | 42.1 |  42.7 | 38.5 | 32 |
| 线程4 |       | 67.6 | 51.6 | 38.3 | 38.8 | 35.5 | 32 |
| 线程5 |       | 78.4 | 67.9 | 51.5 | 51.9 | 46.8 | 38.5 |
| 线程6 |       | 74.1 | 62.6 | 46.4 | 46.4 | 43 | 36.9 |
**初步结论**

1. 预分配内存并不能提升性能
2. 随着优化的提升，再提升线程数，优化力度变小，这可能与内存带宽有关

# 欢迎使用低多边形（low poly）转换器

本工具依赖以下内容：

> * [FBX三角形网格导入、导出、渲染](http://usa.autodesk.com/adsk/servlet/pc/item?siteID=123112&id=10775847)
> * [一种简单、快速、高效的多边形减面算法](http://dev.gameres.com/Program/Visual/3D/PolygonReduction.pdf)
> * 顶点分离算法
> * [ImGui](https://github.com/ocornut/imgui)

------

独立游戏通常由比较小规模的团队或个人开发，由于资源有限，在游戏美术上很难像3A游戏公司那样把所有美术资源都花费大量精力去制作。但这并不意味着独立游戏美术逊色于大型游戏美术，相反，独立游戏的美术风格独特而又具有表现力，一些拥有优秀美术风格的独立游戏甚至能称之为艺术作品。

## 什么是低多边形（low poly）

低多边形起源于20世纪90年代的计算机三维建模，在当时，由于计算机性能和游戏引擎等制作条件的限制，计算机三维建模做不到像如今那样精细，只能以减少模型面数来平衡游戏的性能。不过最近两年，复古之风盛行，这种低多边形的建模又变得逐渐流行起来了，原因不再是因为最初为了游戏性能而做出妥协，而是人们发现这种美术风格能给我们的视觉感官带来一种前所未有的刺激。

低多边形有着简约、抽象的特征，其棱角分明、结构激进的风格能产生强大的视觉冲击力；没有太复杂的细节，却又能表达物体最重要的特征，这种能够降低游戏开销又能增强视觉效果的美术风格受到了许多独立游戏开发者的青睐。

## 如何使用低多边形转换器

摆脱美术资源的限制，使用转换器把平滑的多边形转换成低多边形风格吧

### 1.打开一个Fbx网格文件

首先需要一个Fbx文件。

![1](https://github.com/zd304/lowpolyconverter/blob/master/ReadMe/1.png)

### 2.编辑Fbx网格文件

根据需要改变网格模型。

![2](https://github.com/zd304/lowpolyconverter/blob/master/ReadMe/2.png)

> * 蒙皮模型减面【使用坍塌算法和顶点分离算法处理蒙皮模型
>> * 预期顶点数【坍塌后，模型预期能减少到得的顶点数
>> * lowpoly风格选择框【坍塌后使用顶点分离算法将模型转换成低多边形风格的模型
> * 渲染选项【为了方便用户观察设置的选项
>> * 旋转速度【模型旋转速度系数
>> * 相机距离【相机离模型的远近
>> * 相机高度【相机离地面的高度
>> * 相机位置【相机水平方向的位置偏移
>> * 显示模型【是否显示多边形网格
>> * 显示骨骼【是否显示模型骨骼
>> * 播放动画【当前模型播放的动画名称
> * 模型信息【网格模型的一些信息

### 3.转换为低多边形网格模型

![3](https://github.com/zd304/lowpolyconverter/blob/master/ReadMe/3.png)

### 4.保存为Fbx网格模型

![4](https://github.com/zd304/lowpolyconverter/blob/master/ReadMe/4.png)

## 运行工程

由于lib文件太大，工程里不再包含。

如果要用Visual Studio运行项目，请先下载FBX SDK和DirectX 9.0 SDK。

* 将FBX SDK下的include文件夹和lib文件夹复制粘贴到“LowpolyConvertor/FBX/”文件夹下。

* 将DirectX SDK下的Include文件夹和Lib文件夹复制粘贴到“LowpolyConvertor/DirectX/”文件夹下。
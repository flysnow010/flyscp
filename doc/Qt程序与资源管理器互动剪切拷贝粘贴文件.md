## Qt程序如何与Windows资源管理器互动剪切/拷贝/粘贴文件
### 1.概述
本文描述Qt程序与Windows资源管理器互动拷贝粘贴文件。包括两个方面:
* Windows资源管理器剪切/拷贝文件,Qt程序粘贴文件。
* Qt程序剪切/拷贝文件,Windows资源管理器粘贴文件。

### 2.Qt程序访问剪切板 
Qt程序通过剪切板类QClipboard访问Windows系统剪切板。
通过如下代码可以访问剪贴板中的内容.
```cpp
 const QClipboard *clipboard = QApplication::clipboard();
 qDebug() << clipboard->text();
```
在Windows资源管理器中选中文件并剪切，剪切板内容如下:
```
"file:///E:/test.dat\nfile:///E:/test.zip\n"
```
在Windows资源管理器中选中文件并复制，剪切板内容如下:
```
"file:///E:/test.dat\nfile:///E:/test.zip\n"
```

剪切和复制操作，从剪切板中内容是相同的，都是选择则的文件并在文件名前增加file:///多个文件名以\n分割。如何区分是剪切和复制操作呢？下一节将描述如何区分。
### 3.Qt程序区分剪切和复制操作
QClipboard类有函数mimeData返回QMimeData对象可以访问剪切板中数据类型。
通过下面代码可以返回剪切板中数据类型:
```CPP
const QClipboard *clipboard = QApplication::clipboard();
const QMimeData* mimeData = clipboard->mimeData();
qDebug() << mimeData->formats();
```
剪切和复制时剪切板中数据格式如下:
```
"Shell IDList Array", 
"DataObjectAttributes",
"DataObjectAttributesRequiringElevation", 
"text/uri-list", 
"UIDisplayed", 
"DropDescription", 
"FileName", 
"FileNameW", 
"Shell Object Offsets", 
"Preferred DropEffect", 
"AsyncFlag"
```
通过如下代码可以获取不同数据格式中内容:
```cpp
const QClipboard *clipboard = QApplication::clipboard();
const QMimeData* mimeData = clipboard->mimeData();
qDebug() << "Shell IDList Array: " << mimeData()->data("Shell IDList Array");
qDebug() << "DataObjectAttributes: " << mimeData()->data("DataObjectAttributes");
qDebug() << "DataObjectAttributesRequiringElevation: " 
    << mimeData()->data("DataObjectAttributesRequiringElevation");
qDebug() << "UIDisplayed: " << mimeData()->data("UIDisplayed");
qDebug() << "DropDescription: " << mimeData()->data("DropDescription");
qDebug() << "FileName: " << mimeData()->data("FileName");
qDebug() << "FileNameW: " << mimeData()->data("FileNameW");
qDebug() << "Shell Object Offsets: " << mimeData()->data("Shell Object Offsets");
qDebug() << "Preferred DropEffect: " << mimeData()->data("Preferred DropEffect");
qDebug() << "AsyncFlag: " << mimeData()->data("AsyncFlag");
```

通过以上代码比较剪切和复制时剪切板中内容发现类型Preferred DropEffect和Shell Object Offsets的数据内容不同:

剪切时:
```
Preferred DropEffect:  "\x02\x00\x00\x00"
Shell Object Offsets:  "\xDA\x00\x00\x00j\x02\x00\x00"
```

复制时:
```
Preferred DropEffect:  "\x05\x00\x00\x00"
Shell Object Offsets:  "\xD3\x00\x00\x00u\x02\x00\x00"
```
参考[这篇文章](http://www.taodudu.cc/news/show-1413348.html)发现DropEffect区分剪切和复制操作的。
Qt程序粘贴代码如下:
```cpp
#define DROPEFFECT_COPY (1)
#define DROPEFFECT_MOVE (2)

const QClipboard *clipboard = QApplication::clipboard();
const QMimeData* mimeData = clipboard->mimeData();
QByteArray data = mimeData()->data("Preferred DropEffect");
uint32_t dropEffect = *((uint32_t *)data.data());

if(dropEffect & DROPEFFECT_COPY)
{
    QString srcFileName = "E:/test.dat";
    QString dstFileName = "D:/test.dat";
    CopyFileEx(srcFileName.toStdWString().c_str(),
        dstFileName.toStdWString().c_str(),
            LpprogressRoutine, 0, 0,   COPY_FILE_OPEN_SOURCE_FOR_WRITE);
}

if(dropEffect & DROPEFFECT_MOVE)
{
    QString srcFileName = "E:/test.dat";
    QString dstFileName = "D:/test.dat";
    MoveFileWithProgress(srcFileName.toStdWString().c_str(),
        dstFileName.toStdWString().c_str(),
        LpprogressRoutine, 0, MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH);
}
```

### 4.Qt程序实现剪切操作

Qt程序实现剪切操作，需要将文件名称和Preferred DropEffect类型数据放入剪切板中，Preferred DropEffect的数据是 [0x2, 0 , 0 ,0].
```cpp
QClipboard *clipboard = QApplication::clipboard();
QMimeData* mineData = new QMimeData();
mineData->setText("file:///E:/test.dat");
QByteArray data(4, 0);
data[0] = 0x2;
mineData->setData("Preferred DropEffect", data);
clipboard->setMimeData(mineData);
```

### 5.Qt程序实现复制操作
Qt程序实现复制操作，同样需要将文件名称和Preferred DropEffect类型数据放入剪切板中，不过Preferred DropEffect的数据是 [0x5, 0 , 0 ,0].
```cpp
QClipboard *clipboard = QApplication::clipboard();
QMimeData* mineData = new QMimeData();
mineData->setText("file:///E:/test.dat\nfile:///E:/test.zip\n");
QByteArray data(4, 0);
data[0] = 0x5;
mineData->setData("Preferred DropEffect", data);
clipboard->setMimeData(mineData);
```
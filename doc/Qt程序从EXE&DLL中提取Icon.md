## Qt程序从EXE&DLL中提取Icon
### 代码

首先调用Windows API函数ExtractIconEx从文件获取HICON句柄，再利用QtWin::fromHICON函数将HICON转换为QIcon
```cpp
#include <windows.h>
#include <QtWin>

QIcon GetIcon(QString const& fileName, int index = 0)
{
    HICON hicon;
    if(ExtractIconEx(fileName.toStdWString().c_str(), index, &hicon, 0, 1) > 0)
    {
        if(hicon)
            return QIcon(QtWin::fromHICON(hicon));
    }
    return QIcon();
}

```
### 使用

直接EXE和DLL的文件名和图标索引传递给GetIcon来获取EXE和DLL中的图标。
```cpp
QIcon icon1 = GetIcon("aaa.exe", 1);
QIcon icon2 = GetIcon("bbb.dll");
```
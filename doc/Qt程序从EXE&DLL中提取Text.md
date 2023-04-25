## Qt程序从EXE&DLL中提取Text
### 代码

首先调用Windows API函数LoadLibrary从文件获取HMODULE句柄，再利用Windows API函数LoadString获取文件中的文本。需要注意的时获取的文本时UNICODE编码，需要通过QString::fromStdWString转换为QString文本
```cpp
#include <windows.h>

QString GetText(QString const& fileName, quint32 index)
{
    HMODULE h = LoadLibrary(fileName.toStdWString().c_str());
    wchar_t text[256];
    if(!h)
         return QString();
    int len = LoadString(h, index, text, 256);
    FreeLibrary(h);
    return QString::fromStdWString(std::wstring(text, len));
}

```
### 使用

直接EXE和DLL的文件名和文本索引传递给GetText来获取EXE和DLL中的Text。
```cpp
QString text = GetText("shell32.dll", 8506);
```
编写好 CMakeLists.txt
xample.cpp


CMake, 然后在终端执行:

cmake --build build --config Release -j 4

```
PS E:\pyProj\pyBindCpp> cmake --build build --config Release -j 4
MSBuild version 17.6.3+07e294721 for .NET Framework
  Checking Build System
  Building Custom Rule E:/pyProj/pyBindCpp/callCpp
  /CMakeLists.txt
  example.cpp
    正在创建库 E:/pyProj/pyBindCpp/build/callCpp/Releas
  e/example.lib 和对象 E:/pyProj/pyBindCpp/build/call
  Cpp/Release/example.exp
  正在生成代码
  已完成代码的生成
  example.vcxproj -> E:\pyProj\pyBindCpp\build\cal
  lCpp\Release\example.cp310-win_amd64.pyd        
  Building Custom Rule E:/pyProj/pyBindCpp/CMakeLi
  sts.txt
```
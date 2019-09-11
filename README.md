# MGenner  
midi在线编辑器，使用emscripten开发，留有js API，便于与自己的前端应用嵌合。  
![img](shot.png)
基于层次包围盒算法（虽然我不知道有没有用）  
[在线使用](http://game.sinriv.com/mgenner)  
## js API  
`loadStringData(string)` 加载序列化的字符串数据  
`loadMidiFile(url)` 从url加载文件(支持mid格式或者自己的序列化格式)  
`toStringData(function(data){})` 将数据转成序列化格式（异步）  

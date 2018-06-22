ARC与非ARC在一个项目中同时使用，

1，选择项目中的Targets，选中你所要操作的Target，

2，选Build Phases，在其中Complie Sources中选择需要ARC的文件双击，并在输入框中输入：-fobjc-arc，如果不要ARC则输入：-fno-objc-arc
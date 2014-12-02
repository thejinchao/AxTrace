@echo off
set ANDROID_TARGET=android-20

echo "Cleaning up / removing build folders..."
rd /s /q .\gen
mkdir gen

echo "Build zeromq and axtrace java code..."
javac -d .\gen .\src\zmq\*.java
javac -d .\gen -classpath .\gen .\src\org\zeromq\*.java
javac -d .\gen -classpath .\gen -bootclasspath %ANDROID_SDK_ROOT%\platforms\%ANDROID_TARGET%\android.jar .\src\com\thecodeway\axtrace\*.java

echo "Creating axtrace.jar..."
pause
cd gen
jar cvfM ..\axtrace.jar .
cd ..

echo "Cleaning up / removing build folders..."
pause
rd /s /q .\gen
